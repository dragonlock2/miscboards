#define F_CPU 8000000L

#include <xc.h>
#include <stdbool.h>
#include <util/delay.h>

/* constants and defines */
#define LIN_LED_PID (0xc1) // 0x01
#define LIN_LED_LEN (1)

#define LIN_BTN_PID (0x42) // 0x02
#define LIN_BTN_LEN (1)

// precomputed enhanced checksums
#define LIN_BTN_PRESSED_CHECKSUM     (0xbc)
#define LIN_BTN_NOT_PRESSED_CHECKSUM (0xbd)

// tuned for 19.2kbps
#define LIN_TX_DELAY      (129)
#define LIN_RX_DELAY      (125)
#define LIN_RX_HALF_DELAY (64)

#define LIN_SYNC (0x55)

/* global variables */
typedef struct {
	bool breaked;
	uint8_t data[8];
	uint8_t checksum;
} lin_data_S;
static lin_data_S lin_data;

/* app functions */
static inline void app_init() {
	// LED on PB0, phase-correct 8-bit PWM
	DDRB   |= (1 << DDB0);
	TCCR0A |= (1 << COM0A1) | (1 << WGM00);
	TCCR0B |= (1 << CS00);
	OCR0A = 1; // start at min brightness

	// BTN on PB1
	PUEB |= (1 << PUEB1);
}

static inline void app_led_write(uint8_t duty) {
	OCR0A = duty;
}

static inline bool app_btn_read() {
	return !((PINB >> PINB1) & 0x1);
}

static inline void app_led_process() {
	uint16_t c = LIN_LED_PID + lin_data.data[0] + lin_data.checksum; // enhanced
	while (c >= 256) {
		c -= 255;
	}
	if (c == 0xFF) {
		app_led_write(lin_data.data[0]);
	}
}

static inline void app_btn_process() {
	bool p = app_btn_read();
	lin_data.data[0] = p;
	lin_data.checksum = p ? LIN_BTN_PRESSED_CHECKSUM : LIN_BTN_NOT_PRESSED_CHECKSUM;
}

/* LIN functions */
static inline void lin_od_high() {
	DDRB &= ~(1 << DDB2); // input
}

static inline void lin_od_low() {
	DDRB |= (1 << DDB2); // output
}

static inline bool lin_od_read() {
	return (PINB >> PINB2) & 0x1;
}

static void lin_write_byte(uint8_t b) {
	// start bit
	lin_od_low();
	_delay_loop_1(LIN_TX_DELAY);
	// data, LSB first
	for (int i = 0; i < 8; i++) {
		b & 0x1 ? lin_od_high() : lin_od_low();
		b >>= 1;
		_delay_loop_1(LIN_TX_DELAY);
	}
	// stop bit
	lin_od_high();
	_delay_loop_1(LIN_TX_DELAY);
}

static uint8_t lin_read_byte() {
	// start bit
	while (lin_od_read());
	_delay_loop_1(LIN_RX_HALF_DELAY);
	// data, LSB first
	uint8_t b = 0;
	for (int i = 0; i < 8; i++) {
		_delay_loop_1(LIN_RX_DELAY);
		b >>= 1;
		b |= lin_od_read() << 7;
	}
	// stop bit (break if not there)
	_delay_loop_1(LIN_RX_DELAY);
	lin_data.breaked = !lin_od_read();
	_delay_loop_1(LIN_RX_HALF_DELAY);
	while (!lin_od_read());
	return b;
}

static inline void lin_init() {
	PORTB &= ~(1 << DDB2); // force low when an output
	lin_od_high();
}

static inline void lin_loop() {
	uint8_t b = 0x69;
	while (true) {
lin_loop_start:
		// break
		while (!lin_data.breaked || b != 0) {
			b = lin_read_byte();
		}
		// sync
		b = lin_read_byte();
		if (b != LIN_SYNC || lin_data.breaked) {
			goto lin_loop_start;
		}
		// pid
		b = lin_read_byte();
		if (lin_data.breaked) {
			goto lin_loop_start;
		}
		// data + checksum
		if (b == LIN_LED_PID) {
			for (int i = 0; i < LIN_LED_LEN; i++) {
				b = lin_read_byte();
				lin_data.data[i] = b;
				if (lin_data.breaked) {
					goto lin_loop_start;
				}
			}
			b = lin_read_byte();
			lin_data.checksum = b;
			if (lin_data.breaked) {
				goto lin_loop_start;
			}
			app_led_process();
		} else if (b == LIN_BTN_PID) {
			app_btn_process();
			for (int i = 0; i < LIN_BTN_LEN; i++) {
				lin_write_byte(lin_data.data[0]);
			}
			lin_write_byte(lin_data.checksum);
		}
	}
}

int main(void) {
	// prescaler to 1 (8MHz)
	CCP = 0xD8;
	CLKPSR = 0x00;

	app_init();
	lin_init();

	lin_loop();
}
