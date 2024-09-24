import os

def is_source_file(filename):
    """
    Check if the file is a C++ source/header/module file based on its extension.
    """
    source_extensions = {
        '.cpp', '.hpp', '.h', '.ixx', '.cppm', '.cxx', '.c++', '.cc', '.c',
        '.hxx', '.hh', '.h++'
    }
    _, ext = os.path.splitext(filename)
    return ext.lower() in source_extensions

def consolidate_source_files(directory, output_file, exclude_list):
    """
    Traverse the directory, find all source files, and write their contents to the output file.
    Each file's content is preceded by a header indicating the filename.
    """
    with open(output_file, 'w', encoding='utf-8') as outfile:
        for root, _, files in os.walk(directory):
            for file in files:
                if is_source_file(file) and file not in exclude_list:
                    file_path = os.path.join(root, file)
                    relative_path = os.path.relpath(file_path, directory)
                    header = f"\n{'='*80}\n// File: {relative_path}\n{'='*80}\n\n"
                    outfile.write(header)
                    try:
                        with open(file_path, 'r', encoding='utf-8') as infile:
                            content = infile.read()
                            outfile.write(content)
                            outfile.write("\n\n")  # Add some spacing after each file
                    except Exception as e:
                        print(f"Error reading {file_path}: {e}")

    print(f"Consolidation complete! All source files have been written to '{output_file}'.")

if __name__ == "__main__":
    import argparse

    # Set up command-line argument parsing
    parser = argparse.ArgumentParser(description="Consolidate C++ source files into a single text file.")
    parser.add_argument(
        "directory",
        type=str,
        help="Path to the solution directory containing source files."
    )
    parser.add_argument(
        "-o", "--output",
        type=str,
        default="consolidated_code.txt",
        help="Name of the output text file. Default is 'consolidated_code.txt'."
    )

    parser.add_argument(
        "-e", "--exclude",
        type=list,
        default=[],
        help="list of files to exclude"
    )

    args = parser.parse_args()

    # Validate directory path
    if not os.path.isdir(args.directory):
        print(f"Error: The directory '{args.directory}' does not exist or is not a directory.")
        exit(1)

    # Call the consolidation function
    consolidate_source_files(args.directory, args.output, args.exclude)
