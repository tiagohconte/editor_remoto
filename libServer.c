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
#include <errno.h>
#include "kermitProtocol.h"
#include "libServer.h"

// Comando cd - server side
// Executa change directory no server
void comando_cd(kermitHuman *package, int *seq, int soquete)
{
  
  if( chdir(package->data) ){
    sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
    return;
  }

  sendACK(package->orig, package->dest, seq, soquete);

  free(package->data);
  package->data = NULL;

}

// Comando ls - server side
// Executa ls no server
void comando_ls(int *seq, int soquete)
{  
  kermitHuman package;

  FILE *retorno;
  char str[16];
  int espera;

  // executa ls no servidor
  retorno = popen("ls", "r");
  if (retorno == NULL){
    fprintf(stderr, "Erro na execução do comando!\n");
  }

  fgets(str, 16, retorno);
  while(!feof(retorno)){ 

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

    fgets(str, 16, retorno);

    incrementaSeq(seq);

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

  incrementaSeq(seq);

}

// Comando ver - server side
// Mostra o conteúdo do arquivo texto do servidor na tela do cliente
void comando_ver(kermitHuman *package, int *seq, int soquete)
{  

  kermitHuman packageSend, packageRec;

  FILE *arquivo;

  // abre <ARQUIVO> no servidor
  arquivo = fopen(package->data, "r");
  if (arquivo == NULL){
    sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
    return;
  }

  char str[16];
  int espera;

  while(!feof(arquivo)){ 
    fgets(str, 16, arquivo);

    packageSend.inicio = 126;
    packageSend.dest = 1;
    packageSend.orig = 2;
    packageSend.tam = strlen(str);
    packageSend.seq = *seq;
    packageSend.tipo = 12;
    packageSend.par = 0;
    packageSend.data = malloc(packageSend.tam);
    strncpy(packageSend.data, str, packageSend.tam);

    espera = 0;
    while( !espera )
    {
      resetPackage(&packageRec);
      // prepara e envia o buffer
      if( sendPackage(&packageSend, soquete) == -1 )
        exit(-1);

      // espera receber o ACK ou NACK
      if( waitPackage(&packageRec, soquete) == -1 ){
        exit(-1);
      } else if( (packageRec.dest == 2) && (packageRec.tipo == 8) ){
        espera = 1;        
      }
      
    }

    incrementaSeq(seq);

  }

  // prepara o pacote de final de transmissão
  packageSend.inicio = 126;
  packageSend.dest = 1;
  packageSend.orig = 2;
  packageSend.tam = 0;
  packageSend.seq = *seq;
  packageSend.tipo = 13;
  packageSend.par = 0;
  free(packageSend.data);
  packageSend.data = NULL;

  // envia pacote de final de transmissão e aguarda ACK
  espera = 0;
  while( !espera )
  {
    resetPackage(&packageRec);
    // prepara e envia o buffer
    if( sendPackage(&packageSend, soquete) == -1 )
      exit(-1);

    // espera receber o ACK ou NACK
    if( waitPackage(&packageRec, soquete) == -1 ){
      exit(-1);
    } else if( (packageRec.dest == 2) && (packageRec.tipo == 8) ){
      espera = 1;        
    }
    
  }

  incrementaSeq(seq);

}

// Comando linha - server side
// Mostra a linha <numero_linha> do arquivo <nome_arq> que esta no servidor na tela do cliente.
void comando_linha(kermitHuman *package, int *seq, int soquete)
{
  kermitHuman packageSend, packageRec;

  FILE *arquivo;
  char str[16];
  int espera, cont = 1, linha;

  // abre <ARQUIVO> no servidor
  arquivo = fopen(package->data, "r");
  if (arquivo == NULL){
    sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
    return;
  }

  sendACK(package->orig, package->dest, seq, soquete);

  // espera receber o número da linha
  espera = 0;
  while( !espera )
  {
    resetPackage(&packageRec);

    if( waitPackage(&packageRec, soquete) == -1 ){
      exit(-1);
    } else if( (packageRec.dest == 2) && (packageRec.tipo == 10) ){
      espera = 1;        
    }
    
  }

  linha = strtol(packageRec.data, NULL, 10);

  // se a linha for menor que 1, ela não existe
  if( linha < 1 )
  {
    sendError(package->orig, package->dest, seq, package->tipo, -1, soquete);
    return;
  }


  // encontra a linha e a envia
  while( (!feof(arquivo)) && (cont <= linha) ){
    fgets(str, 16, arquivo);


    if( cont == linha )
    {
      // envia string   
      packageSend.inicio = 126;
      packageSend.dest = 1;
      packageSend.orig = 2;
      packageSend.tam = strlen(str);
      packageSend.seq = *seq;
      packageSend.tipo = 12;
      packageSend.par = 0;
      packageSend.data = malloc(packageSend.tam);
      strncpy(packageSend.data, str, packageSend.tam);

      espera = 0;
      while( !espera )
      {
        resetPackage(&packageRec);
        // prepara e envia o buffer
        if( sendPackage(&packageSend, soquete) == -1 )
          exit(-1);

        // espera receber o ACK ou NACK
        if( waitPackage(&packageRec, soquete) == -1 ){
          exit(-1);
        } else if( (packageRec.dest == 2) && (packageRec.tipo == 8) ){
          espera = 1;        
        }
        
      }

      incrementaSeq(seq);

    }
    
    if( str[strlen(str)-1] == '\n' )
      cont++;

  }

  // envia erro caso linha não exista
  if( cont < linha )
  {
    sendError(package->orig, package->dest, seq, package->tipo, -1, soquete);
    return;
  }

  // prepara o pacote de final de transmissão
  packageSend.inicio = 126;
  packageSend.dest = 1;
  packageSend.orig = 2;
  packageSend.tam = 0;
  packageSend.seq = *seq;
  packageSend.tipo = 13;
  packageSend.par = 0;
  free(packageSend.data);
  packageSend.data = NULL;

  // envia pacote de final de transmissão e aguarda ACK
  espera = 0;
  while( !espera )
  {
    resetPackage(&packageRec);
    // prepara e envia o buffer
    if( sendPackage(&packageSend, soquete) == -1 )
      exit(-1);

    // espera receber o ACK ou NACK
    if( waitPackage(&packageRec, soquete) == -1 ){
      exit(-1);
    } else if( (packageRec.dest == 2) && (packageRec.tipo == 8) ){
      espera = 1;        
    }
    
  }

  incrementaSeq(seq);
}