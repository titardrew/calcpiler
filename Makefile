CFLAGS=-std=c11 -g #-static Does not work on OSX 

BUILDDIR=build

all: makedir ${BUILDDIR}/compile

${BUILDDIR}/compile: src/compile.c
	cc ${CFLAGS} $^ -o $@

test:
	sh test.sh

clean:
	rm -f build/*

makedir:
	mkdir -p ${BUILDDIR}

.PHONY: test clean makedir
