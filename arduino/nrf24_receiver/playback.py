import numpy as np
import sounddevice as sd
import serial
import time

FS = 22e3 # 22kHz

s = serial.Serial('COM6', 1000000, timeout=0.05)
time.sleep(2.0) # roughly Arduino reboot time

print("starting capture!")
s.flush()
start = time.time()
audio = b''
while time.time() - start < 5.0:
    audio += s.read(4096)

print("playing back!")
audio = np.array(list(audio), dtype=np.float64)
audio -= 109 # DC offset
audio /= np.max(np.abs(audio))
sd.play(audio, FS) # some noise from RF?
time.sleep(len(audio) / FS)
sd.stop()
