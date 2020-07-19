build/compile "${1}" > build/temp_asm.s
cc -o build/${2} build/temp_asm.s 
build/${2}
