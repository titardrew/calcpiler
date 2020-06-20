#!/bin/bash

test_retval() {
    input=$1
    expected_output=$2

    build/compile $1 > build/temp_asm.s
    cc -o build/test_retval build/temp_asm.s
    build/test_retval
    output=$?

    if [ "${output}" = "${expected_output}" ]; then
        echo "${input} => ${output}"
    else
        echo "Test failed. Expected: ${expected_output}, Got: ${output}"
        exit 1
    fi
}

test_retval 0 0
test_retval 99 99
test_retval 255 255

echo "Success!"
