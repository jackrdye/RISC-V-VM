#!/bin/bash

input_dir="./tests"
output_dir="./tests/actual_outputs"

for input_file in $input_dir/*.in; do
    echo $(basename $input_file)
    # Expected output
    expected_output_file="${input_file%.in}.out"

    # Actual output
    output_file=${output_dir}/$(basename ${input_file%.in}.out)
    
    # output_file="${output_dir}${input_file%.in}.out"
    ./output < $input_file > ${output_file}

    # Produce the actual output and compare to the expected output
    if diff "${expected_output_file}" "$output_file" > /dev/null; then
        echo "   Passed"
    else
        echo "   FAILED"
    fi
    
done


