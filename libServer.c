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
  
  if( chdir( (char*)package->data ) ){
    sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
    return;
  }

  sendACK(package->orig, package->dest, seq, soquete);

}

// Comando ls - server side
// Executa ls no server
void comando_ls(int *seq, int soquete)
{  
  kermitHuman packageSend, packageRec;

  FILE *retorno_ls;
  char str[TAM_DATA+1];
  int espera, retorno;

  // executa ls no servidor
  retorno_ls = popen("ls", "r");
  if (retorno_ls == NULL){
    fprintf(stderr, "Erro na execução do comando!\n");
  }

  fgets(str, TAM_DATA+1, retorno_ls);
  iniciaPackage(&packageRec);
  while(!feof(retorno_ls)){ 

    packageSend.inicio = MARCA_INICIO;
    packageSend.dest = CLIENT;
    packageSend.orig = SERVER;
    packageSend.tam = strlen(str);
    packageSend.seq = *seq;
    packageSend.tipo = LS_CONTENT;
    packageSend.data = malloc(packageSend.tam);
    memcpy(packageSend.data, str, packageSend.tam);

    // prepara e envia o buffer
    if( sendPackage(&packageSend, soquete) == -1 )
      exit(-1);

    espera = 0;
    while( !espera )
    {
      // espera receber pacote
      retorno = receivePackage(&packageRec, packageSend.orig, soquete);
      if( retorno == -1 ){
        exit(-1);
      } 

      // se o retorno for timeout, erro de paridade
      // ou se o pacote for NACK, envia o pacote novamente
      if( (retorno > 0) || ehPack(&packageRec, NACK) ){
        if( sendPackage(&packageSend, soquete) < 0 )
          exit(-1);
      } else if( ehPack(&packageRec, ACK) ){
        espera = 1;        
      }

      resetPackage(&packageRec);
      
    }

    fgets(str, TAM_DATA+1, retorno_ls);

    incrementaSeq(seq);

    resetPackage(&packageSend);

  }

  // prepara o pacote de final de transmissão
  packageSend.inicio = MARCA_INICIO;
  packageSend.dest = CLIENT;
  packageSend.orig = SERVER;
  packageSend.tam = 0;
  packageSend.seq = *seq;
  packageSend.tipo = EOT;
  packageSend.data = NULL;

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  // envia pacote de final de transmissão e aguarda ACK
  espera = 0;
  while( !espera )
  {
    resetPackage(&packageRec);
    // espera receber pacote
    retorno = receivePackage(&packageRec, packageSend.orig, soquete);
    if( retorno == -1 ){
      exit(-1);
    } 

    // se o retorno for timeout, erro de paridade
    // ou se o pacote for NACK, envia o pacote novamente
    if( (retorno > 0) || ehPack(&packageRec, NACK) ){
      if( sendPackage(&packageSend, soquete) < 0 )
        exit(-1);
    } else if( ehPack(&packageRec, ACK) ){
      espera = 1;        
    }    
    
  }

  incrementaSeq(seq);

  // Libera memória
  resetPackage(&packageSend);
  resetPackage(&packageRec);

}

// Comando ver - server side
// Mostra o conteúdo do arquivo texto do servidor na tela do cliente
void comando_ver(kermitHuman *package, int *seq, int soquete)
{  

  kermitHuman packageSend, packageRec;

  FILE *arquivo;

  // abre <ARQUIVO> no servidor
  arquivo = fopen((char*) package->data, "r");
  if (arquivo == NULL){
    sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
    return;
  }

  char str[TAM_DATA+1];
  int espera, retorno;
  iniciaPackage(&packageRec);

  while(!feof(arquivo)){ 
    fgets(str, TAM_DATA+1, arquivo);

    packageSend.inicio = MARCA_INICIO;
    packageSend.dest = CLIENT;
    packageSend.orig = SERVER;
    packageSend.tam = strlen(str);
    packageSend.seq = *seq;
    packageSend.tipo = FILE_CONTENT;
    packageSend.data = malloc(packageSend.tam);
    memcpy(packageSend.data, str, packageSend.tam);

    if( sendPackage(&packageSend, soquete) < 0 )
      exit(-1);

    espera = 0;
    while( !espera )
    {
      // espera receber pacote
      retorno = receivePackage(&packageRec, packageSend.orig, soquete);
      if( retorno == -1 ){
        exit(-1);
      } 

      // se o retorno for timeout, erro de paridade
      // ou se o pacote for NACK, envia o pacote novamente
      if( (retorno > 0) || ehPack(&packageRec, NACK) ){
        if( sendPackage(&packageSend, soquete) < 0 )
          exit(-1);
      } else if( ehPack(&packageRec, ACK) ){
        espera = 1;        
      }

      resetPackage(&packageRec);
      
    }

    incrementaSeq(seq);

    resetPackage(&packageSend);

  }

  // prepara o pacote de final de transmissão
  packageSend.inicio = MARCA_INICIO;
  packageSend.dest = CLIENT;
  packageSend.orig = SERVER;
  packageSend.tam = 0;
  packageSend.seq = *seq;
  packageSend.tipo = EOT;
  packageSend.data = NULL;

  // envia pacote de final de transmissão e aguarda ACK
  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);
  
  espera = 0;
  while( !espera )
  {
    // espera receber pacote
    retorno = receivePackage(&packageRec, packageSend.orig, soquete);
    if( retorno == -1 ){
      exit(-1);
    } 

    // se o retorno for timeout, erro de paridade
    // ou se o pacote for NACK, envia o pacote novamente
    if( (retorno > 0) || ehPack(&packageRec, NACK) ){
      if( sendPackage(&packageSend, soquete) < 0 )
        exit(-1);
    } else if( ehPack(&packageRec, ACK) ){
      espera = 1;        
    }  

    resetPackage(&packageRec);  
    
  }

  incrementaSeq(seq);

  // Libera memória
  resetPackage(&packageSend);
  resetPackage(&packageRec);

}

// Comando linha - server side
// Mostra a linha <numero_linha> do arquivo <nome_arq> que esta no servidor na tela do cliente.
void comando_linha(kermitHuman *package, int *seq, int soquete)
{
  kermitHuman packageSend, packageRec;

  FILE *arquivo;  
  
  // abre <ARQUIVO> no servidor
  arquivo = fopen((char*) package->data, "r");
  if (arquivo == NULL){
    sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
    return;
  }  

  sendACK(package->orig, package->dest, seq, soquete);

  char str[TAM_DATA+1];
  int retorno;

  iniciaPackage(&packageRec);
  // espera receber o número da linha
  while( !ehPack(&packageRec, LINE_NUMBER) )
  {
    resetPackage(&packageRec);

    // espera receber pacote
    retorno = receivePackage(&packageRec, SERVER, soquete);
    if( retorno == -1 ){
      exit(-1);
    }

    // se o retorno for timeout, erro de paridade
    // ou se o pacote for NACK, envia o pacote novamente
    if( (retorno > 0) || ehPack(&packageRec, NACK) ){
      sendACK(package->orig, package->dest, seq, soquete);
    }
    
  }

  unsigned int linha, cont = 1;
  // pega bytes da esquerda e direita e transforma em unsigned int
  linha = (unsigned int) ((packageRec.data[0] << 8) | (packageRec.data[1]));

  // se a linha for menor que 1, ela não existe
  if( linha < 1 )
  {
    sendError(package->orig, package->dest, seq, package->tipo, -1, soquete);
    // Libera memória
    resetPackage(&packageRec);
    return;
  }

  // encontra a linha e a envia
  while( (!feof(arquivo)) && (cont <= linha) ){
    fgets(str, TAM_DATA+1, arquivo);
    // quando encontra linha, envia o pacote
    if( cont == linha )
    {
      // envia string   
      packageSend.inicio = MARCA_INICIO;
      packageSend.dest = CLIENT;
      packageSend.orig = SERVER;
      packageSend.tam = strlen(str);
      packageSend.seq = *seq;
      packageSend.tipo = FILE_CONTENT;
      packageSend.data = malloc(packageSend.tam);
      memcpy(packageSend.data, str, packageSend.tam);

      if( sendPackage(&packageSend, soquete) < 0 )
        exit(-1);

      resetPackage(&packageRec);
      // espera receber o ACK
      while( !ehPack(&packageRec, ACK) )
      {
        resetPackage(&packageRec);

        // espera receber pacote
        retorno = receivePackage(&packageRec, packageSend.orig, soquete);
        if( retorno == -1 ){
          exit(-1);
        } 

        // se o retorno for timeout, erro de paridade
        // ou se o pacote for NACK, envia o pacote novamente
        if( (retorno > 0) || ehPack(&packageRec, NACK) ){
          if( sendPackage(&packageSend, soquete) < 0 )
            exit(-1);
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
    // Libera memória
    resetPackage(&packageRec);
    return;
  }

  // prepara o pacote de final de transmissão
  packageSend.inicio = MARCA_INICIO;
  packageSend.dest = CLIENT;
  packageSend.orig = SERVER;
  packageSend.tam = 0;
  packageSend.seq = *seq;
  packageSend.tipo = EOT;
  free(packageSend.data);
  packageSend.data = NULL;

  // envia pacote de final de transmissão e aguarda ACK
  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);
  
  resetPackage(&packageRec);
  while( !ehPack(&packageRec, ACK) )
  {
    resetPackage(&packageRec);

    // espera receber pacote
    retorno = receivePackage(&packageRec, packageSend.orig, soquete);
    if( retorno == -1 ){
      exit(-1);
    } 

    // se o retorno for timeout, erro de paridade
    // ou se o pacote for NACK, envia o pacote novamente
    if( (retorno > 0) || ehPack(&packageRec, NACK) ){
      if( sendPackage(&packageSend, soquete) < 0 )
        exit(-1);
    } 
    
  }

  incrementaSeq(seq);

  // Libera memória
  resetPackage(&packageSend);
  resetPackage(&packageRec);
}

// Comando linhas - server side
// Mostra as linhas entre a <numero_linha_inicial> e <numero_linha_final> do arquivo <nome_arq>, que está no servidor, na tela do cliente.
void comando_linhas(kermitHuman *package, int *seq, int soquete)
{
  kermitHuman packageSend, packageRec;

  FILE *arquivo;
  
  // abre <ARQUIVO> no servidor
  arquivo = fopen((char*) package->data, "r");
  if (arquivo == NULL){
    sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
    return;
  }

  sendACK(package->orig, package->dest, seq, soquete);

  char str[TAM_DATA+1];
  int retorno;

  iniciaPackage(&packageRec);
  // espera receber o número das linhas inicial e final
  while( !ehPack(&packageRec, LINE_NUMBER) )
  {
    resetPackage(&packageRec);

    // espera receber pacote
    retorno = receivePackage(&packageRec, SERVER, soquete);
    if( retorno == -1 ){
      exit(-1);
    }

    // se o retorno for timeout, erro de paridade
    // ou se o pacote for NACK, envia o pacote novamente
    if( (retorno > 0) || ehPack(&packageRec, NACK) ){
      sendACK(package->orig, package->dest, seq, soquete);
    }
    
  }

  unsigned int linha_inicial, linha_final, cont = 1;

  // pega bytes da esquerda e direita e transforma em unsigned int
  linha_inicial = (unsigned int) ((packageRec.data[0] << 8) | (packageRec.data[1]));
  linha_final = (unsigned int) ((packageRec.data[2] << 8) | (packageRec.data[3]));

  // se a linha inicial for menor que 1, ela não existe
  if( linha_inicial < 1 )
  {
    sendError(package->orig, package->dest, seq, package->tipo, -1, soquete);
    // Libera memória
    resetPackage(&packageRec);
    return;
  }

  // verifica se linha final é menor que inicial
  // caso seja, será enviado até o fim do arquivo
  if( linha_final < linha_inicial )
    linha_final = 0;

  // encontra a linha inicial e começa o envio
  while( (!feof(arquivo)) && ( (cont <= linha_final) || (!linha_final) ) ){
    fgets(str, TAM_DATA+1, arquivo);

    if( cont >= linha_inicial )
    {
      // envia string   
      packageSend.inicio = MARCA_INICIO;
      packageSend.dest = CLIENT;
      packageSend.orig = SERVER;
      packageSend.tam = strlen(str);
      packageSend.seq = *seq;
      packageSend.tipo = FILE_CONTENT;
      packageSend.data = malloc(packageSend.tam);
      memcpy(packageSend.data, str, packageSend.tam);

      if( sendPackage(&packageSend, soquete) < 0 )
        exit(-1);

      resetPackage(&packageRec);
      // espera receber o ACK
      while( !ehPack(&packageRec, ACK) )
      {
        resetPackage(&packageRec);

        // espera receber pacote
        retorno = receivePackage(&packageRec, packageSend.orig, soquete);
        if( retorno == -1 ){
          exit(-1);
        } 

        // se o retorno for timeout, erro de paridade
        // ou se o pacote for NACK, envia o pacote novamente
        if( (retorno > 0) || ehPack(&packageRec, NACK) ){
          if( sendPackage(&packageSend, soquete) < 0 )
            exit(-1);
        }
        
      }

      incrementaSeq(seq);

    }
    
    if( str[strlen(str)-1] == '\n' )
      cont++;

  }

  // envia erro caso linha inicial não exista
  if( cont < linha_inicial )
  {
    sendError(package->orig, package->dest, seq, package->tipo, -1, soquete);
    // Libera memória
    resetPackage(&packageRec);
    return;
  }

  // prepara o pacote de final de transmissão
  packageSend.inicio = MARCA_INICIO;
  packageSend.dest = CLIENT;
  packageSend.orig = SERVER;
  packageSend.tam = 0;
  packageSend.seq = *seq;
  packageSend.tipo = EOT;
  free(packageSend.data);
  packageSend.data = NULL;

  // envia pacote de final de transmissão e aguarda ACK
  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);
  
  resetPackage(&packageRec);
  while( !ehPack(&packageRec, ACK) )
  {
    resetPackage(&packageRec);

    // espera receber pacote
    retorno = receivePackage(&packageRec, packageSend.orig, soquete);
    if( retorno == -1 ){
      exit(-1);
    } 

    // se o retorno for timeout, erro de paridade
    // ou se o pacote for NACK, envia o pacote novamente
    if( (retorno > 0) || ehPack(&packageRec, NACK) ){
      if( sendPackage(&packageSend, soquete) < 0 )
        exit(-1);
    } 
    
  }

  incrementaSeq(seq);

  // Libera memória
  resetPackage(&packageSend);
  resetPackage(&packageRec);

}