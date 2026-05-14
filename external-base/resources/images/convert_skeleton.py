#!/usr/bin/env python3
"""Convert PNG image file to C++ header with embedded binary data."""
import os

def png_to_header(png_path, header_path, var_name):
    with open(png_path, 'rb') as f:
        data = f.read()
    
    with open(header_path, 'w') as f:
        f.write('#pragma once\n')
        f.write(f'// Auto-generated from {os.path.basename(png_path)}\n')
        f.write(f'// Image data embedded as binary array\n\n')
        f.write(f'const unsigned int {var_name}_size = {len(data)};\n\n')
        f.write(f'const unsigned char {var_name}_data[] = {{\n    ')
        
        # Write 16 bytes per line
        for i, byte in enumerate(data):
            if i > 0:
                if i % 16 == 0:
                    f.write(',\n    ')
                else:
                    f.write(', ')
            f.write(f'0x{byte:02x}')
        
        f.write('\n};\n')
    
    print(f"Generated {header_path} ({len(data)} bytes)")

if __name__ == '__main__':
    # Convert skeleton image
    png_file = r'c:\Users\User\Desktop\blood strike external\external-base\resources\images\skeleton.png'
    header_file = r'c:\Users\User\Desktop\blood strike external\external-base\src\skeleton_embedded.hpp'
    
    if not os.path.exists(png_file):
        print(f"Error: {png_file} not found")
        exit(1)
    
    png_to_header(png_file, header_file, 'skeleton_png')
