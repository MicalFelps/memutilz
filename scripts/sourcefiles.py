import os
import fnmatch
import argparse

def find_files(root_dir, pattern):
    matches = []
    for root, dirs, files in os.walk(root_dir):
        for name in files:
            if fnmatch.fnmatch(name, pattern):
                matches.append(os.path.join(root, name))
    return matches

def main():
    parser = argparse.ArgumentParser(description="Recursively find files matching a pattern")
    parser.add_argument("directory", help="Directory to scan")
    parser.add_argument("pattern", help="Filename pattern")
    args = parser.parse_args()

    matches = find_files(args.directory, args.pattern)
    for path in matches:
        index = path.find("src")
        print(path[index:])

if __name__ == "__main__":
    main()
