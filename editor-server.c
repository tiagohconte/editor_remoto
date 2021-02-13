/*  
  Feito por:
    Tiago Henrique Conte

  Redes de Computadores 1 - BCC UFPR
  Trabalho Prático - Editor de Texto Remoto

  Editor Remoto - Server Side
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "rawSocket.h"
#include "kermitProtocol.h"
#include "libServer.h"

int main()
{
  char *device = "lo";
  int soquete;

  soquete = conexaoRawSocket(device);

  char *buffer = (char *) malloc(TAM_PACKAGE);
  memset(buffer, 0, TAM_PACKAGE);
  int tamBuffer = 0;
  char temp;

  kermitHuman package;

  while(1) 
  {
    resetPackage(&package);

    tamBuffer = read(soquete, buffer, TAM_PACKAGE);
    if( tamBuffer == -1 )
    {
      fprintf(stderr, "Erro no recebimento: %s\n", strerror(errno));
      exit(-1);
    }

    struct kermitBit *packageBit = (struct kermitBit *) buffer;

    printf("buffer: %s\n", buffer);

    if( verificaBits(packageBit->header[0], 126) )
    {
      // coleta os dois bits mais significativos do 2° byte, onde está o end. destino
      temp = packageBit->header[1] & 0xc0;
      package.dest = (unsigned char) temp >> 6; 
      // coleta os 3° e 4° bits mais significativos do 2° byte, onde está o end. origem
      temp = packageBit->header[1] & 0x30;
      package.orig = (unsigned char) temp >> 4;
      // coleta os 4 bits menos significativos do 2° byte, onde está o tamanho
      package.tam = (unsigned char) (packageBit->header[1] & 0x0F);

      // coleta os 4 bits mais significativos do 3° byte, onde está a sequencia
      temp = packageBit->header[2] & 0xF0;
      package.seq = (unsigned char) temp >> 4;
      // coleta os 4 bits menos significativos do 3° byte, onde está o tipo
      package.tipo = (unsigned char) packageBit->header[2] & 0x0F;

      package.data = (char *) malloc(package.tam);
      strcpy(package.data, packageBit->data);

      package.par = (unsigned char) packageBit->data[package.tam];

      printf("dest: %d, orig: %d, tam: %d\n", package.dest, package.orig, package.tam);
      printf("seq: %d, tipo: %d\n", package.seq, package.tipo);
      printf("data: %s, par: %d\n", package.data, package.par);
    }   

    // para não ler a mesma mensagem 2 vezes
    read(soquete, buffer, sizeof(buffer));    
  }  

  return 0;
}