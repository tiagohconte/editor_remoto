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
#include <errno.h>
#include "kermitProtocol.h"
#include "libClient.h"

/* COMANDOS LOCAIS */

// Comando change directory local
void comando_lcd()
{
  char option[100];
  scanf("%s", option);
  if( chdir(option) )
    fprintf(stderr, "Erro na execução do comando: %s\n", strerror(errno));
}

// Comando ls local
void comando_lls()
{
  FILE *retorno;
  char str[15];

  // executa ls no servidor
  retorno = popen("ls", "r");
  if (retorno == NULL){
    fprintf(stderr, "Erro na execução do comando!\n");
  }

  fgets(str, 15, retorno);
  while(!feof(retorno)){
    printf("%s", str);
    fgets(str, 15, retorno);       
  }
}

/* COMANDOS EXTERNOS */

// Comando cd - client side
// Executa change directory no server
void comando_cd()
{

}

// Comando ls - client side
// Executa ls no server
void comando_ls(int *seq, int soquete)
{  
  kermitHuman package;

  package.inicio = 126;
  package.dest = 2;
  package.orig = 1;
  package.tam = 0;
  package.seq = 0;
  package.tipo = 1;
  package.par = 0;
  package.data = NULL;
  
  if( sendPackage(&package, soquete) < 0 )
    exit(-1);

  // quando tipo = 13, acabou a transmissão do ls
  while( package.tipo != 13 )
  {

    resetPackage(&package);

    // espera receber os dados do comando ls
    if( waitPackage(&package, soquete) == -1 ){
      exit(-1);
    } else 
    {
      // verifica se o destino está correto e se o tipo é ls
      if( (package.dest == 1) )
      {

        if( package.tipo == 11 )
        {
          printf("%s", package.data);
          sendACK(2, 1, soquete);
        } 
        else if( package.tipo == 13 )
          sendACK(2, 1, soquete);
        else
          sendNACK(2, 1, soquete);

      } else 
      {    
        sendNACK(2, 1, soquete);
      }
    }

  }

}