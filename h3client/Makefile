CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -O3 -MMD -MP
SRC = $(filter-out $(wildcard test_*.c),$(wildcard *.c))
OBJ = $(SRC:.c=.o)
HDR = $(wildcard h3c_*.h)
LIB = libh3client.a
PREFIX ?= /usr/local

all: $(LIB)

$(LIB): $(OBJ)
	ar rcs $@ $^

-include $(SRC:.c=.d)

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

test_h3client: %: %.o $(LIB) $(HDR)
	$(CC) $(CFLAGS) $< -L. -lh3client -lh3result -llio -llite_pack -lm -o $@

check: test_h3client
	./start_daemon.sh data/ross.5.hmm
	./test_h3client
	./stop_daemon.sh data/ross.5.hmm

install: $(LIB) $(HDR)
	@mkdir -p $(PREFIX)/lib $(PREFIX)/include
	install -m 0755 $(LIB) $(PREFIX)/lib/
	install -m 0644 $(HDR) $(PREFIX)/include/

uninstall:
	rm -f $(PREFIX)/lib/$(LIB) $(HDR:%=$(PREFIX)/include/%)

.PHONY: all clean check uninstall
clean:
	rm -f $(OBJ) $(LIB) test_h3client.o test_h3client *.d
	rm -f data/ross.5.hmm.h3f
	rm -f data/ross.5.hmm.h3i
	rm -f data/ross.5.hmm.h3m
	rm -f data/ross.5.hmm.h3p
	rm -f data/ross.5.hmm.pid
