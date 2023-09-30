#ifndef LSM303AH_H_
#define LSM303AH_H_

#include <stdint.h>

typedef struct {
    int16_t ax, ay, az;
    int16_t mx, my, mz;
} lsm303ah_result_S;

void lsm303ah_init();
void lsm303ah_read(lsm303ah_result_S *result);

#endif /* LSM303AH_H_ */
