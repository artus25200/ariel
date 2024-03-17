OBJS	= *.o
OBJS    := $(filter-out output.o, $(OBJS))
SOURCE	= *.c
HEADER	= *.h
OUT	= cariel
CC	 = gcc
FLAGS	 = -g -c -Wall -ggdb
LFLAGS	 =

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)
	$(MAKE) clean


*.o: *.c
	$(CC) $(FLAGS) *.c


clean:
	rm -f $(OBJS)

