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
#include <string.h>
#include <errno.h>
#include "rawSocket.h"
#include "kermitProtocol.h"
#include "libServer.h"

int main()
{
  char *device = "lo";
  int soquete;

  soquete = conexaoRawSocket(device);

  kermitHuman package;
  int seq = 0;
  iniciaPackage(&package);

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
        comando_ver(&package, &seq, soquete);

      } else if (package.tipo == 3) // comando linha
      {
        comando_linha(&package, &seq, soquete);

      } else if (package.tipo == 4)  // comando linhas
      {
        comando_linhas(&package, &seq, soquete);

      } else if (package.tipo == 5)  // comando edit
      {
        printf("edit\n");
      }
    }

  }  

  return 0;
}