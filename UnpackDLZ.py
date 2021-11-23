# This script unpacks files that were compressed with diet packer
#
# It uses a program called undiet.exe that can be compiled from the sources in the ".\undiet\src" folder
# The compiled undiet.exe must be located in the ".\undiet\bin" folder
# 
# david4599 -2021

import sys
import os
from subprocess import check_output


# extensions = (".MOD", ".IFF", ".FLC", ".CFG", ".LV0", ".LV1")

magic_find =    b'\xB4\x4C\xCD\x21\x9D\x89\x45\x4F\x53'
magic_replace = b'\xB4\x4C\xCD\x21\x9D\x89\x64\x6C\x7A'

undiet_exe = "undiet.exe"
undiet_exe_path = os.path.join(os.getcwd(), "undiet", "bin", undiet_exe)



def main(argv):
    if len(argv) != 2:
        print("Usage: " + argv[0] + " <input folder>")
        # print("    -> Supported types: *.MOD, *.IFF, *.FLC, *.CFG, *.LV0, *.LV1")
        sys.exit(-1)
    
    
    if not os.path.isfile(undiet_exe_path):
        print("\"" + undiet_exe_path + "\" doesn't exist.")
        print("Please compile it from the sources and place the exe file at this location: \"" + undiet_exe_path + "\"")
        sys.exit(-2)
    
    
    input_path = os.path.join(os.getcwd(), argv[1])
    
    
    if not os.path.exists(input_path):
        print("The input folder specified doesn't exist!")
        sys.exit(-3)

    
    unpacked_count = 0
    
    for filename in os.listdir(input_path):
        # if not filename.upper().endswith(extensions):
        #     continue
        
        input_file_path = os.path.join(input_path, filename)
        
        with open(input_file_path, "r+b") as file:
            magic = file.read(len(magic_find))
            
            # Replace "EOS" by the original "dlz" letters inside the header of compressed MOD, IFF and FLC files
            if magic == magic_find:
                print("Fixing DLZ header of \"" + str(filename) + "\"")
                
                magic = magic_replace
                file.seek(0)
                file.write(magic)
                file.flush()
            
            if magic == magic_replace:
                print("Unpacking \"" + str(filename) + "\"")
                
                try:
                    check_output(undiet_exe_path + " \"" + input_file_path + "\" \"" + input_file_path + "\"", stderr=None, shell=False)
                    unpacked_count += 1
                except:
                    print("Error: Something gone wrong with the file \"" + input_file_path + "\"")
                
            
    print("\n" + str(unpacked_count) + " unpacked files!")
    


if __name__ == "__main__":
    main(sys.argv)