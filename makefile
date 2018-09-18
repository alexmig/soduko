CC = gcc
CFLAGS = -Wall -Werror -std=gnu11 -Wno-unused-function

.DEFAULT_GOAL := all

sdk_gen: am_common.c sdk_common.c sdk_gen.c
	 $(CC) $(CFLAGS) -o $@ $^

sdk_strip: am_common.c sdk_common.c sdk_strip.c
	 $(CC) $(CFLAGS) -o $@ $^

sdk_solve: am_common.c sdk_common.c sdk_solve.c
	 $(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f sdk_gen sdk_strip sdk_solve *.o *~ ~*

all: clean sdk_gen sdk_strip

