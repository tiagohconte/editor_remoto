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
void comando_ls(int *seq, int soquete)
{  
  kermitHuman package;

  package.inicio = 126;
  package.dest = 2;
  package.orig = 1;
  package.tam = 0;
  package.seq = *seq;
  package.tipo = 1;
  package.par = 0;
  package.data = NULL;
  
  if( sendPackage(&package, soquete) < 0 )
    exit(-1);

  (*seq)++;

  int terminou = 0;

  while( !terminou )
  {

    sleep(1);
    resetPackage(&package);

    // espera receber os dados do comando ls
    if( waitPackage(&package, soquete) == -1 ){
      //sendNACK(seq, soquete, 2, 1);
    } else {
      // verifica se o destino está correto e se o tipo é ls
      if( (package.dest == 1) && (package.seq == *seq) && ((package.tipo == 11) || (package.tipo == 13)) ){
        // fim do ls
        if( package.tipo == 13 ){  
          terminou = 1;
        } else {
          printf("%s", package.data);
        }

        sendACK(seq, soquete, 2, 1);
      } else {
        //sendNACK(seq, soquete, 2, 1);        
      }
    }

  }


}