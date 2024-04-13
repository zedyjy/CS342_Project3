CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS =

SRCS = mfserver.c mf.c
OBJS = $(SRCS:.c=.o)
EXEC = mfserver

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)
