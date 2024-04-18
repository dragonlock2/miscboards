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

// increase pressure in syringe to speed up extrusion
#define EXTRUDE_EXTRA (256)
#define RETRACT_EXTRA (-256)
#define WAIT_EXTRA    (512)

enum class extrude_state {
    idle,
    extrude,
    wait,
    retract,
    continuous,
};

static struct {
    extrude_state state;
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
                motor_write(true, 0);
                data.state = extrude_state::wait;
            }
            break;

        case extrude_state::wait:
            motor_write(true, 0);
            data.wait--;
            if (data.wait <= 0) {
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

        case extrude_state::continuous:
            motor_write(true, 255);
            data.state = extrude_state::idle;
            break;
    }
}

bool extrude_idle(void) {
    return data.state == extrude_state::idle;
}

void extrude_dispense(float amt) {
    if (data.state == extrude_state::idle) {
        data.extrude = EXTRUDE_EXTRA + lround(static_cast<float>(TICKS_PER_MM3) * amt);
        data.wait    = WAIT_EXTRA;
        data.retract = RETRACT_EXTRA;
        data.state = extrude_state::extrude;
    }
}

void extrude_hold_dispense(void) {
    data.state = extrude_state::continuous;
}
