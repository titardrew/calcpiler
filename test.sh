#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

test_retval() {
    input="$1" expected_output="$2"

    build/compile "${input}" > build/temp_asm.s
    cc -o build/test_retval build/temp_asm.s 
    build/test_retval
    output=$?

    if [ "${output}" = "${expected_output}" ]; then
        echo "${input} ${GREEN}=> ${output}${NC}"
    else
        echo ${input}
        echo "${RED}Test failed. Expected: ${expected_output}, Got: ${output}${NC}"
        exit 1
    fi
}

test_stdout() {
    input="$1" expected_output="$2"

    build/compile "${input}" > build/temp_asm.s
    cc -o build/test_retval build/temp_asm.s 
    output=$(build/test_retval)

    if [ "${output}" = "${expected_output}" ]; then
        echo "${input} ${GREEN}=> ${output}${NC}"
    else
        echo ${input}
        echo "${RED}Test failed. Expected: ${expected_output}, Got: ${output}${NC}"
        exit 1
    fi
}

test_comp() {
    input="$1"
    expected_output="$2"

    build/compile "${input}" > build/temp_asm.s
    output=$?

    if [ "${output}" = "${expected_output}" ]; then
        echo "${GREEN}Exitcode: ${output}${NC}"
    else
        echo "${RED}Test failed. Expected: ${expected_output}, Got: ${output}${NC}"
        exit 1
    fi
}

echo "\n${YELLOW}Add/Sub:${NC}"
test_retval "main() { return 0; }" 0
test_retval "main() { return 99+1-5+10-3; }" 102
test_retval "main() { return 99 + 1 - 9 +    9 ; }" 100

echo "\n${YELLOW}Mul/Div:${NC}"
test_retval "main() { return 5 + 6*7; }" 47
test_retval "main() { return 5*(9 - 6); }" 15
test_retval "main() { return (3 + 5)/2; }" 4
test_retval "main() { return 100 % 3; }" 1

echo "\n${YELLOW}Errors:${NC}"
test_comp "main() { return 99 # 1; }" 1
test_comp "main() { return (5 + 1) / 2 }" 1
test_comp "main() { return 1; " 1
test_comp "main() { { i = 21; } return i; }" 1
test_comp "main() { i=i+1; return 0; }" 1
test_comp "main() { i+j=1; return 0; }" 1

echo "\n${YELLOW}Unary:${NC}"
test_retval "main() { return -( -3 + 8)/-5; }" 1
test_retval "main() { return 99 ++ 1; }" 100

echo "\n${YELLOW}Comparison:${NC}"
test_retval "main() { return 0 < 1; }" 1
test_retval "main() { return 0 > 1; }" 0
test_retval "main() { return 1 <= 0; }" 0
test_retval "main() { return 1 >= 0; }" 1
test_retval "main() { return (10 > 3) + (1 + 9 <= 10) != 1; }" 1
test_retval "main() { return 2 + 1 == -1 + 4 ; }" 1 
test_retval "main() { return (1 != 1 ) >= 1; }" 0 

echo "\n${YELLOW}Variables:${NC}"
test_retval "main() { return a = 3 * 5; }" 15
test_retval "main() { var1 = 3 * 5; var_2 = var1 + var1 / 5; return var_2; }" 18

echo "\n${YELLOW}If-else:${NC}"
test_retval "main() { v1 = 5; v2 = 6; if (v1 < v2) return v2; else v2 = 20; return v2; }" 6
test_retval "main() { v1 = 5; v2 = 6; if (v1 > v2) return v2; else v2 = 20; return v2; }" 20

echo "\n${YELLOW}Loops:${NC}"
test_retval "main() { v1 = 1; while (v1 < 10) v1 = v1 + 1; return v1; }" 10
test_retval "main() { v1 = 0; for (i=1; i<10; i=i+1) v1 = v1 + i; return v1; }" 45
test_retval "main() { i = 1; v1 = 0; for (;i<10;) {i=i+1; v1=v1+i;} return v1; }" 54
test_retval "main() { v1=0; for (i=1; i<10; i=i+1) if (i<5) v1=v1+i; else v1=v1+1; return v1; }" 15

echo "\n${YELLOW}Scopes:${NC}"
test_retval "main() { i = 12; { i = 21; } return i; }" 21

echo "\n${YELLOW}Functions:${NC}"
test_retval "foo() { i = 1; i = i + 11; return i; } main() { i = foo() + foo(); return i; }" 24
test_retval "calc(a, b, c) { a = a + b; return a*c; } main() { a = 3; return calc(a, 1, a + 2); }" 20
test_retval "fib(n) { if (n <= 1) return n; return fib(n-1) + fib(n-2); } main() { return fib(10); }" 55
test_stdout "print_num(num) { if (num > 0) { print_num(num / 10); _putchar(num % 10 + 48); } } main() { ptr = 455; print_num(ptr); return 0; }" 455

test_stdout "main() { _putchar(72); _putchar(101); _putchar(108); _putchar(108); _putchar(111); _putchar(44); _putchar(32); _putchar(87); _putchar(111); _putchar(114); _putchar(108); _putchar(100); _putchar(33); return 0; }" "Hello, World!"

echo "\n${GREEN}Success!${NC}"
