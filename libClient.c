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

  incrementaSeq(seq);

  if( packageRec.tipo == 15 )
  {
    printError(&packageRec);
  }

}

// Comando ls - client side
// Executa ls no server
void comando_ls(int *seq, int soquete)
{  
  kermitHuman packageSend, packageRec;

  packageSend.inicio = 126;
  packageSend.dest = 2;
  packageSend.orig = 1;
  packageSend.tam = 0;
  packageSend.seq = *seq;
  packageSend.tipo = 1;
  packageSend.par = 0;
  packageSend.data = NULL;
  
  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  incrementaSeq(seq);

  int seqEsperada = -1;
  resetPackage(&packageRec);
  // quando tipo = 13, acabou a transmissão do ls
  while( packageRec.tipo != 13 )
  {

    resetPackage(&packageRec);

    // espera receber os dados do comando ls
    if( waitPackage(&packageRec, soquete) == -1 ){
      exit(-1);
    } else 
    {
      // verifica se o destino está correto
      if( packageRec.dest == packageSend.orig )
      {
        // se for o primeiro pacote recebido, sequencia é setada
        if( (seqEsperada == -1) && ( (packageRec.tipo == 11) || (packageRec.tipo == 13) ) ){
          seqEsperada = packageRec.seq;
        }

        // verifica a sequência e se o tipo é ls
        if( (packageRec.seq == seqEsperada) )
        {

          if( (packageRec.tipo == 11) || (packageRec.tipo == 13) )
          {
            if( packageRec.tipo == 11 ){
              printf("%s", packageRec.data);
            }

            sendACK(packageRec.orig, packageRec.dest, seq, soquete);

            incrementaSeq(&seqEsperada);
          }
          else
            sendNACK(packageRec.orig, packageRec.dest, seq, soquete);

        } else 
        {    
          sendNACK(packageRec.orig, packageRec.dest, seq, soquete);
        }
      }
    }

  }

}

// Comando ver - client side
// Mostra o conteúdo do arquivo texto do servidor na tela do cliente
void comando_ver(int *seq, int soquete)
{  
  kermitHuman packageSend, packageRec;

  char arq[15];
  scanf("%s", arq);

  packageSend.inicio = 126;
  packageSend.dest = 2;
  packageSend.orig = 1;
  packageSend.tam = strlen(arq);
  packageSend.seq = *seq;
  packageSend.tipo = 2;
  packageSend.par = 0;
  packageSend.data = malloc(packageSend.tam);
  strncpy(packageSend.data, arq, packageSend.tam);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  incrementaSeq(seq);

  int seqEsperada = -1;
  int linha = 1;

  resetPackage(&packageRec);
  // espera receber os dados do comando ver
  if( waitPackage(&packageRec, soquete) == -1 ){
    exit(-1);
  }

  // quando tipo = 13, acabou a transmissão
  // quando tipo = 15, houve erro
  while( packageRec.tipo != 13 && packageRec.tipo != 15 )
  {
    
    // verifica se o destino está correto
    if( packageRec.dest == packageSend.orig )
    {
      // se for o primeiro pacote recebido, sequencia é setada
      if( (seqEsperada == -1) && ( (packageRec.tipo == 12) || (packageRec.tipo == 13) ) ){
        seqEsperada = packageRec.seq;
        
        printf("%d ", linha++);
      }

      // verifica a sequência 
      if( packageRec.seq == seqEsperada )
      {
        // verifica se o tipo é conteúdo arquivo
        if( (packageRec.tipo == 12) )
        {         
          printf("%s", packageRec.data);
          if(packageRec.data[strlen(packageRec.data)-1] == '\n')
            printf("%d ", linha++);          

          sendACK(packageRec.orig, packageRec.dest, seq, soquete);

          incrementaSeq(&seqEsperada);
        }
        else
          sendNACK(packageRec.orig, packageRec.dest, seq, soquete);

      } else 
      {    
        sendNACK(packageRec.orig, packageRec.dest, seq, soquete);
      }
    }

    resetPackage(&packageRec);

    // espera receber os dados do comando ver
    if( waitPackage(&packageRec, soquete) == -1 ){
      exit(-1);
    }
  }

  // verifica se o tipo é erro
  if( (packageRec.tipo == 15) )
    printError(&packageRec);
  else
    sendACK(packageRec.orig, packageRec.dest, seq, soquete);

  printf("\n");

}