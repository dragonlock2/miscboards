#include "mcc_generated_files/mcc.h"
#include "ssd1306.h"
#include "monitor.h"
#include "btns.h"
#include "gui.h"

/* private defines */
#define LIMIT_DISPLAY_TIME (20) // # of loop iterations

#define FONT_WIDTH  (4)
#define FONT_HEIGHT (6)

#define PWR_EN_Enable()  PWR_EN_SetDigitalOutput()
#define PWR_EN_Disable() PWR_EN_SetDigitalInput()

typedef enum {
    GUI_STATE_IDLE,
    GUI_STATE_ARMING,
    GUI_STATE_RUNNING,
    GUI_STATE_OVERCURRENT,
} gui_state_E;

const char* GUI_STATE_LUT[] = {
    "idle",
    "arming",
    "running",
    "overcurrent",
};

const char* GUI_MONITOR_LUT[] = {
    " 12V",
    "  5V",
    " 3V3",
    "-12V",
};

typedef struct {
    gui_state_E state;
    monitor_chan_E selector;
    int16_t limits[MONITOR_CHAN_COUNT];
    uint8_t limit_timer;
} gui_data_S;
static gui_data_S gui_data;

/* public functions */
void gui_init() {
    PWR_EN_SetLow(); // using as open-drain
    
    ssd1306_init();
    gui_data.state = GUI_STATE_IDLE;
    gui_data.selector = MONITOR_CHAN_12V;
    memset(gui_data.limits, 0, sizeof gui_data.limits);
}

void gui_loop() {
    // process user input and set outputs
    switch (gui_data.state) {
        case GUI_STATE_IDLE:
            PWR_EN_Disable();
            // TODO disable rails too
            
            if (btns_read(BTNS_CHANNEL_CENTER) == BTNS_PRESS_LONG) {
                // TODO enable global
                gui_data.state = GUI_STATE_ARMING;
            }
            break;
            
        case GUI_STATE_ARMING:
            if (PWR_GOOD_GetValue()) {
                // TODO enable all rails
                gui_data.state = GUI_STATE_RUNNING;
            }
            break;
            
        case GUI_STATE_RUNNING:
            if (btns_read(BTNS_CHANNEL_CENTER) == BTNS_PRESS_LONG) {
                gui_data.state = GUI_STATE_IDLE;
            }
            break;

        case GUI_STATE_OVERCURRENT:
            if (btns_read(BTNS_CHANNEL_CENTER) == BTNS_PRESS_LONG) {
                gui_data.state = GUI_STATE_ARMING; // re-enable rails
            }
            break;
    }
    if (btns_read(BTNS_CHANNEL_CENTER) == BTNS_PRESS_SHORT) {
        gui_data.limit_timer = LIMIT_DISPLAY_TIME;
        gui_data.selector++;
        if (gui_data.selector == MONITOR_CHAN_COUNT) {
            gui_data.selector = MONITOR_CHAN_12V;
        }
    }
    
    if (gui_data.limit_timer > 0) {
        gui_data.limit_timer--;
    }
    
    // display user feedback
    ssd1306_clear_display();
    
    ssd1306_set_cursor(0, 0);
    ssd1306_draw_string("matt's power supply monitor!");
    
    ssd1306_set_cursor(FONT_WIDTH, FONT_HEIGHT);
    ssd1306_draw_string("state: ");
    ssd1306_draw_string(GUI_STATE_LUT[gui_data.state]);
    
    ssd1306_set_cursor(0, FONT_HEIGHT * (2 + gui_data.selector));
    ssd1306_draw_char('>');
    for (int i = 0; i < MONITOR_CHAN_COUNT; i++) {
        char buf[32];
        if (gui_data.state == GUI_STATE_IDLE || (gui_data.selector == i && gui_data.limit_timer > 0)) {
            snprintf(buf, sizeof buf, "%s: %6dmV %6dmA(lim)",
                GUI_MONITOR_LUT[i], monitor_read_mV(i), gui_data.limits[i]);
        } else {
            snprintf(buf, sizeof buf, "%s: %6dmV %6dmA",
            GUI_MONITOR_LUT[i], monitor_read_mV(i), monitor_read_mA(i));
        }
        ssd1306_set_cursor(FONT_WIDTH, FONT_HEIGHT * (2 + i));
        ssd1306_draw_string(buf);
    }
    
    ssd1306_display();
}
