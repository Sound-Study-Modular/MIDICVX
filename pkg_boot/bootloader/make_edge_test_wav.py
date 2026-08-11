#!/usr/bin/env python3
import argparse, math, struct, wave

p=argparse.ArgumentParser(description='Generate MIDICVX CLOCK-jack bootloader input test WAV')
p.add_argument('-o','--output', default='midicvx_clock_input_test.wav')
p.add_argument('--seconds', type=float, default=10.0)
p.add_argument('--frequency', type=float, default=1000.0)
p.add_argument('--rate', type=int, default=48000)
p.add_argument('--level', type=float, default=0.95)
a=p.parse_args()
level=max(0.0,min(1.0,a.level))
amp=int(32767*level)
frames=int(a.rate*a.seconds)
with wave.open(a.output,'wb') as w:
    w.setnchannels(2); w.setsampwidth(2); w.setframerate(a.rate)
    buf=bytearray()
    for i in range(frames):
        v=amp if math.sin(2*math.pi*a.frequency*i/a.rate)>=0 else -amp
        buf += struct.pack('<hh',v,v)
    w.writeframes(buf)
print(a.output)
