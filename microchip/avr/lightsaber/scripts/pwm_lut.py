import numpy as np

PWM_PERIOD = 400
LUT_LENGTH = 256

if __name__ == '__main__':
    map = np.linspace(-0.5, 0.5, num=LUT_LENGTH)
    lut  = np.round((map + 0.5) * PWM_PERIOD).astype(np.uint16)

    print(f'const uint16_t PWM_LUT[{LUT_LENGTH}] = {{')
    for r in np.split(lut, LUT_LENGTH // 16):
        print('    ' + ', '.join([f'{i:3}' for i in r]) + ',')
    print(f'}};')
