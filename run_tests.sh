#!/bin/bash

if [[ ! -x ./bin/benchmark_cpu ]] || [[ ! -x ./bin/benchmark_gpu ]]; then
    echo "Building..."
    make
    if [[ $? != 0 ]]; then
        exit $?
    fi
fi

echo "Running tests..."
data_lens=(100 1000 10000 100000 1000000 10000000 100000000 1000000000 10000000000)
block_sizes=(10 100 1000 10000 100000 1000000 10000000)
for data_len in ${data_lens[@]}; do
    for block_size in ${block_sizes[@]}; do
        ./bin/benchmark_cpu ${data_len} ${block_size}
        ./bin/benchmark_gpu ${data_len} ${block_size}
    done
done
