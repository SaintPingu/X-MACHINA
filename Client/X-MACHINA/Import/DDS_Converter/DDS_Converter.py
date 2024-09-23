import os
import subprocess

INPUT_PATH_BC1 = 'Input_BC1/'
INPUT_PATH_BC3 = 'Input_BC3/'
INPUT_PATH_BC7 = 'Input_BC7/'
INPUT_PATH_UI = 'Input_UI/'

INPUT_CONVERTED_PATH_BC1 = 'Input_converted_BC1/'
INPUT_CONVERTED_PATH_BC3 = 'Input_converted_BC3/'
INPUT_CONVERTED_PATH_BC7 = 'Input_converted_BC7/'
INPUT_CONVERTED_PATH_UI = 'Input_converted_UI/'
OUTPUT_PATH_BC1 = 'Output_BC1/'
OUTPUT_PATH_BC3 = 'Output_BC3/'
OUTPUT_PATH_BC7 = 'Output_BC7/'
OUTPUT_PATH_UI = 'Output_UI/'

os.makedirs(INPUT_CONVERTED_PATH_BC1, exist_ok=True)
os.makedirs(INPUT_CONVERTED_PATH_BC3, exist_ok=True)
os.makedirs(INPUT_CONVERTED_PATH_BC7, exist_ok=True)
os.makedirs(INPUT_CONVERTED_PATH_UI, exist_ok=True)


def get_files(input_path, output_path):
    if not os.path.isdir(input_path):
        os.makedirs(input_path)
        os.makedirs(output_path, exist_ok=True)
    return os.listdir(input_path)

os.chdir(os.path.dirname(os.path.abspath(__file__)))

files = get_files(INPUT_PATH_BC1, OUTPUT_PATH_BC1)
for file in files:
    file_path = INPUT_PATH_BC1 + file
    cmd = texconv_cmd = 'texconv.exe -srgb -pow2 -f BC1_UNORM ' + file_path
    subprocess.run(texconv_cmd, shell=True, cwd=os.getcwd())
    
    os.replace(file_path, INPUT_CONVERTED_PATH_BC1 + file)
    
    dds_file = file[:-4] + '.dds'
    os.replace(dds_file, OUTPUT_PATH_BC1 + dds_file)

files = get_files(INPUT_PATH_BC3, OUTPUT_PATH_BC3)
for file in files:
    file_path = INPUT_PATH_BC3 + file
    cmd = texconv_cmd = 'texconv.exe -srgb -pow2 -f BC3_UNORM ' + file_path
    subprocess.run(texconv_cmd, shell=True)
    
    os.replace(file_path, INPUT_CONVERTED_PATH_BC3 + file)
    
    dds_file = file[:-4] + '.dds'
    os.replace(dds_file, OUTPUT_PATH_BC3 + dds_file)

files = get_files(INPUT_PATH_BC7, OUTPUT_PATH_BC7)
for file in files:
    file_path = INPUT_PATH_BC7 + file
    cmd = texconv_cmd = 'texconv.exe -srgb -pow2 -f BC7_UNORM ' + file_path
    subprocess.run(texconv_cmd, shell=True)
    
    os.replace(file_path, INPUT_CONVERTED_PATH_BC7 + file)
    
    dds_file = file[:-4] + '.dds'
    os.replace(dds_file, OUTPUT_PATH_BC7 + dds_file)

files = get_files(INPUT_PATH_UI, OUTPUT_PATH_UI)
for file in files:
    file_path = INPUT_PATH_UI + file
    cmd = texconv_cmd = 'texconv.exe -srgb ' + file_path
    subprocess.run(texconv_cmd, shell=True, cwd=os.getcwd())
    
    os.replace(file_path, INPUT_CONVERTED_PATH_UI + file)
    
    dds_file = file[:-4] + '.dds'
    os.replace(dds_file, OUTPUT_PATH_UI + dds_file)

import ctypes  # An included library with Python install.   
ctypes.windll.user32.MessageBoxW(0, "Successfully converted", "DDSConverter", 0)