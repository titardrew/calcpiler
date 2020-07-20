# Calc-piler - [WIP] C-like compiler

I do some commits only in spare time and in certain mood. The main purpose is education.
At the moment, the compiler generates x86 assembly. Tested on OSx machine.

| Feature                     | Progress  |
|-----------------------------|-----------|
| Arithmetic/Binary/Unary ops | Partially |
| If/else                     | Done      |
| For/While                   | Done      |
| Scopes                      | Done      |
| Functions                   | Done      |
| Types + Pointers            | TBD       |
| Arrays, Floats, Strings     | TBD       |
| Comments                    | TBD       |
| Preprocessor                | TBD       |
| ...                         |           |


# Build and run tests
```bash
    make test
```
You should see the colored test output.

# Example
Current example contains simple hello-world program, that calculates 21st entry in the fibonacci sequence via recursion and loops.
Run:
```bash
    sh compile.sh "-i example.c" example
```
You may see the generated assembly in `example_x86.s`. There's a lot of work to be done to make it look less crappy, though.
