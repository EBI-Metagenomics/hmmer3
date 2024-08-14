CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -O3 -MMD -MP
SRC = $(filter-out $(wildcard test_*.c),$(wildcard *.c))
OBJ = $(SRC:.c=.o)
HDR = $(wildcard h3c_*.h)
LIB = libh3client.a
PREFIX ?= /usr/local
TEST_SRC = $(wildcard test_*.c)
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_TARGET = $(basename $(TEST_OBJ))

all: $(LIB)

$(LIB): $(OBJ)
	ar rcs $@ $^

-include $(SRC:.c=.d)

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_TARGET): %: %.o $(LIB) $(HDR)
	$(CC) $(CFLAGS) $< -L. -lh3client -lh3result -lnng -llio -llite_pack -lm -o $@

check: $(TEST_TARGET)
	./start_daemon.sh data/ross.5.hmm
	for test in $(TEST_TARGET); do ./$$test || exit 1; done
	./stop_daemon.sh data/ross.5.hmm

install: $(LIB) $(HDR)
	@mkdir -p $(PREFIX)/lib $(PREFIX)/include
	install -m 0755 $(LIB) $(PREFIX)/lib/
	install -m 0644 $(HDR) $(PREFIX)/include/

uninstall:
	rm -f $(PREFIX)/lib/$(LIB) $(HDR:%=$(PREFIX)/include/%)

.PHONY: all clean check uninstall
clean:
	rm -f $(OBJ) $(LIB) $(TEST_OBJ) $(TEST_TARGET) *.d
	rm -f data/ross.5.hmm.h3f
	rm -f data/ross.5.hmm.h3i
	rm -f data/ross.5.hmm.h3m
	rm -f data/ross.5.hmm.h3p
	rm -f data/ross.5.hmm.pid
