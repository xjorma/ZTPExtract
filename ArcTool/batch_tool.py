import sys
import os
import ARCTool
from pathlib import Path

def is_yaz0(path: str) -> bool:
    """
    Check if a file is a Yaz0 archive based on its magic bytes.

    Parameters:
        path (str): Path to the file to check.

    Returns:
        bool: True if the file starts with 'Yaz0', False otherwise.
    """
    if not os.path.isfile(path):
        return False

    try:
        with open(path, "rb") as f:
            header = f.read(4)
            return header == b"Yaz0"
    except OSError:
        return False

def extract_arc(src, out):
    """
    Extracts an archive file to a specified output folder.
    
    Parameters:
    src (str): Path to the source archive file.
    out (str): Path to the output folder where files will be extracted.
    """
    saved_cwd = os.getcwd()  # Save current working directory because ARCTool changes it

    Path(out).mkdir(parents=True, exist_ok=True) # Ensure output directory exists
    if(is_yaz0(src)):
        # Pass 1: Uncompress the archive to a temporary file
        temp = str(Path("./temp.arc").resolve())  # Intermediate uncompressed archive
        sys.argv = ["ARCTool", "-o", temp, src]
        ARCTool.main()

        # Pass 2: Extract uncompressed archive to the output folder
        sys.argv = ["ARCTool", "-o", out, temp]
        ARCTool.main()
    else:
        # Directly extract the archive to the output folder
        sys.argv = ["ARCTool", "-o", out, src]
        ARCTool.main()

    os.chdir(saved_cwd)  # Restore working directory

    # Delete intermediate file
    if os.path.exists(temp):
        os.remove(temp)
    else:
        print(f"The file {temp} does not exist")

def extract_all(src_root, dst_root):
    src_root = Path(src_root).resolve()
    dst_root = Path(dst_root).resolve()

    # Gather candidates. Include .arc and .ARC for safety.
    paths = [p for p in src_root.rglob("*") if p.suffix.lower() == ".arc"]
    total = len(paths)
    print(f"Found {total} archives under {src_root}")

    if total == 0:
        return

    for i, arc_path in enumerate(paths, start=1):
        try:
            rel_parent = arc_path.parent.resolve().relative_to(src_root)
        except ValueError:
            # Fallback if file is not under src_root after resolution
            rel_parent = Path()

        out_folder = dst_root / rel_parent / arc_path.stem
        out_folder.mkdir(parents=True, exist_ok=True)

        print(f"Extracting {i}/{total}: {arc_path} -> {out_folder}")
        try:
            extract_arc(str(arc_path), str(out_folder))
        except Exception as ex:
            print(f"WARNING: failed to extract {arc_path}: {ex}")

if __name__ == "__main__":
    # example usage
    SRC = "D:/ztp_work/res/Object"
    DST = "D:/temp/object_extracted"
    extract_all(SRC, DST)