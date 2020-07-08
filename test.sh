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
        echo ${input}
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
test_retval "0;" 0
test_retval "99+1-5+10-3;" 102
test_retval " 99 + 1 - 9 +    9 ;" 100
test_comp "99 # 1;" 1
# Mul/Div
test_retval "5 + 6*7;" 47
test_retval "5*(9 - 6);" 15
test_retval "(3 + 5)/2;" 4
test_comp "[5 + 1]#2;" 1
# Unary:
test_retval "-( -3 + 8)/-5;" 1
test_retval "99 ++ 1;" 100
# Mod:
test_retval "100 % 3;" 1
# Comparison:
test_retval "0 < 1;" 1
test_retval "0 > 1;" 0
test_retval "1 <= 0;" 0
test_retval "1 >= 0;" 1
test_retval "(10 > 3) + (1 + 9 <= 10) != 1;" 1
test_retval "2 + 1 == -1 + 4 ;" 1 
test_retval "(1 != 1) >= 1;" 0 
# Variables:
test_retval "a = 3 * 5;" 15
test_retval "var1 = 3 * 5; var_2 = var1 + var1 / 5; return var_2;" 18
# If-else:
test_retval "v1 = 5; v2 = 6; if (v1 < v2) return v2; else v2 = 20; return v2;" 6
test_retval "v1 = 5; v2 = 6; if (v1 > v2) return v2; else v2 = 20; return v2;" 20
# Loops:
test_retval "v1 = 1; while (v1 < 10) v1 = v1 + 1; return v1;" 10
test_retval "v1 = 0; for (i=1; i<10; i=i+1) v1 = v1 + i; return v1;" 45
test_retval "i = 1; for (;i<10;) i = i + 1; return i;" 10

echo "Success!"
