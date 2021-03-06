CFLAGS=-std=c11 -g #-static Does not work on OSX 
BUILDDIR=build
SOURCES=$(wildcard src/*.c)
INCLUDE=$(wildcard src/*.h)
OBJECTS=$(patsubst src/*.c,build/%.o,$(SOURCES))

all: makedir $(OBJECTS)
	$(CC) -o $(BUILDDIR)/compile $(OBJECTS) $(LDFLAGS) -g #-DDEBUG_PARSER #-DDEBUG_LEXER

$(OBJECTS): $(INCLUDE)

test: all
	sh test.sh

clean:
	rm -f build/*

makedir:
	mkdir -p ${BUILDDIR}

.PHONY: test clean makedir
