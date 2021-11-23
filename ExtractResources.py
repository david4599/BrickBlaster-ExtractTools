# This script allows to extract resources (images, musics, etc) from blaster.exe
#
# The needed archive header-like (containing filenames, sizes and offsets) can be read 
# automatically by launching the game briefly and search it in memory
#
# This data can also be dumped manually in a file by using CheatEngine or a debugger 
# (RVA should be 0x21AC4 and size is 1060 bytes) and set it as the third command line argument
# 
# david4599 - 2021

import sys
import os
import time
import ctypes as c
from ctypes import wintypes as w

import win32api as wapi
import win32process as wproc
import win32con as wcon
from subprocess import Popen

from resources import Resources


k32 = c.WinDLL('kernel32', use_last_error=True)
ReadProcessMemory = k32.ReadProcessMemory
ReadProcessMemory.argtypes = [w.HANDLE,w.LPCVOID,w.LPVOID,c.c_size_t,c.POINTER(c.c_size_t)]
ReadProcessMemory.restype = w.BOOL


magic_find = b'\x45\x4F\x53'
res_header_base_address = 0x21AC4
res_header_size = 0x424 # 1060 bytes



def get_res_header(pid, rva, size):
    buffer = (c.c_byte * size)()
    bytes_read = c.c_ulong(0)
    
    module_handles = []
    timeout_count = 10
    for i in range(0, timeout_count):
        try:
            process_handle = wapi.OpenProcess(wcon.PROCESS_ALL_ACCESS, False, pid)
            module_handles = wproc.EnumProcessModules(process_handle)
            break
        except:
            if i == 9:
                raise
            else:
                pass
            
        time.sleep(0.5)
    
    if len(module_handles) <= 0:
        print("Error: No module handle found, dump the resources header data manually from memory (e.g. by using CheatEngine or a debugger)..")
        sys.exit(-1)
    
    base_address = module_handles[0]
    address = base_address + rva
    
    
    print("Trying to read resources header from memory (address: " + hex(address) + ")")
    magic_found = False
    timeout_count = 10
    for i in range(0, timeout_count):
        res = ReadProcessMemory(int(process_handle), c.c_void_p(address), buffer, len(buffer), c.byref(bytes_read))
        
        if bytearray(buffer[0:3]) == bytearray(magic_find):
            print("Resources header found!\n")
            magic_found = True
            break
        
        time.sleep(0.5)
    
    wapi.CloseHandle(process_handle)
    
    
    if not res:
        err = wapi.GetLastError()
        print("Failed Win32 API - Last Error: ", err)
        print("Error: Something gone wrong, dump the resources header data manually from memory (e.g. by using CheatEngine or a debugger).")
        sys.exit(-2)
        
    if not magic_found:
        return None
        
    return buffer



def main(argv):
    if len(argv) != 3 and len(argv) != 4:
        print("Usage: " + argv[0] + " <blaster.exe> <output folder> [blaster resources header file]")
        sys.exit(-3)
    
    
    blaster_exe_path = os.path.join(os.getcwd(), argv[1])
    output_path = os.path.join(os.getcwd(), argv[2])
    
    if len(argv) == 4:
        resources_path = os.path.join(os.getcwd(), argv[3])
    else:
        resources_path = os.path.join(os.getcwd(), "blaster_res_header.bin")
        
        print("The game will be launched briefly to read data from memory.")
        print("Starting the process in 5s...\n")
        time.sleep(5)
        
        proc = Popen([blaster_exe_path], cwd=os.path.dirname(os.path.abspath(blaster_exe_path)), stdin=None, stdout=None, stderr=None, shell=False)
        pid = proc.pid

        buffer = get_res_header(pid, res_header_base_address, res_header_size)
        
        if not buffer:
            print("Error: Couldn't read resources header!")
            sys.exit(-4)
        
        proc.terminate()
        
        with open(resources_path, "wb") as file:
            file.write(buffer)
        
    
    try: 
        os.makedirs(output_path)
    except OSError:
        if not os.path.isdir(output_path):
            print("Error: The output folder specified doesn't exist!")
            sys.exit(-5)
    
    
    resources = Resources.from_file(resources_path)

    num_files = resources.header.num_files
    print(str(num_files) + " files are indexed:")


    extracted_count = 0
    
    with open(blaster_exe_path, "rb") as blaster_exe:
        for file_entry in resources.files:
            filename = str(file_entry.name.strip("\x00"))
            print("\"" + filename + "\": " + str(file_entry.size) + " bytes (offset " + hex(file_entry.offset) + ")")
            
            
            blaster_exe.seek(file_entry.offset)
            file_content = blaster_exe.read(file_entry.size)
            
            
            with open(os.path.join(output_path, filename), "wb") as file:
                file.write(file_content)
                extracted_count += 1
    
    
    if extracted_count == num_files:
        print("\nSuccess: the " + str(extracted_count) + " indexed files are extracted!")
    else:
        print("\nError: Only " + str(extracted_count) + " indexed files are extracted of " + str(num_files) + "!")
    


if __name__ == "__main__":
    main(sys.argv)