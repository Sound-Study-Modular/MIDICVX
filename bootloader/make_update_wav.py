#!/usr/bin/env python3
"""MIDICVX Intel-HEX -> audio update WAV encoder.

Protocol v1 matches bootloader.c:
  - 48 kHz, mono, 16-bit PCM
  - edge interval preamble: 125 us
  - sync marker: 2500 us without an edge, then an edge
  - bit 0: 250 us edge interval
  - bit 1: 500 us edge interval
  - bytes are LSB-first
  - packets carry CRC16-CCITT
  - complete application carries CRC32
"""
import argparse
import struct
import wave
import zlib
from pathlib import Path

SAMPLE_RATE = 48000
AMP = 30000
PREAMBLE_SAMPLES = 6
ZERO_SAMPLES = 12
ONE_SAMPLES = 24
SYNC_SAMPLES = 120
PACKET_GAP_SAMPLES = 720
LEAD_SILENCE = SAMPLE_RATE // 4
TAIL_SILENCE = SAMPLE_RATE // 2
PREAMBLE_EDGES = 160
PAGE_SIZE = 128
DATA_CHUNK_SIZE = 32
APP_LIMIT = 0x7000
PACKET_MANIFEST = 0x10
PACKET_DATA = 0x20
PACKET_END = 0x30

def parse_ihex(path: Path) -> bytes:
    mem = bytearray([0xFF] * APP_LIMIT); upper = 0; highest = -1
    for lineno, raw in enumerate(path.read_text().splitlines(), 1):
        line = raw.strip()
        if not line: continue
        if not line.startswith(':'): raise ValueError(f"line {lineno}: not Intel HEX")
        rec = bytes.fromhex(line[1:]); count = rec[0]; addr = (rec[1] << 8) | rec[2]; rtype = rec[3]; data = rec[4:4+count]
        if len(data) != count or ((sum(rec) & 0xFF) != 0): raise ValueError(f"line {lineno}: bad length/checksum")
        if rtype == 0x00:
            base = upper + addr; end = base + count
            if end > APP_LIMIT: raise ValueError(f"application data reaches 0x{end-1:04X}; bootloader starts at 0x7000")
            mem[base:end] = data
            if count: highest = max(highest, end - 1)
        elif rtype == 0x01: break
        elif rtype == 0x02: upper = int.from_bytes(data, 'big') << 4
        elif rtype == 0x04: upper = int.from_bytes(data, 'big') << 16
        elif rtype not in (0x03, 0x05): raise ValueError(f"line {lineno}: unsupported record type {rtype:02X}")
    if highest < 0: raise ValueError("HEX contains no application data")
    return bytes(mem[:highest + 1])

def crc16_ccitt(data: bytes) -> int:
    crc = 0xFFFF
    for b in data:
        crc ^= b << 8
        for _ in range(8): crc = ((crc << 1) ^ 0x1021) & 0xFFFF if (crc & 0x8000) else (crc << 1) & 0xFFFF
    return crc

def packet(ptype: int, sequence: int, payload: bytes) -> bytes:
    if len(payload) > DATA_CHUNK_SIZE: raise ValueError("packet payload too large")
    body = bytes((ptype, sequence & 0xFF, (sequence >> 8) & 0xFF, len(payload))) + payload
    return body + struct.pack('<H', crc16_ccitt(body))

class WaveBuilder:
    def __init__(self): self.samples = []; self.level = -AMP
    def hold(self, n): self.samples.extend([self.level] * n)
    def silence(self, n): self.samples.extend([0] * n)
    def edge(self): self.level = AMP if self.level < 0 else -AMP
    def preamble_sync(self):
        for _ in range(PREAMBLE_EDGES): self.hold(PREAMBLE_SAMPLES); self.edge()
        self.hold(SYNC_SAMPLES); self.edge()
    def bit(self, value): self.hold(ONE_SAMPLES if value else ZERO_SAMPLES); self.edge()
    def byte(self, value):
        for i in range(8): self.bit((value >> i) & 1)
    def emit_packet(self, raw):
        self.preamble_sync()
        for b in raw: self.byte(b)
        self.hold(PACKET_GAP_SAMPLES)
    def write(self, path):
        with wave.open(str(path), 'wb') as w:
            w.setnchannels(1); w.setsampwidth(2); w.setframerate(SAMPLE_RATE)
            raw = bytearray()
            for s in self.samples: raw += struct.pack('<h', s)
            w.writeframes(raw)

def parse_version(text):
    parts = text.split('.')
    if len(parts) != 3: raise argparse.ArgumentTypeError("version must be MAJOR.MINOR.PATCH")
    vals = tuple(int(x) for x in parts)
    if any(x < 0 or x > 255 for x in vals): raise argparse.ArgumentTypeError("version fields must be 0..255")
    return vals

def main():
    ap = argparse.ArgumentParser(); ap.add_argument('hex', type=Path); ap.add_argument('-o','--output',type=Path); ap.add_argument('--version',type=parse_version,default=(1,0,0)); args=ap.parse_args()
    app=parse_ihex(args.hex); app_crc=zlib.crc32(app)&0xFFFFFFFF; major,minor,patch=args.version
    manifest=b'MXU1'+bytes((1,))+struct.pack('<H',len(app))+struct.pack('<I',app_crc)+bytes((major,minor,patch))
    wb=WaveBuilder(); wb.silence(LEAD_SILENCE); manifest_packet=packet(PACKET_MANIFEST,0xFFFF,manifest)
    for _ in range(16): wb.emit_packet(manifest_packet)
    pages=(len(app)+PAGE_SIZE-1)//PAGE_SIZE; chunks=(len(app)+DATA_CHUNK_SIZE-1)//DATA_CHUNK_SIZE
    for seq in range(chunks):
        chunk=app[seq*DATA_CHUNK_SIZE:(seq+1)*DATA_CHUNK_SIZE]; data_packet=packet(PACKET_DATA,seq,chunk); wb.emit_packet(data_packet); wb.emit_packet(data_packet)
    end_packet=packet(PACKET_END,chunks,b''); wb.emit_packet(end_packet); wb.emit_packet(end_packet); wb.silence(TAIL_SILENCE)
    out=args.output or args.hex.with_suffix('.wav'); wb.write(out); duration=len(wb.samples)/SAMPLE_RATE
    print(f"Application: {len(app)} bytes\nCRC32:       0x{app_crc:08X}\nPages:       {pages}\nChunks:      {chunks}\nWAV:         {out}\nDuration:    {duration:.1f} s")

if __name__ == '__main__': main()
