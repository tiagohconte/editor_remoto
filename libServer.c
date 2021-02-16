/*  
  Feito por:
    Tiago Henrique Conte

  Redes de Computadores 1 - BCC UFPR
  Trabalho Prático - Editor de Texto Remoto

  Biblioteca Editor Remoto - Server
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "kermitProtocol.h"
#include "libServer.h"

//  Comando ls - server side
void comando_ls(char *buffer, int *seq, int soquete)
{  
  kermitHuman package;
  struct kermitBit packageBit;  
  FILE *retorno = NULL;
  char str[15];

  // executa ls no servidor
  retorno = popen("ls", "r");
  if (retorno == NULL){
    fprintf(stderr, "Erro na execução do comando!\n");
  }

  while(!feof(retorno)){
    fgets(str, 15, retorno);
    printf("%s", str);   

    package.dest = 1;
    package.orig = 2;
    package.tam = strlen(str);
    package.seq = *seq;
    package.tipo = 11;
    package.par = 0;
    package.data = malloc(package.tam);
    strncpy(package.data, str, package.tam);

    //transforma para bits
    writePackageBit(&packageBit, &package);

    int espera = 0;
    while( espera != 1 )
    {
      // prepara e envia o buffer
      if( sendBuffer(buffer, &packageBit, package.tam, soquete) == -1 )
        exit(-1);

      espera = waitACK(buffer, soquete);
    }

    sleep(1);

  }

}