#!/bin/bash
# Bash script used to collect data on performance of particle simulation for different time intervals.

declare -a total_time_interval=(10, 100, 1000, 10000)

pos=$(( ${#total_time_interval[*]} - 1 ))
last=${total_time_interval[$pos]}

filename_parallel="benchmark_parallel_time.txt"

echo "total_time_interval" "," "time_ns" > $filename_parallel

# Test with constant delta_t for parallel
delta_t=0.5
for i in "${total_time_interval[@]}"
do
sbatch <<-_EOF
#!/bin/bash
#SBATCH --job-name=PPtime${i}
#SBATCH --ntasks-per-node=1
#SBATCH --nodes=1
#SBATCH --time=23:59:59
#SBATCH --output=./output/pp_${i}.out
#SBATCH --error=./errors/err_p_${i}.err
#SBATCH --partition=gpu
#SBATCH --mem=128000

echo "total_time_interval=${i}"
echo "delta_t=${delta_t}"

module load use.own
module load gcc/6.1.0
module load pgi

# run the experiment
srun ./particles_parallel n=10000 delta=$delta_t total_time_interval=$i
_EOF
done







