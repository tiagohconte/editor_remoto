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

    if( receivePackage(&package, soquete) < 0 )
      exit(-1);

    // Verifica se o destino é mesmo o servidor
    if( package.dest == 2 )
    {
      // Verifica o tipo do package
      // comando cd
      if(package.tipo == 0)
      {
        comando_cd(&package, &seq, soquete);

      } else if (package.tipo == 1) // comando ls
      {
        comando_ls(&seq, soquete);

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

  }  

  return 0;
}