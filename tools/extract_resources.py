#!/usr/bin/env python3
"""
Extract the sprites (and, if present, the sounds) from ski32.exe into resources/
and sounds/. Python 3 standard library only.

    python3 tools/extract_resources.py /path/to/ski32.exe

ski32.exe: https://ski.ihoc.net/ (the author's site). For sound effects, use
Alice Averlong's ski32sounds.exe, which has the WAVs patched back in:
https://web.archive.org/web/20240324032244/http://foone.org/downloads/skifree/ski32sounds.zip
"""

import os
import shutil
import struct
import sys

# Sound slot names from Alice Averlong's research
SOUND_NAMES = {1: "ouch", 2: "whee", 3: "woof", 4: "oof", 5: "dude",
               6: "myhair", 7: "gobble", 8: "piddle", 9: "argh"}
RT_BITMAP, RT_RCDATA = 2, 10


def resources(data):
    """Yield (type, id_or_name, bytes) for every resource in a PE32 image."""
    pe = struct.unpack_from('<I', data, 0x3C)[0]
    if data[pe:pe + 4] != b'PE\0\0':
        raise ValueError("not a Win32 (PE) program")
    nsect, optsize = struct.unpack_from('<H', data, pe + 6)[0], struct.unpack_from('<H', data, pe + 20)[0]
    opt = pe + 24
    rsrc_rva = struct.unpack_from('<I', data, opt + 96 + 2 * 8)[0]  # data directory 2: resources
    sections = [struct.unpack_from('<IIII', data, opt + optsize + i * 40 + 8) for i in range(nsect)]

    def off(rva):
        for vsize, va, rawsize, raw in sections:
            if va <= rva < va + max(vsize, rawsize):
                return rva - va + raw
        raise ValueError("RVA %x is in no section" % rva)

    base = off(rsrc_rva)

    def entries(d):
        named, ids = struct.unpack_from('<HH', data, base + d + 12)
        for i in range(named + ids):
            name, target = struct.unpack_from('<II', data, base + d + 16 + i * 8)
            if name & 0x80000000:
                p = base + (name & 0x7FFFFFFF)
                n = struct.unpack_from('<H', data, p)[0]
                name = data[p + 2:p + 2 + n * 2].decode('utf-16-le')
            yield name, target

    for rtype, t in entries(0):
        for rid, r in entries(t & 0x7FFFFFFF):
            for _lang, leaf in entries(r & 0x7FFFFFFF):
                rva, size = struct.unpack_from('<II', data, base + leaf)
                yield rtype, rid, data[off(rva):off(rva) + size]


def main(exe):
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    res_dir, snd_dir = os.path.join(root, 'resources'), os.path.join(root, 'sounds')
    os.makedirs(res_dir, exist_ok=True)
    with open(exe, 'rb') as f:
        data = f.read()
    bitmaps = sounds = 0
    for rtype, rid, blob in resources(data):
        if rtype == RT_BITMAP:
            # A bitmap resource is a BMP without its 14-byte file header: put it back.
            hsize, bits, used = struct.unpack_from('<I', blob, 0)[0], struct.unpack_from('<H', blob, 14)[0], struct.unpack_from('<I', blob, 32)[0]
            palette = (used or 1 << bits) * 4 if bits <= 8 else 0
            head = struct.pack('<2sIHHI', b'BM', 14 + len(blob), 0, 0, 14 + hsize + palette)
            with open(os.path.join(res_dir, 'ski32_%d.bmp' % rid), 'wb') as f:
                f.write(head + blob)
            bitmaps += 1
        elif rtype == RT_RCDATA or (isinstance(rtype, str) and rtype.upper() in ('WAVE', 'WAV', 'SOUND')):
            if blob[:4] != b'RIFF':
                continue
            name = rid.lower() if isinstance(rid, str) else SOUND_NAMES.get(rid, 'sound_%d' % rid)
            os.makedirs(snd_dir, exist_ok=True)
            with open(os.path.join(snd_dir, name + '.wav'), 'wb') as f:
                f.write(blob)
            sounds += 1
    if bitmaps == 0:
        sys.exit("No sprites in %s: is that SkiFree's ski32.exe?" % exe)
    # The status window's font. Windows has the original; elsewhere the game
    # falls back to the ProggyClean.ttf beside it.
    fon = os.path.join(os.environ.get('WINDIR', r'C:\Windows'), 'Fonts', 'vgaoem.fon')
    if os.name == 'nt' and os.path.exists(fon):
        shutil.copy(fon, res_dir)
    print("Extracted %d sprites and %d sounds from %s" % (bitmaps, sounds, exe))


if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    main(sys.argv[1])
