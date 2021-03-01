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

  char dir[TAM_DATA+1];
  scanf("%s", dir);
  dir[TAM_DATA] = '\0';

  packageSend.inicio = 126;
  packageSend.dest = 2;
  packageSend.orig = 1;
  packageSend.tam = strlen(dir);
  packageSend.seq = *seq;
  packageSend.tipo = 0;
  packageSend.data = malloc(packageSend.tam);
  memcpy(packageSend.data, dir, packageSend.tam);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  // quando tipo = 8, sucesso no comando cd
  // quando tipo = 15, houve erro
  iniciaPackage(&packageRec);
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

  // Libera memória
  resetPackage(&packageSend);
  resetPackage(&packageRec);

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
  packageSend.data = NULL;
  
  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  incrementaSeq(seq);

  int seqEsperada = -1;
  iniciaPackage(&packageRec);
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

  // Libera memória
  resetPackage(&packageSend);
  resetPackage(&packageRec);

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
  packageSend.data = malloc(packageSend.tam);
  memcpy(packageSend.data, arq, packageSend.tam);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  incrementaSeq(seq);

  int seqEsperada = -1;
  int linha = 1;

  iniciaPackage(&packageRec);
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
          if( packageRec.data[strlen( (char*) packageRec.data )-1] == '\n' )
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

  // Libera memória
  resetPackage(&packageSend);
  resetPackage(&packageRec);

}

// Comando linha - client side
// Mostra a linha <numero_linha> do arquivo <nome_arq> que esta no servidor na tela do cliente.
void comando_linha(int *seq, int soquete)
{
  kermitHuman packageSend, packageRec;

  unsigned int linha;
  scanf("%d", &linha);
  
  char arq[TAM_DATA+1];
  scanf("%s", arq);
  arq[TAM_DATA] = '\0';

  packageSend.inicio = 126;
  packageSend.dest = 2;
  packageSend.orig = 1;
  packageSend.tam = strlen(arq);
  packageSend.seq = *seq;
  packageSend.tipo = 3;
  packageSend.data = malloc(packageSend.tam);
  memcpy(packageSend.data, arq, packageSend.tam);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  // quando tipo = 8, sucesso no pacote inicial do comando linha
  // quando tipo = 15, houve erro
  iniciaPackage(&packageRec);
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

  // caso tenha dado erro, finaliza a função
  if( packageRec.tipo == 15 )
  {
    printError(&packageRec);
    return;
  }

  // envia a linha desejada
  packageSend.inicio = 126;
  packageSend.dest = 2;
  packageSend.orig = 1;
  packageSend.tam = 2;    // tamanho necessário para armazenar UM inteiro
  packageSend.seq = *seq;
  packageSend.tipo = 10;
  // aloca espaço para dados e insere os bytes dos numeros inteiros
  packageSend.data = malloc(packageSend.tam);
  // pega bytes da esquerda e direita e transforma em unsigned char
  packageSend.data[0] = (unsigned char) ((linha >> 8) & 0xff); 
  packageSend.data[1] = (unsigned char) (linha & 0xff);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  int seqEsperada = packageRec.seq;
  incrementaSeq(&seqEsperada);

  resetPackage(&packageRec);
  // espera receber os dados do comando ver
  if( waitPackage(&packageRec, soquete) == -1 ){
    exit(-1);
  }

  // quando tipo = 13, acabou a transmissão
  // quando tipo = 15, houve erro
  if( packageRec.tipo != 15 )
    printf("%d ", linha);

  while( packageRec.tipo != 13 && packageRec.tipo != 15 )
  {
    
    // verifica se o destino está correto
    if( packageRec.dest == packageSend.orig )
    {
      
      // verifica a sequência 
      if( packageRec.seq == seqEsperada )
      {
        // verifica se o tipo é conteúdo arquivo
        if( (packageRec.tipo == 12) )
        {         
          printf("%s", packageRec.data);        

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
  else{
    sendACK(packageRec.orig, packageRec.dest, seq, soquete);
    printf("\n");
  }

  // Libera memória
  resetPackage(&packageSend);
  resetPackage(&packageRec);
  
}

// Comando linhas - client side
// Mostra as linhas entre a <numero_linha_inicial> e <numero_linha_final> do arquivo <nome_arq>, que está no servidor, na tela do cliente.
void comando_linhas(int *seq, int soquete)
{

  kermitHuman packageSend, packageRec;

  unsigned int linha_inicial;
  scanf("%d", &linha_inicial);

  unsigned int linha_final;
  scanf("%d", &linha_final);

  char arq[TAM_DATA+1];
  scanf("%s", arq);
  arq[TAM_DATA] = '\0';

  packageSend.inicio = 126;
  packageSend.dest = 2;
  packageSend.orig = 1;
  packageSend.tam = strlen(arq);
  packageSend.seq = *seq;
  packageSend.tipo = 4;
  packageSend.data = malloc(packageSend.tam);
  memcpy(packageSend.data, arq, packageSend.tam);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);  

  // quando tipo = 8, sucesso no pacote inicial do comando linhas
  // quando tipo = 15, houve erro
  iniciaPackage(&packageRec);
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

  // caso tenha dado erro, imprime o erro e finaliza a função
  if( packageRec.tipo == 15 )
  {
    printError(&packageRec);
    return;
  }

  // envia as linhas desejadas
  packageSend.inicio = 126;
  packageSend.dest = 2;
  packageSend.orig = 1;
  packageSend.tam = 4;    // tamanho necessário para armazenar DOIS inteiros
  packageSend.seq = *seq;
  packageSend.tipo = 10;
  // aloca espaço para dados e insere os bytes dos numeros inteiros
  packageSend.data = malloc(packageSend.tam);
  // pega bytes da esquerda e direita e transforma em char
  packageSend.data[0] = (unsigned char) ((linha_inicial >> 8) & 0xff); 
  packageSend.data[1] = (unsigned char) (linha_inicial & 0xff);
  packageSend.data[2] = (unsigned char) ((linha_final >> 8) & 0xff); 
  packageSend.data[3] = (unsigned char) (linha_final & 0xff);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  int seqEsperada = packageRec.seq;
  incrementaSeq(&seqEsperada);

  resetPackage(&packageRec);
  // espera receber os dados do comando ver
  if( waitPackage(&packageRec, soquete) == -1 ){
    exit(-1);
  }

  // quando tipo = 13, acabou a transmissão
  // quando tipo = 15, houve erro
  /*if( packageRec.tipo != 15 )
    printf("%d ", linha_inicial);*/

  int linha = linha_inicial, imprime_linha = 1;

  while( packageRec.tipo != 13 && packageRec.tipo != 15 )
  {    

    if( imprime_linha ){
      printf("%d ", linha++);
      imprime_linha = 0;
    }
    
    // verifica se o destino está correto
    if( packageRec.dest == packageSend.orig )
    {
      
      // verifica a sequência 
      if( packageRec.seq == seqEsperada )
      {
        // verifica se o tipo é conteúdo arquivo
        if( (packageRec.tipo == 12) )
        {         
          printf("%s", packageRec.data);                        

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

    if( packageRec.data[strlen( (char*) packageRec.data )-1] == '\n' )
      imprime_linha = 1;

    resetPackage(&packageRec);

    // espera receber os dados do comando ver
    if( waitPackage(&packageRec, soquete) == -1 ){
      exit(-1);
    }
  }

  // verifica se o tipo é erro
  if( (packageRec.tipo == 15) )
    printError(&packageRec);
  else{
    sendACK(packageRec.orig, packageRec.dest, seq, soquete);
    printf("\n");
  }

  // Libera memória
  resetPackage(&packageSend);
  resetPackage(&packageRec);

}