cc = gcc

all : sender requester

sender : senderVCSocketPermissionsTest.o
	$(cc) -o $@ $<

requester : requesterVCSocketPermissionsTest.o
	$(cc) -o $@ $<

%.o : %.c
	$(cc) -c -g $<

clean :
	rm -f sender
	rm -f requester
	rm -f uds
