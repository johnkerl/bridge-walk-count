#!/bin/sh

Nmin=1
Nmax=24
fracs="0/1 1/10 1/9 1/8 1/7 1/6 1/5 2/9 2/7 3/10 1/3 3/8 2/5 3/7 4/9 1/2 5/9 \
4/7 3/5 5/8 2/3 7/10 5/7 7/9 4/5 5/6 6/7 7/8 8/9 9/10 1/1"

for frac in $fracs; do
	frac_name_for_file=`echo $frac | sed 's:/:_:'`
	script_name=ice_${frac_name_for_file}.sh
	cat > $script_name <<@@@
#!/bin/sh

### Set the job name
#PBS -N kerl_${frac_name_for_file}

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

cd \$HOME/pub_http_internet/bridge/walk_count/ice
mkdir -p ../databig
../count_walks slope=$frac $Nmin-$Nmax | right > ../databig/raw_counts_${frac_name_for_file}.txt
@@@
	chmod u+x $script_name
done
