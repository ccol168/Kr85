import os
import argparse
import re

def find_files_with_string(directory, run_name):
    """
    Find files that exactly match pattern: RUN<run_name>_YYYYMMDD.root
    """
    pattern = re.compile(rf'^RUN{run_name}_\d{{8}}\.root$')
    matching_files = []
    for root, dirs, files in os.walk(directory):
        for file in files:
            if pattern.match(file):
                matching_files.append(os.path.join(root, file))
    return matching_files

def extract_name(filepath):
    """
    Extract base name: RUNXXXX_YYYYMMDD from RUNXXXX_YYYYMMDD.root
    """
    filename = os.path.basename(filepath)
    match = re.match(r'^(RUN\d+_\d{8})\.root$', filename)
    if not match:
        raise ValueError(f"Filename {filename} does not match expected pattern")
    extracted_name = match.group(1)
    return extracted_name

def create_c_launch(RunName):
    # NEW: search in results directly
    directory = "/storage/gpfs_data/juno/junofs/users/caccianijuno/new_prod/results/"

    if not os.path.isdir(directory):
        print(f"----------------- Directory {directory} not found ----------------------------------------")
        return

    names_string = find_files_with_string(directory, RunName)

    if len(names_string) == 0:
        print(f"ERROR: No list file found for RUN{RunName}")
        return

    c_launch_file = open(f"c_launch/Launch_RUN{RunName}.sh", "w")

    for listname in names_string:
        RunID = extract_name(listname)  # now RUN5668_20250513
        os.system("python Make_sh_sub.py -inFile " + listname + " -RunID " + RunID)
        print("python Make_sh_sub.py -inFile " + listname + " -RunID " + RunID)
        c_launch_file.write(
            "condor_submit -spool -name sn01-htc.cr.cnaf.infn.it -batch-name "
            + RunID
            + " /storage/gpfs_data/juno/junofs/users/ccoletta/BiPo212/With_JVertex/sub/"
            + RunID
            + ".sub\n"
        )

    os.chmod(f"c_launch/Launch_RUN{RunName}.sh", 0o775)


parser = argparse.ArgumentParser(description="Launch a run from esd files, list in this dir")
parser.add_argument("-DatasetFile",help="Path of a file containing the list of runs to launch")
parser.add_argument("-DatasetName",help="Name of the resulting dataset")

args = parser.parse_args()


To_launch = []
with open(args.DatasetFile, 'r') as f:
    for line in f:
        if line.strip():  # skip empty lines
            first_value = line.split()[0]  # split by whitespace and take first item
            To_launch.append(first_value)

with open(f"c_launch/Launch_{args.DatasetName}.sh","w") as file :
    for element in To_launch :
        print(f"Launching RUN{element}")
        file.write(f"junosub Launch_RUN{element}.sh\n")
        create_c_launch(element)

os.chmod(f"c_launch/Launch_{args.DatasetName}.sh", 0o775)


