import sys
import arctool

archive_path = r"D:\ztp_work\res\Stage\D_MN01A\STG_00.arc"
destination  = r"D:\temp\test"

# simulate CLI: python arctool.py -o <destination> <archive_path>
sys.argv = ["arctool", "-o", destination, archive_path]
arctool.main()