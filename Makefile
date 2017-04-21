cc = gcc
cflags = -Wall
execs = daemonSecureBindTest requester myUDS parse_config
all : parse_config

secure_bindTest : daemonSecureBindTest requester

parse_config : parse_config.c
	gcc parse_config.h parse_config.c tokenizer.c tokenizer.h -o parse_config

daemonSecureBindTest : daemonSecureBindTest.o
	$(cc) $(cflags) -o $@ $<

requester : requester.o secure_bind.o
	$(cc) $(cflags) -o $@ $< secure_bind.o

%.o : %.c
	$(cc) $(cflags) -c -g $<

clean :
	$(RM) *.o , *.gch , *~ , *#
	$(RM) $(execs)
