CC := gcc
CFLAGS := -Wall -Wextra -pthread

TARGETS := app mfserver
LIBRARY := libmf.a

all: $(TARGETS)

MF_SRC := mf.c
MF_OBJS := $(MF_SRC:.c=.o)

libmf.a: $(MF_OBJS)
	ar rcs $@ $(MF_OBJS)

MF_LIB := -L. -lmf

mf.o: mf.c mf.h
	$(CC) -c $(CFLAGS) -o $@ $<

app.o: app.c mf.h
	$(CC) -c $(CFLAGS) -o $@ $<

mfserver.o: mfserver.c mf.h
	$(CC) -c $(CFLAGS) -o $@ $<

app: app.o $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $^

mfserver: mfserver.o $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGETS) $(LIBRARY)
