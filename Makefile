PROJ_NAME = ted
CC = gcc

CFLAGS = -std=c99 -fstack-protector-all -Wall -Wextra -g -Isrc -Itst/unity

SRCS = src/hash_extensivel.c tst/t_hash_extensivel.c tst/unity/unity.c
OBJS = $(SRCS:.c=.o)

$(PROJ_NAME): $(OBJS)
	$(CC) $(CFLAGS) -o src/$(PROJ_NAME) $(OBJS)

%.o: %.c src/hash_extensivel.h
	$(CC) $(CFLAGS) -c $< -o $@

tstall: $(PROJ_NAME)
	./src/$(PROJ_NAME)

all: $(PROJ_NAME)

clean:
	rm -f src/$(PROJ_NAME) $(OBJS) *.bin

.PHONY: all tstall clean
