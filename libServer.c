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
void comando_ls(int *seq, int soquete)
{  
  kermitHuman package;

  FILE *retorno;
  char str[15];
  int espera;

  // executa ls no servidor
  retorno = popen("ls", "r");
  if (retorno == NULL){
    fprintf(stderr, "Erro na execução do comando!\n");
  }

  fgets(str, 15, retorno);
  while(!feof(retorno)){
    // printf("%s", str);   

    package.inicio = 126;
    package.dest = 1;
    package.orig = 2;
    package.tam = strlen(str);
    package.seq = *seq;
    package.tipo = 11;
    package.par = 0;
    package.data = malloc(package.tam);
    strncpy(package.data, str, package.tam);

    espera = 0;
    while( !espera )
    {
      // prepara e envia o buffer
      if( sendPackage(&package, soquete) == -1 )
        exit(-1);

      // espera receber o ACK ou NACK
      if( waitPackage(&package, soquete) == -1 ){
        exit(-1);
      } else if( (package.dest == 2) && (package.tipo == 8) ){
        espera = 1;        
      }
      
    }

    fgets(str, 15, retorno);

    if( *seq > 14 )
      *seq = 0;
    else
      (*seq)++;

  }

  // prepara o pacote de final de transmissão
  package.inicio = 126;
  package.dest = 1;
  package.orig = 2;
  package.tam = 0;
  package.seq = *seq;
  package.tipo = 13;
  package.par = 0;
  free(package.data);
  package.data = NULL;

  // envia pacote de final de transmissão e aguarda ACK
  espera = 0;
  while( !espera )
  {
    // prepara e envia o buffer
    if( sendPackage(&package, soquete) == -1 )
      exit(-1);

    // espera receber o ACK ou NACK
    if( waitPackage(&package, soquete) == -1 ){
      exit(-1);
    } else if( (package.dest == 2) && (package.tipo == 8) ){
      espera = 1;        
    }
    
  }

}