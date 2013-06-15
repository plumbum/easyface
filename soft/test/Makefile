CC=gcc
LD=gcc
INCLUDE=-I .
CCFLAGS=
LDFLAGS=
LDLIB=-lm -lgcc

all: main

main: main.o modbus.o
	$(LD) $(LDFLAGS) $^ -o $@ $(LDLIB)

%.o: %.c
	$(CC) $(CCFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f *.o
	rm -f main

