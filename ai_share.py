# This script is used to create a single txt file contianing the entire source. this is usefull for copy-pasting into browser-based AI chatbots.

import argparse
from pathlib import Path

SOURCE_EXTENSIONS = {
        '.cpp', '.hpp', '.h', '.ixx', '.cppm', '.cxx', '.c++', '.cc', '.c',
        '.hxx', '.hh', '.h++', '.vcxproj'
    }

def parse_list(list_str):
    return [item.strip() for item in list_str.split(',')] if list_str else []

def should_exclude(file_path, exclude_list):
    return any(exclude in str(file_path) for exclude in exclude_list)

def collect_files(directories, exclude_list, recursive):
    pattern = '**/*' if recursive else '*'
    files = []
    for directory in directories:
        path = Path(directory)
        if not path.is_dir():
            print(f"Warning: '{directory}' is not a directory. Skipping.")
            continue
        files.extend(
            f for f in path.glob(pattern)
            if f.is_file() and f.suffix.lower() in SOURCE_EXTENSIONS and not should_exclude(f, exclude_list)
        )
    return files

def add_file_to_output(file_path, outfile):
    header = f"\n{'='*80}\n// File: {file_path}\n{'='*80}\n\n"
    outfile.write(header)
    with file_path.open('r', encoding='utf-8') as infile:
        outfile.write(infile.read())
    outfile.write("\n\n")
    print(f"Added '{file_path}'.")

def main():
    parser = argparse.ArgumentParser(description="Consolidate source files into a single text file.")
    parser.add_argument("directories", nargs='+', help="Directories containing source files.")
    parser.add_argument("-o", "--output", default="consolidated_code.txt", help="Output file name.")
    parser.add_argument("-e", "--exclude", default="", help="Comma-separated list of files/directories to exclude.")
    parser.add_argument("-s", "--specific", default="", help="Comma-separated list of specific files to include.")
    parser.add_argument("-nr", "--no-recursive", action='store_true', help="Disable recursive traversal.")
    args = parser.parse_args()

    exclude_list = parse_list(args.exclude)
    specific_files = parse_list(args.specific)
    recursive = not args.no_recursive

    files_to_add = set(collect_files(args.directories, exclude_list, recursive))
    for file in specific_files:
        file_path = Path(file)
        if file_path.is_file():
            files_to_add.add(file_path)
        else:
            print(f"Specific file '{file}' does not exist. Skipping.")

    with open(args.output, 'w', encoding='utf-8') as outfile:
        for file_path in sorted(files_to_add):
            add_file_to_output(file_path, outfile)

    print(f"Consolidation complete! All source files have been written to '{args.output}'.")

if __name__ == '__main__':
    main()

