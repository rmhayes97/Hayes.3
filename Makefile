CC = gcc
CFLAG = -g

all: parent child

%.o: %.c
	$(CC) $(CFLAG) -c  $< -o $@ -lm

parent: parent.o
	$(CC) $(CFLAG) $< -o $@ -lm

child: child.o
	$(CC) $(CFLAG) $< -o $@ -lm

clean:
	rm -f *.o parent child
