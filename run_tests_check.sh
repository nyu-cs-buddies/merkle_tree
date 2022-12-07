#!/bin/bash
# This is a test script that run benchmarks with different configs on
# both CPU and GPU. The resulting root hashes will be checked whether
# they match.

if [[ ! -x ./bin/benchmark_cpu ]] || [[ ! -x ./bin/benchmark_gpu ]]; then
    echo "Building..."
    make
    if [[ $? != 0 ]]; then
        exit $?
    fi
fi

echo "Running tests..."
# data_lens=(100 1000 10000 100000 1000000 10000000 100000000 1000000000 10000000000)
data_lens=(100 1000 10000 100000 1000000 10000000 100000000 1000000000)
# block_sizes=(100 1000 10000 100000 1000000 10000000)
# block_sizes=(32 50 64 100 128 200 256 400 512 800 1024)
block_sizes=(30 32 60 64 120 128 250 256 500 512 1000 1024)
for data_len in ${data_lens[@]}; do
    for block_size in ${block_sizes[@]}; do
	cpu_results=($(./bin/benchmark_cpu ${data_len} ${block_size} 2>&1))
	echo ${cpu_results[1]}
	gpu_results=($(./bin/benchmark_gpu ${data_len} ${block_size} 2>&1))
	echo ${gpu_results[1]}
	if [[ ${cpu_results[0]} != ${gpu_results[0]} ]]; then
	    echo "Root hashes from CPU and GPU do not match!"
	    echo "CPU: ${cpu_results[0]}"
	    echo "GPU: ${gpu_results[0]}"
	fi
    done
done

