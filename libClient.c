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
void comando_cd(int *seq, int soquete)
{
  kermitHuman packageSend, packageRec;

  char dir[15];
  scanf("%s", dir);

  packageSend.inicio = 126;
  packageSend.dest = 2;
  packageSend.orig = 1;
  packageSend.tam = strlen(dir);
  packageSend.seq = *seq;
  packageSend.tipo = 0;
  packageSend.par = 0;
  packageSend.data = malloc(packageSend.tam);
  strncpy(packageSend.data, dir, packageSend.tam);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  incrementaSeq(seq);

  // quando tipo = 8, sucesso no comando cd
  // quando tipo = 15, houve erro
  resetPackage(&packageRec);
  while( (packageRec.tipo != 8) && (packageRec.tipo != 15))
  {
    resetPackage(&packageRec);

    // espera receber pacote
    if( waitPackage(&packageRec, soquete) == -1 )
      exit(-1);

    // se pacote for NACK, envia o pacote novamente
    if( packageRec.tipo == 9 ){
      if( sendPackage(&packageSend, soquete) < 0 )
        exit(-1);
    }
  }

  if( packageRec.tipo == 15 )
  {
    printError(&packageRec);
  }

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
  package.seq = *seq;
  package.tipo = 1;
  package.par = 0;
  package.data = NULL;
  
  if( sendPackage(&package, soquete) < 0 )
    exit(-1);

  incrementaSeq(seq);

  int seqEsperada = -1;
  // quando tipo = 13, acabou a transmissão do ls
  while( package.tipo != 13 )
  {

    resetPackage(&package);

    // espera receber os dados do comando ls
    if( waitPackage(&package, soquete) == -1 ){
      exit(-1);
    } else 
    {
      // se for o primeiro pacote recebido, sequencia é setada
      if( (seqEsperada == -1) && (package.dest == 1) && ( (package.tipo == 11) || (package.tipo == 13) ) ){
        seqEsperada = package.seq;
      }

      // verifica se o destino está correto, a sequência e se o tipo é ls
      if( (package.dest == 1) && (package.seq == seqEsperada) )
      {

        if( (package.tipo == 11) || (package.tipo == 13) )
        {
          if( package.tipo == 11 ){
            printf("%s", package.data);
          }

          sendACK(package.orig, package.dest, seq, soquete);

          incrementaSeq(&seqEsperada);
        }
        else
          sendNACK(package.orig, package.dest, seq, soquete);

      } else 
      {    
        sendNACK(package.orig, package.dest, seq, soquete);
      }
    }

  }

}