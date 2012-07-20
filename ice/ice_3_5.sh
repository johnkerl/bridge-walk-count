#!/bin/sh

### Set the job name
#PBS -N kerl_3_5

### Request email when job begins and ends
#PBS -m bea

### Specify email address to use for notification.
#PBS -M kerl@math.arizona.edu

### Specify the PI group found with va command
#PBS -W group_list=tgk

### Set the queue to submit this job.
#PBS -q default
#.BS -q windfall

#PBS -lselect=1:ncpus=1
#PBS -lplace=pack:shared
#PBS -l cput=60:0:0
#PBS -l walltime=60:0:0
#.BS -l mem=2GB

cd $HOME/pub_http_internet/bridge/walk_count/ice
mkdir -p ../databig
../count_walks slope=3/5 1-24 | right > ../databig/raw_counts_3_5.txt
