import sounddevice as sd
import matplotlib.pyplot as plt

if __name__ == '__main__':
    fs  = 48000 # Hz
    dur = 0.1 # s

    rec = sd.rec(int(fs * dur), samplerate=fs, channels=1, dtype='int16', device='rvice_adc')
    sd.wait()

    print(min(rec), max(rec))

    # plt.plot(rec)
    # plt.show()
