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

# Add/Sub
test_retval 0 0
test_retval "99+1-5+10-3" 102
test_retval " 99 + 1 - 9 +    9 " 100
test_comp "99 # 1" 1
# Mul/Div
test_retval "5 + 6*7" 47
test_retval "5*(9 - 6)" 15
test_retval "(3 + 5)/2" 4
test_comp "[5 + 1]#2" 1
# Unary:
test_retval "-( -3 + 8)/-5" 1
test_retval "99 ++ 1" 100
# Mod:
test_retval "100 % 3" 1

echo "Success!"
