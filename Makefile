cc = gcc
cflags = -Wall
execs = myUDS sprd
all : daemon

daemon : sprd.o parse_config.o linked_list.o
	gcc sprd.o parse_config.o tokenizer.o linked_list.o -o sprd

parse_config.o : parse_config.c
	gcc parse_config.c tokenizer.c -c #-o parse_config

%.o : %.c
	$(cc) $(cflags) -c -g $<

clean :
	$(RM) *.o , *.gch , *~ , *#
	$(RM) $(execs)
