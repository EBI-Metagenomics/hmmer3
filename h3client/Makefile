CC     ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -O3 -MMD -MP
PREFIX ?= /usr/local
SRC = $(filter-out $(wildcard test_*.c),$(wildcard *.c))
OBJ = $(SRC:.c=.o)
HDR = $(wildcard h3c_*.h)
LIB = libh3client.a

all: $(LIB)

$(LIB): $(OBJ)
	ar rcs $@ $^

-include $(SRC:.c=.d)

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

test_h3client: %: %.o $(LIB) $(HDR)
	$(CC) $(CFLAGS) $< -L. -lh3client -lh3result -llio -llite_pack -lm -o $@

check: test_h3client
	./start_daemon.sh
	./test_h3client && ./stop_daemon.sh || { ./stop_daemon.sh; exit 1; }

install: $(LIB) $(HDR)
	@mkdir -p $(PREFIX)/lib $(PREFIX)/include
	install -m 0755 $(LIB) $(PREFIX)/lib/
	install -m 0644 $(HDR) $(PREFIX)/include/

uninstall:
	rm -f $(PREFIX)/lib/$(LIB) $(HDR:%=$(PREFIX)/include/%)

.PHONY: all clean check uninstall
clean:
	rm -f $(OBJ) $(LIB) test_h3client.o test_h3client *.d
	./stop_daemon.sh
