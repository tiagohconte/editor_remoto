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
void comando_ls(int seq, int soquete)
{  
  kermitHuman package;

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

    package.inicio = 126;
    package.dest = 1;
    package.orig = 2;
    package.tam = strlen(str);
    package.seq = ++seq;
    package.tipo = 11;
    package.par = 0;
    package.data = malloc(package.tam);
    strncpy(package.data, str, package.tam);
    seq++;

    int espera = 0;
    while( !espera )
    {
      // prepara e envia o buffer
      if( sendPackage(&package, soquete) == -1 )
        exit(-1);

      sleep(1);

      /*// espera receber os dados do comando ls
      if( waitPackage(&package, soquete) == -1 ){
        //sendNACK(seq, soquete, 2, 1);
      } else {
        // verifica se o destino está correto e se o tipo é ls
        if( (package.dest == 1) && (package.seq == seq) && ((package.tipo == 11) || (package.tipo == 13)) ){
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
      }*/
      
    }

  }

  // prepara o pacote de final de transmissão
  package.inicio = 126;
  package.dest = 1;
  package.orig = 2;
  package.tam = 0;
  package.seq = seq++;
  package.tipo = 13;
  package.par = 0;
  free(package.data);
  package.data = NULL;
  // envia pacote de final de transmissão
  if( sendPackage(&package, soquete) == -1 )
    exit(-1);

}