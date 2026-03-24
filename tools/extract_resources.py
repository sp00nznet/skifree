#!/usr/bin/env python3
"""
Extract resources from ski32.exe (SkiFree v1.04) into the resources/ directory.

Extracts bitmap sprites and (if present) WAV sound effects.

Usage:
    python extract_resources.py <path_to_ski32.exe>

For sound effects, use Foone's modified ski32.exe which has WAVs embedded:
    Download from: https://web.archive.org/web/20240324032244/http://foone.org/downloads/skifree/ski32sounds.zip

For the original game (sprites only):
    Download from: https://ski.ihoc.net/

Requires: pip install pefile
"""

import sys
import os
import struct

# Sound slot names from Foone's research
SOUND_NAMES = {
    1: "ouch",    # crash into tree/rock
    2: "whee",    # jump off bump
    3: "woof",    # dog barks
    4: "oof",     # land from air
    5: "dude",    # snowboarder crash
    6: "myhair",  # beginner crash
    7: "gobble",  # yeti eats player
    8: "piddle",  # dog pees
    9: "argh",    # yeti nearby
}


def extract_with_pefile(exe_path, output_dir):
    """Extract bitmap and WAV resources using pefile library."""
    try:
        import pefile
    except ImportError:
        print("ERROR: pefile not installed. Run: pip install pefile")
        sys.exit(1)

    pe = pefile.PE(exe_path)
    os.makedirs(output_dir, exist_ok=True)

    bitmap_count = 0
    sound_count = 0

    if not hasattr(pe, 'DIRECTORY_ENTRY_RESOURCE'):
        print("ERROR: No resources found in", exe_path)
        sys.exit(1)

    for resource_type in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        if resource_type.id == 2:  # RT_BITMAP
            for resource_id_entry in resource_type.directory.entries:
                for resource_lang in resource_id_entry.directory.entries:
                    data_rva = resource_lang.data.struct.OffsetToData
                    size = resource_lang.data.struct.Size
                    data = pe.get_memory_mapped_image()[data_rva:data_rva + size]

                    rid = resource_id_entry.id
                    filename = os.path.join(output_dir, f"ski32_{rid}.bmp")

                    # Bitmap resources don't include BITMAPFILEHEADER — add it
                    bfh_size = 14
                    bi_size = struct.unpack_from('<I', data, 0)[0]
                    bit_count = struct.unpack_from('<H', data, 14)[0]

                    if bit_count <= 8:
                        colors_used = struct.unpack_from('<I', data, 32)[0]
                        if colors_used == 0:
                            colors_used = 1 << bit_count
                        color_table_size = colors_used * 4
                    else:
                        color_table_size = 0

                    pixel_offset = bfh_size + bi_size + color_table_size
                    file_size = bfh_size + len(data)
                    bfh = struct.pack('<2sIHHI', b'BM', file_size, 0, 0, pixel_offset)

                    with open(filename, 'wb') as f:
                        f.write(bfh)
                        f.write(data)

                    bitmap_count += 1
                    print(f"  Extracted bitmap #{rid} -> {filename}")

        elif resource_type.id == 3:  # RT_ICON
            for resource_id_entry in resource_type.directory.entries:
                for resource_lang in resource_id_entry.directory.entries:
                    data_rva = resource_lang.data.struct.OffsetToData
                    size = resource_lang.data.struct.Size
                    data = pe.get_memory_mapped_image()[data_rva:data_rva + size]
                    print(f"  Found icon resource #{resource_id_entry.id}")

        # RT_RCDATA (10) or custom "WAVE" type — check for WAV resources
        # Foone's modified exe embeds sounds as named WAVE resources
        elif resource_type.id == 10 or (resource_type.name and str(resource_type.name) == "WAVE"):
            sounds_dir = os.path.join(output_dir, "..", "sounds")
            os.makedirs(sounds_dir, exist_ok=True)

            for resource_id_entry in resource_type.directory.entries:
                for resource_lang in resource_id_entry.directory.entries:
                    data_rva = resource_lang.data.struct.OffsetToData
                    size = resource_lang.data.struct.Size
                    data = pe.get_memory_mapped_image()[data_rva:data_rva + size]

                    rid = resource_id_entry.id
                    name = SOUND_NAMES.get(rid, f"sound_{rid}")
                    filename = os.path.join(sounds_dir, f"{name}.wav")

                    with open(filename, 'wb') as f:
                        f.write(data)

                    sound_count += 1
                    print(f"  Extracted sound #{rid} ({name}) -> {filename}")

    # Check for named string resources that might be WAVs
    # Some PE files store WAVs under custom resource types by name
    for resource_type in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        if resource_type.name:
            type_name = str(resource_type.name)
            if type_name.upper() in ("WAVE", "WAV", "SOUND"):
                sounds_dir = os.path.join(output_dir, "..", "sounds")
                os.makedirs(sounds_dir, exist_ok=True)
                for resource_id_entry in resource_type.directory.entries:
                    for resource_lang in resource_id_entry.directory.entries:
                        data_rva = resource_lang.data.struct.OffsetToData
                        size = resource_lang.data.struct.Size
                        data = pe.get_memory_mapped_image()[data_rva:data_rva + size]

                        if resource_id_entry.name:
                            name = str(resource_id_entry.name).lower()
                        elif resource_id_entry.id:
                            rid = resource_id_entry.id
                            name = SOUND_NAMES.get(rid, f"sound_{rid}")
                        else:
                            name = f"unknown_{sound_count}"

                        filename = os.path.join(sounds_dir, f"{name}.wav")
                        with open(filename, 'wb') as f:
                            f.write(data)

                        sound_count += 1
                        print(f"  Extracted sound ({name}) -> {filename}")

    # Font reminder
    font_path = os.path.join(output_dir, "vgaoem.fon")
    if not os.path.exists(font_path):
        print(f"\n  NOTE: You need a VGA OEM font at {font_path}")
        print(f"  On Windows: copy C:\\Windows\\Fonts\\vgaoem.fon")
        print(f"  On other platforms: any small monospace .ttf will work")

    print(f"\nExtracted {bitmap_count} bitmaps, {sound_count} sounds to {output_dir}/")
    if sound_count == 0:
        print("\nNo sounds found in this exe. For sound effects, use Foone's modified version:")
        print("  https://web.archive.org/web/20240324032244/http://foone.org/downloads/skifree/ski32sounds.zip")
    print("\nYou're ready to build! Run: cmake -B build && cmake --build build")


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    exe_path = sys.argv[1]
    if not os.path.exists(exe_path):
        print(f"ERROR: File not found: {exe_path}")
        sys.exit(1)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_dir = os.path.join(os.path.dirname(script_dir), "resources")

    print(f"Extracting resources from {exe_path}...")
    extract_with_pefile(exe_path, output_dir)
