#!/bin/bash

test_retval() {
    input="$1"
    expected_output="$2"

    build/compile "${input}" > build/temp_asm.s
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

test_comp() {
    input="$1"
    expected_output="$2"

    build/compile "${input}" > build/temp_asm.s
    output=$?

    if [ "${output}" = "${expected_output}" ]; then
        echo "Exitcode: ${output}"
    else
        echo "Test failed. Expected: ${expected_output}, Got: ${output}"
        exit 1
    fi
}

test_retval 0 0
test_retval "99+1-5+10-3" 102
test_retval " 99 + 1 - 9 +    9 " 100
test_comp "99 ++ 1" 1

echo "Success!"
