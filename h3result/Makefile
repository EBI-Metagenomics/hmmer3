CC     ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -O3 -MMD -MP
PREFIX ?= /usr/local
SRC = $(filter-out $(wildcard test_*.c),$(wildcard *.c))
OBJ = $(SRC:.c=.o)
HDR = $(wildcard *.h)
LIBHDR = $(wildcard h3r_*.h)
LIB = libh3result.a

all: $(LIB)

$(LIB): $(OBJ)
	ar rcs $@ $^

-include $(SRC:.c=.d)

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

test_h3result: %: %.o $(LIB) $(HDR)
	$(CC) $(CFLAGS) $< -L. -lh3result -llio -llite_pack -lm -o $@

check: test_h3result
	./test_h3result

install: $(LIB) $(LIBHDR)
	@mkdir -p $(PREFIX)/lib $(PREFIX)/include
	install -m 0755 $(LIB) $(PREFIX)/lib/
	install -m 0644 $(LIBHDR) $(PREFIX)/include/

uninstall:
	rm -f $(PREFIX)/lib/$(LIB) $(HDR:%=$(PREFIX)/include/%)

.PHONY: all clean check uninstall
clean:
	rm -f $(OBJ) $(LIB) test_h3result.o test_h3result *.d
