# This script convert the IFF sounds to the WAV format (resampled to 12963Hz to have the correct speed and tone)
#
# It uses SoX (https://sourceforge.net/projects/sox/) to convert them
# 
# david4599 - 2021

import sys
import os
from subprocess import check_output


extension_original = ".IFF"
extension_replace = ".WAV"

sox_exe = "sox.exe"



def main(argv):
    if len(argv) != 3:
        print("Usage: " + argv[0] + " <input folder> <output folder>")
        sys.exit(-1)
    
    
    input_path = os.path.join(os.getcwd(), argv[1])
    output_path = os.path.join(os.getcwd(), argv[2])
    
    
    if not os.path.exists(input_path):
        print("The input folder specified doesn't exist!")
        sys.exit(-2)
    
    
    try: 
        os.makedirs(output_path)
    except OSError:
        if not os.path.isdir(output_path):
            print("The output folder specified doesn't exist!")
            sys.exit(-3)
    
    
    converted_count = 0
    
    for filename in os.listdir(input_path):
        if not filename.upper().endswith(extension_original):
            continue
    
        input_file_path = os.path.join(input_path, filename.upper())
        
        filename_no_ext = os.path.splitext(filename.upper())[0]
        output_file_path = os.path.join(output_path, filename_no_ext + extension_replace)
        output_tempfile_path = os.path.join(output_path, "_tmp_" + filename_no_ext + extension_replace)
    
        try:
            print("Converting \"" + str(filename) + "\" to \"" + str(filename_no_ext + extension_replace) + "\"")
            
            # Converting to 8363Hz sample rate wav first and then resampling wav to avoid using raw SoX option that interprets file header as sound
            check_output("sox -t 8svx \"" + input_file_path + "\" \"" + output_tempfile_path + "\"", stderr=None, shell=False)
            
            # Original sample rate seems to be around 12963Hz to the ear (8363Hz IFF (8svx format) sample rate * 1.55)
            check_output("sox -r 12963 \"" + output_tempfile_path + "\" \"" + output_file_path + "\"", stderr=None, shell=False)
            os.remove(output_tempfile_path)
            converted_count += 1
        except FileNotFoundError:
            print("\"" + sox_exe + "\" couldn't be found.")
            print("Please install SoX (https://sourceforge.net/projects/sox/) and add its install folder to your PATH environment variable")
            sys.exit(-4)
        except:
            print("Error: Something gone wrong with the file \"" + input_file_path + "\"")
    
    print("\n" + str(converted_count) + " converted files!")
    


if __name__ == "__main__":
    main(sys.argv)