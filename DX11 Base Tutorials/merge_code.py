import pathlib
import argparse
import chardet
from typing import List, Optional

def detect_encoding(file_path: pathlib.Path) -> str:
    with file_path.open('rb') as file:
        raw = file.read(10000)  # Read up to 10000 bytes
    return chardet.detect(raw)['encoding']

def merge_files(directory: pathlib.Path, extensions: List[str], output_file: pathlib.Path) -> None:
    total_files = sum(1 for ext in extensions for _ in directory.rglob(f'*{ext}'))
    processed_files = 0

    try:
        with output_file.open('w', encoding='utf-8') as outfile:
            for extension in extensions:
                for file_path in directory.rglob(f'*{extension}'):
                    try:
                        encoding = detect_encoding(file_path)
                        with file_path.open('r', encoding=encoding) as infile:
                            outfile.write(f"\n\n// File: {file_path}\n")
                            outfile.write(infile.read())
                        processed_files += 1
                        print(f"Progress: {processed_files}/{total_files} files processed", end='\r')
                    except Exception as e:
                        print(f"\nError processing {file_path}: {str(e)}")
    except IOError as e:
        print(f"Error writing to output file: {str(e)}")
    
    print(f"\nFiles have been merged into {output_file}")

def main(directory: Optional[str] = None, 
         extensions: Optional[List[str]] = None, 
         output: Optional[str] = None) -> None:
    if directory is None:
        directory = '.'
    if extensions is None:
        extensions = ['.h', '.cpp']
    if output is None:
        output = 'merged_cpp_files.cpp'

    directory_path = pathlib.Path(directory).resolve()
    output_path = pathlib.Path(output).resolve()

    if not directory_path.is_dir():
        raise ValueError(f"'{directory}' is not a valid directory")

    merge_files(directory_path, extensions, output_path)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Merge C++ files into a single file.")
    parser.add_argument("-d", "--directory", help="Directory to search for files (default: current directory)")
    parser.add_argument("-e", "--extensions", nargs='+', default=['.h', '.cpp'], help="File extensions to include (default: .h .cpp)")
    parser.add_argument("-o", "--output", default="merged_cpp_files.cpp", help="Output file name (default: merged_cpp_files.cpp)")
    args = parser.parse_args()

    main(args.directory, args.extensions, args.output)