cc = gcc

all : sender requester

sender : senderVCSocketPermissionsTest.o
	$(cc) -o $@ $<

requester : requesterVCSocketPermissionsTest.o
	$(cc) -o $@ $<

%.o : %.c
	$(cc) -c -g $<

clean :
	$(RM) sender , requester , uds , *.o , *.gch , *~ , *#
