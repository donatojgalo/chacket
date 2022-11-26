CPP = g++
CC  = gcc
INC = 
OPC = -g -c
LIB = -lpthread

.c.o:
	$(CC) $(OPC) $(INC) $<

.cpp.o:
	$(CPP) $(OPC) $(INC) $<

all:	chatServer chatClient

chatServer: chatServer.o
	$(CC) -o $@ $< $(LIB)

chatClient: chatClient.o
	$(CC) -o $@ $< $(LIB)

clean:
	rm -fr *~ *.o chatServer chatClient

