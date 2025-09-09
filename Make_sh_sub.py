import argparse
import os

def make_sh (infile, RunID) :

    thispath = os.path.dirname(os.path.abspath(__file__))
    outfile = thispath + "/root/" + RunID + ".root"
    logfile = thispath + "/log/" + RunID + ".log"

    with open(thispath + "/sh/" + RunID + ".sh" , "w") as file:
        file.write("#!/bin/bash \n")
        file.write("source /cvmfs/juno.ihep.ac.cn/el9_amd64_gcc11/Release/Jlatest/setup.sh \n")
        file.write(f"{thispath}/JVertex_BiPo212/JVertex_BiPo212_reader {infile} CdEvents {outfile} &> {logfile} \n")

    os.chmod(thispath + "/sh/" + RunID + ".sh",0o774 )
    return

def make_sub (RunID) :

    thispath = os.path.dirname(os.path.abspath(__file__))
    subpath = thispath + "/sub/" + RunID + ".sub"

    with open(subpath,"w") as file:
        file.write("universe = vanilla \n")
        file.write("executable = " + thispath + "/sh/" + RunID + ".sh \n")
        file.write("log = "+ thispath + "/log/" + RunID + ".log \n")
        file.write("output = "+ thispath + "/out/" + RunID + ".out \n")
        file.write("error = "+ thispath + "/err/" + RunID + ".err \n")
        file.write("+MaxRuntime = 86400 \n")
        file.write("ShouldTransferFiles = YES \n")
        file.write("WhenToTransferOutput = ON_EXIT \n")
        file.write("+SingularityImage = false \n")
        file.write("queue 1 \n")

    return


parser = argparse.ArgumentParser(description="Make submission for a muon reco from a list of esd files")
parser.add_argument("-inFile", help="Name of the input list",required=True)
parser.add_argument("-RunID",help="Identifying name of this run",required=True)

args = parser.parse_args()

make_sh(args.inFile,args.RunID)
make_sub(args.RunID)



