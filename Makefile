cc = gcc
cflags = -Wall
execs = myUDS sprd finalRequester
all : daemon

daemon : sprd.o parse_config.o linked_list.o
	$(cc) $(cflags) sprd.o parse_config.o tokenizer.o linked_list.o -o sprd

parse_config.o : parse_config.c
	$(cc) $(cflags) parse_config.c tokenizer.c -c #-o parse_config

socktest : finalRequester.o daemon secure_bind.o secure_close.o
	$(cc) $(cflags) $< secure_bind.o secure_close.o -o finalRequester

install : daemon
	mv sprd.conf /tmp
	./sprd
%.o : %.c
	$(cc) $(cflags) -c -g $<

clean :
	$(RM) *.o , *.gch , *~ , *#
	$(RM) $(execs)
	$(RM) /tmp/proc*

