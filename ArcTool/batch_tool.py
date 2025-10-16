import sys
import os
import ARCTool

def extract_arc(src, out):
    """
    Extracts an archive file to a specified output folder.
    
    Parameters:
    src (str): Path to the source archive file.
    out (str): Path to the output folder where files will be extracted.
    """
    saved_cwd = os.getcwd()  # Save current working directory because ARCTool changes it

    # Pass 1: Uncompress the archive to a temporary file
    temp = "./temp.arc"  # Intermediate uncompressed archive
    sys.argv = ["ARCTool", "-o", temp, src]
    ARCTool.main()

    # Pass 2: Extract uncompressed archive to the output folder
    sys.argv = ["ARCTool", "-o", out, temp]
    ARCTool.main()

    os.chdir(saved_cwd)  # Restore working directory

    # Delete intermediate file
    if os.path.exists(temp):
        os.remove(temp)
    else:
        print(f"The file {temp} does not exist")

src  = "D:/ztp_work/res/Stage/D_MN01A/STG_00.arc"
out2 = "D:/temp/STG_00_extracted"      # folder for final extraction

extract_arc(src, out2)