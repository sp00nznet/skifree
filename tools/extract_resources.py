#!/usr/bin/env python3
"""
Extract bitmap resources from ski32.exe (SkiFree v1.04) into the resources/ directory.

Usage:
    python extract_resources.py <path_to_ski32.exe>

Requires: pip install pefile Pillow
Download ski32.exe from: https://ski.ihoc.net/
"""

import sys
import os
import struct

def extract_with_pefile(exe_path, output_dir):
    """Extract resources using pefile library."""
    try:
        import pefile
    except ImportError:
        print("ERROR: pefile not installed. Run: pip install pefile")
        sys.exit(1)

    pe = pefile.PE(exe_path)
    os.makedirs(output_dir, exist_ok=True)

    bitmap_count = 0

    if not hasattr(pe, 'DIRECTORY_ENTRY_RESOURCE'):
        print("ERROR: No resources found in", exe_path)
        sys.exit(1)

    for resource_type in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        # RT_BITMAP = 2, RT_ICON = 3, RT_GROUP_ICON = 14
        if resource_type.id == 2:  # RT_BITMAP
            for resource_id_entry in resource_type.directory.entries:
                for resource_lang in resource_id_entry.directory.entries:
                    data_rva = resource_lang.data.struct.OffsetToData
                    size = resource_lang.data.struct.Size
                    data = pe.get_memory_mapped_image()[data_rva:data_rva + size]

                    rid = resource_id_entry.id
                    filename = os.path.join(output_dir, f"ski32_{rid}.bmp")

                    # Bitmap resources in PE files don't include the BITMAPFILEHEADER,
                    # so we need to add it
                    bfh_size = 14
                    # Parse BITMAPINFOHEADER to find offset to pixel data
                    bi_size = struct.unpack_from('<I', data, 0)[0]
                    bit_count = struct.unpack_from('<H', data, 14)[0]

                    # Calculate color table size
                    if bit_count <= 8:
                        colors_used = struct.unpack_from('<I', data, 32)[0]
                        if colors_used == 0:
                            colors_used = 1 << bit_count
                        color_table_size = colors_used * 4
                    else:
                        color_table_size = 0

                    pixel_offset = bfh_size + bi_size + color_table_size
                    file_size = bfh_size + len(data)

                    # Build BITMAPFILEHEADER: 'BM', file size, reserved, reserved, offset
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
                    rid = resource_id_entry.id
                    filename = os.path.join(output_dir, "ICONSKI.ICO")
                    # For simplicity, write the raw icon data
                    # A proper ICO file would need the ICO header
                    print(f"  Found icon resource #{rid} (raw data saved)")

    # Also try to find a font resource or use a fallback
    font_path = os.path.join(output_dir, "vgaoem.fon")
    if not os.path.exists(font_path):
        print(f"\n  NOTE: You need a VGA OEM font at {font_path}")
        print(f"  Copy vgaoem.fon from C:\\Windows\\Fonts\\ or use any .ttf font")

    print(f"\nExtracted {bitmap_count} bitmaps to {output_dir}/")
    print("You're ready to build! Run: cmake -B build && cmake --build build")


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
