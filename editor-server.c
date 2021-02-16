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

  kermitHuman package;
  int seq = 0;

  while(1) 
  {
    resetPackage(&package);

    if( receiveBuffer(buffer, soquete) < 0 )
      exit(-1);

    struct kermitBit *packageBit = (struct kermitBit *) buffer;

    // printf("buffer: %s\n", buffer);

    if( verificaBits(packageBit->header[0], 126) )
    {
      readPackageBit(&package, packageBit);  

      printf("dest: %d, orig: %d, tam: %d\n", package.dest, package.orig, package.tam);
      printf("seq: %d, tipo: %d\n", package.seq, package.tipo);
      printf("data: %s, par: %d\n", package.data, package.par);
    }

    // Verifica se o destino é mesmo o servidor
    if( package.dest == 2 )
    {
      // Verifica o tipo do package
      // comando cd
      if(package.tipo == 0)
      {
        printf("cd\n");

      } else if (package.tipo == 1) // comando ls
      {
        comando_ls(buffer, &seq, soquete);

      } else if (package.tipo == 2) // comando ver
      {
        printf("ver\n");

      } else if (package.tipo == 3) // comando linha
      {
        printf("linha\n");

      } else if (package.tipo == 4)  // comando linhas
      {
        printf("linhas\n");

      } else if (package.tipo == 5)  // comando edit
      {
        printf("edit\n");
      }
    }

    // para não ler a mesma mensagem 2 vezes
    receiveBuffer(buffer, soquete);    
  }  

  return 0;
}