#include <cstdio>
#include <cmath>
#include <numbers>
#include "motor.h"
#include "extrude.h"

#define SYRINGE_RADIUS  (8.6 / 2.0)
#define MM_PER_REV      (0.5)
#define MM3_PER_REV     (std::numbers::pi * SYRINGE_RADIUS * SYRINGE_RADIUS * MM_PER_REV)
#define TICKS_PER_REV   (1800.0)
#define TICKS_PER_MM3   (TICKS_PER_REV / MM3_PER_REV)

// to prevent oozing, retract a bit after extrusion
// parameters dependent on paste used, temperature, tip, etc.
#define PRIME_TICKS   (500)
#define RETRACT_TICKS (-500)
#define RETRACT_WAIT  (5)

enum class extrude_state {
    idle,
    extrude,
    continuous,
    wait,
    retract,
};

static struct {
    extrude_state state;
    bool cont;
    int32_t wait_ctr;
    int32_t extrude, wait, retract;
} data;

void extrude_run(void) {
    switch (data.state) {
        case extrude_state::idle:
            motor_write(true, 0);
            break;

        case extrude_state::extrude:
            motor_write(true, 255);
            if (motor_read() >= data.extrude) {
                if (data.cont) {
                    data.state = extrude_state::continuous;
                } else {
                    motor_write(true, 0);
                    data.wait_ctr = 0;
                    data.state = extrude_state::wait;
                }
            }
            data.cont = false;
            break;

        case extrude_state::continuous:
            motor_write(true, 80); // slower speed, more control
            if (!data.cont) {
                motor_write(true, 0);
                data.wait_ctr = 0;
                data.state = extrude_state::wait;
            }
            data.cont = false;
            break;

        case extrude_state::wait:
            motor_write(true, 0);
            data.wait_ctr++;
            if (data.wait_ctr >= data.wait) {
                data.state = extrude_state::retract;
            }
            break;

        case extrude_state::retract:
            motor_write(false, 255);
            if (motor_read() <= data.retract) {
                motor_write(false, 0);
                data.state = extrude_state::idle;
            }
            break;

        default:
            data.state = extrude_state::idle;
            break;
    }
}

bool extrude_idle(void) {
    return data.state == extrude_state::idle;
}

void extrude_set(float amt) {
    data.extrude = PRIME_TICKS + lround(static_cast<float>(TICKS_PER_MM3) * amt);
    data.wait    = RETRACT_WAIT;
    data.retract = RETRACT_TICKS;
}

void extrude_dispense(void) {
    if (data.state == extrude_state::idle) {
        data.state = extrude_state::extrude;
    }
    data.cont = true;
}
