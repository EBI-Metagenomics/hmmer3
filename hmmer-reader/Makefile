CC     ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -O3 -MMD -MP
PREFIX ?= /usr/local
SRC = $(filter-out $(wildcard test_*.c),$(wildcard *.c))
OBJ = $(SRC:.c=.o)
HDR = hmmer_reader.h
LIB = libhmmer_reader.a

all: $(LIB)

$(LIB): $(OBJ)
	ar rcs $@ $^

-include $(SRC:.c=.d)

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

test_hmr: %: %.o $(LIB) $(HDR)
	$(CC) $(CFLAGS) $< -L. -lhmmer_reader -o $@

check: test_hmr
	./test_hmr

install: $(LIB) $(HDR)
	@mkdir -p $(PREFIX)/lib $(PREFIX)/include
	install -m 0755 $(LIB) $(PREFIX)/lib/
	install -m 0644 $(HDR) $(PREFIX)/include/

uninstall:
	rm -f $(PREFIX)/lib/$(LIB) $(HDR:%=$(PREFIX)/include/%)

.PHONY: all clean check uninstall
clean:
	rm -f $(OBJ) $(LIB) test_hmr.o test_hmr *.d
