cc = gcc
cflags = -Wall
execs = daemonSecureBindTest requester myUDS
all : secure_bindTest

secure_bindTest : daemonSecureBindTest requester

daemonSecureBindTest : daemonSecureBindTest.o
	$(cc) $(cflags) -o $@ $<

requester : requester.o secure_bind.o
	$(cc) $(cflags) -o $@ $< secure_bind.o

%.o : %.c
	$(cc) $(cflags) -c -g $<

clean :
	$(RM) *.o , *.gch , *~ , *#
	$(RM) $(execs)
