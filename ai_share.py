import os

def is_source_file(filename):
    """
    Check if the file is a C++ source/header/module/project file based on its extension.
    """
    source_extensions = {
        '.cpp', '.hpp', '.h', '.ixx', '.cppm', '.cxx', '.c++', '.cc', '.c',
        '.hxx', '.hh', '.h++', '.vcxproj'
    }
    _, ext = os.path.splitext(filename)
    return ext.lower() in source_extensions


def parse_list(list_str):
    """
    Parse a comma-separated string into a list of items.
    """
    if not list_str:
        return []
    return [item.strip() for item in list_str.split(',') if item.strip()]


def add_file_to_output(file_path, outfile):
    """
    Add a specific file to the output file.
    """
    header = f"\n{'='*80}\n// File: {file_path}\n{'='*80}\n\n"
    outfile.write(header)
    try:
        with open(file_path, 'r', encoding='utf-8') as infile:
            content = infile.read()
            outfile.write(content)
            outfile.write("\n\n")  # Add some spacing after each file
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return  # Skip printing success message if there's an error

    print(f"File '{file_path}' has been added to '{outfile.name}'.")


def consolidate_source_files(directories, output_file, exclude_list, specific_list, recursive=True):
    """
    Traverse the list of directories, find all source files,
    and write their contents to the output file.
    Each file's content is preceded by a header indicating the filename.
    """
    with open(output_file, 'w', encoding='utf-8') as outfile:
        # Traverse each directory in the list
        for directory in directories:
            if not os.path.isdir(directory):
                print(f"Warning: The directory '{directory}' does not exist or is not a directory. Skipping.")
                continue

            print(f"Processing directory: {directory}")
            if recursive:
                # Traverse directories recursively using os.walk
                for root, _, files in os.walk(directory):
                    for file in files:
                        if is_source_file(file) and file not in exclude_list:
                            file_path = os.path.join(root, file)
                            add_file_to_output(file_path, outfile)
            else:
                # Non-recursive traversal using os.listdir
                try:
                    for file in os.listdir(directory):
                        file_path = os.path.join(directory, file)
                        if os.path.isfile(file_path) and is_source_file(file) and file not in exclude_list:
                            add_file_to_output(file_path, outfile)
                except Exception as e:
                    print(f"Error accessing directory '{directory}': {e}")

        # Add specific files regardless of directories
        for file in specific_list:
            if file and os.path.isfile(file):
                add_file_to_output(file, outfile)
            else:
                print(f"Specific file '{file}' does not exist in the provided directories and will be skipped.")

    print(f"Consolidation complete! All source files have been written to '{output_file}'.")


if __name__ == "__main__":
    import argparse

    # Set up command-line argument parsing
    parser = argparse.ArgumentParser(description="Consolidate C++ source files from multiple directories into a single text file.")
    parser.add_argument(
        "directories",
        type=str,
        nargs='+',  # Allows one or more directories
        help="Path(s) to the solution directories containing source files."
    )
    parser.add_argument(
        "-o", "--output",
        type=str,
        default="consolidated_code.txt",
        help="Name of the output text file. Default is 'consolidated_code.txt'."
    )

    parser.add_argument(
        "-e", "--exclude",
        type=str,
        default="",
        help="Comma-separated list of files to exclude."
    )

    parser.add_argument(
        "-s", "--specific",
        type=str,
        default="",
        help="Comma-separated list of specific files to include."
    )

    parser.add_argument(
        "-nr", "--no-recursive",
        action='store_true',
        help="Disable recursive traversal of directories."
    )

    args = parser.parse_args()

    # Parse the exclude and specific lists
    exclude_files = parse_list(args.exclude)
    specific_files = parse_list(args.specific)

    # Set recursion based on the '--no-recursive' flag
    recursive = not args.no_recursive  # Defaults to True unless '--no-recursive' is specified

    # Call the consolidation function
    consolidate_source_files(args.directories, args.output, exclude_files, specific_files, recursive=recursive)
