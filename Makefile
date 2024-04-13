CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS =

SRCS = main.c libmf.c
OBJS = $(SRCS:.c=.o)
TEST_SRCS = test.c libmf.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

EXEC = main
TEST_EXEC = test

all: $(EXEC) $(TEST_EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC) $(LDFLAGS)

$(TEST_EXEC): $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $(TEST_EXEC) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC) $(TEST_OBJS) $(TEST_EXEC)
