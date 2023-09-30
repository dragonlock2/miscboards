#ifndef RF24_H_
#define RF24_H_

#include <stdbool.h>
#include <stdint.h>

void rf24_init(uint8_t address[5], uint8_t channel); // pipe0 initial address

bool rf24_irq_valid();

void rf24_start_rx();
void rf24_open_rx_pipe(uint8_t pipe, uint8_t address[5]);
uint8_t rf24_recv(uint8_t *data, uint8_t *data_len); // blocking, check rf24_irq_valid() first

void rf24_start_tx(uint8_t address[5]);
void rf24_send(uint8_t *data, uint8_t data_len); // non-blocking, check rf24_irq_valid()
bool rf24_tx_status(); // must call once rf24_irq_valid()

#endif /* RF24_H_ */
