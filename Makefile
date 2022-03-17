CC = gcc
CFLAG = -g -I -std=c99 -std=gnu99 -D_SVID_SOURCE -D_POSIX_C_SOURCE

all: parent child

%.o: %.c
	$(CC) $(CFLAG) -c  $< -o $@ -lm

parent: parent.o
	$(CC) $(CFLAG) $< -o $@ -lm

child: child.o
	$(CC) $(CFLAG) $< -o $@ -lm

clean:
	rm -f *.o parent child cstest logfile.*

