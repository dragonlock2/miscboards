import sounddevice as sd
import matplotlib.pyplot as plt

if __name__ == '__main__':
    fs  = 48000 # Hz
    dur = 0.1 # s

    rec = sd.rec(int(fs * dur), samplerate=fs, channels=8, dtype='int16', device='rvice_adc')
    sd.wait()

    for s in rec.T:
        print(min(s), max(s))
        plt.plot(s)
    plt.show()
