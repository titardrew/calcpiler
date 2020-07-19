fib_rec(n) {
    if (n <= 1) return n;
    return fib_rec(n-1) + fib_rec(n-2);
}

fib_loop(n) {
    if (n == 0) return 0;
    fib_0 = 0;
    fib_1 = 1;
    for (i = 1; i < n; i = i + 1) {
        fib_1 = fib_0 + fib_1;
        fib_0 = fib_1 - fib_0;
    }
    return fib_1; 
}

print_num(num) {
    if (num > 0) {
        print_num(num / 10);
        _putchar(num % 10 + 48);
    }
}

print_hello_world() {
    _putchar(72);
    _putchar(101);
    _putchar(108);
    _putchar(108);
    _putchar(111);
    _putchar(44);
    _putchar(32);
    _putchar(87);
    _putchar(111);
    _putchar(114);
    _putchar(108);
    _putchar(100);
    _putchar(33);
}

main() {
    print_hello_world();
    _putchar(32);
    print_num(fib_rec(21));
    _putchar(32);
    print_num(fib_loop(21));
    _putchar(10);
    return 0;
}

