import sys
import os
import ARCTool

src  = "D:/ztp_work/res/Stage/D_MN01A/STG_00.arc"
temp = "./temp.arc"        # intermediate uncompressed archive
out2 = "D:/temp/STG_00_extracted"      # folder for final extraction

print(f"cwd: {os.getcwd()}")

saved_cwd = os.getcwd()

# pass 1: unyaz to file
sys.argv = ["ARCTool", "-o", temp, src]
ARCTool.main()

# pass 2: extract uncompressed archive to folder
sys.argv = ["ARCTool", "-o", out2, temp]
ARCTool.main()

os.chdir(saved_cwd)
# delete intermediate file

print(f"cwd: {os.getcwd()}")

if os.path.exists(temp):
    os.remove(temp)
else:
    print(f"The file {temp} does not exist")