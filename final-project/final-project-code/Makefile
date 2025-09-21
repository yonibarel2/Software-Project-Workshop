CC = gcc
CFLAGS = -ansi -Wall -Wextra -Werror -pedantic-errors
LDLIBS = -lm

all: symnmf

symnmf: symnmf.c symnmf.h
	$(CC) $(CFLAGS) -o symnmf symnmf.c $(LDLIBS)

clean:
	rm -f symnmf