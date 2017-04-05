cc = gcc
cflags = -Wall

all : secure_bind

secure_bind : secure_bind.o

secure_bindTest : daemonSecureBindTest requester

daemonSecureBindTest : daemonSecureBindTest.o
	$(cc) $(cflags) -o $@ $<

requester : requester.o secure_bind.o 
	$(cc) $(cflags) -o $@ $< secure_bind.o

%.o : %.c
	$(cc) $(cflags) -c -g $<

clean :
	$(RM) daemonSecureBindTest, requester, *.o , *.gch , *~ , *#
