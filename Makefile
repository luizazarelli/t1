CC = gcc

CFLAGS = -Wall -Wextra -g

TARGET = testes

SRCS = hash_extensivel.c testes.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c hash_extensivel.h
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) *.bin

.PHONY: all run clean
