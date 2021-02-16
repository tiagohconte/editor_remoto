/*  
  Feito por:
    Tiago Henrique Conte

  Redes de Computadores 1 - BCC UFPR
  Trabalho Prático - Editor de Texto Remoto

  Biblioteca Editor Remoto - Client
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "kermitProtocol.h"
#include "libClient.h"

//  Comando ls - client side
void comando_ls(char *buffer, int *seq, int soquete)
{  
  kermitHuman package;
  struct kermitBit packageBit;

  package.dest = 2;
  package.orig = 1;
  package.tam = 0;
  package.seq = *seq;
  package.tipo = 1;
  package.par = 0;
  package.data = NULL;

  writePackageBit(&packageBit, &package);
  
  if( sendBuffer(buffer, &packageBit, package.tam, soquete) < 0 )
    exit(-1);

  int terminou = 0;

  while( !terminou )
  {
    // para não ler a mensagem enviada por ele mesmo
    sleep(1);
    // memset(buffer, 0, TAM_PACKAGE);
    resetPackage(&package);

    // espera receber os dados do comando ls
    if( waitPackage(buffer, soquete) == -1 ){
      sendNACK(buffer, seq, soquete, 2, 1);
    } else {
      printf("buffer: %s\n", buffer);
      struct kermitBit *packageBit = (struct kermitBit *) buffer;
      readPackageBit(&package, packageBit);
      if( package.tipo == 13 )
        terminou++;
      sendACK(buffer, seq, soquete, 2, 1);
    }

  }


}