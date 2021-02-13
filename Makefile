#	Feito por:
#		Tiago Henrique Conte

# Redes de Computadores 1 - BCC UFPR
#	Trabalho Prático - Editor de Texto Remoto

#	Makefile Editor Remoto

CFLAGS = -Wall
CC = gcc

BINARIES = editor-client editor-server
 
objs = rawSocket.o kermitProtocol.o
 
# regra default
all: $(BINARIES)
 
# regras de ligacao
editor-client: $(objs) libClient.o editor-client.o
editor-server: $(objs) libServer.o editor-server.o

# regras de compilação
rawSocket.o: rawSocket.c rawSocket.h
kermitProtocol.o: kermitProtocol.c kermitProtocol.h
libClient.o: libClient.c libClient.h kermitProtocol.o
libServer.o: libServer.c libServer.h kermitProtocol.o

# compila com flags de depuração
debug: CFLAGS += -DDEBUG -g
debug: all

# remove arquivos temporários
clean:
	-rm -f *.o

# remove tudo o que não for o código-fonte
purge: clean
	-rm -f $(BINARIES)