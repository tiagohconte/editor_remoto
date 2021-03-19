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

// Comando exit
// Sai do programa
void comando_exit(int *seq, int soquete)
{
  kermitHuman packageSend;

  packageSend.inicio = MARCA_INICIO;
  packageSend.dest = SERVER;
  packageSend.orig = CLIENT;
  packageSend.tam = 0;
  packageSend.seq = *seq;
  packageSend.tipo = 6;
  packageSend.data = NULL;

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

}

// Comando cd - client side
// Executa change directory no server
void comando_cd(int *seq, int soquete)
{
  kermitHuman packageSend, packageRec;

  int retorno;
  char dir[TAM_DATA+1];
  scanf("%s", dir);
  dir[TAM_DATA] = '\0';

  packageSend.inicio = MARCA_INICIO;
  packageSend.dest = SERVER;
  packageSend.orig = CLIENT;
  packageSend.tam = strlen(dir);
  packageSend.seq = *seq;
  packageSend.tipo = 0;
  packageSend.data = malloc(packageSend.tam);
  memcpy(packageSend.data, dir, packageSend.tam);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  // quando tipo = ACK, sucesso no comando cd
  // quando tipo = ERROR, houve erro
  iniciaPackage(&packageRec);
  while( !ehPack(&packageRec, ACK) && !ehPack(&packageRec, ERROR))
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

  if( ehPack(&packageRec, ERROR) )
    printError(&packageRec);

  // Libera memória
  resetPackage(&packageSend);
  resetPackage(&packageRec);

}

// Comando ls - client side
// Executa ls no server
void comando_ls(int *seq, int soquete)
{  
  kermitHuman packageSend, packageRec;
  int retorno;

  packageSend.inicio = MARCA_INICIO;
  packageSend.dest = SERVER;
  packageSend.orig = CLIENT;
  packageSend.tam = 0;
  packageSend.seq = *seq;
  packageSend.tipo = 1;
  packageSend.data = NULL;
  
  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  // quando tipo = LS_CONTENT, sucesso no comando inicial ls
  // quando tipo = EOT, end of transmission comando ls
  iniciaPackage(&packageRec);
  while( !ehPack(&packageRec, LS_CONTENT) && !ehPack(&packageRec, EOT) )
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

  if( ehPack(&packageRec, LS_CONTENT) )
    printf("%s", packageRec.data);

  incrementaSeq(seq);

  sendACK(packageRec.orig, packageRec.dest, seq, soquete);

  int seqEsperada = packageRec.seq;
  incrementaSeq(&seqEsperada);

  // quando tipo = EOT, acabou a transmissão do ls
  while( !ehPack(&packageRec, EOT) )
  {

    resetPackage(&packageRec);

    // espera receber os dados do comando ls
    retorno = receivePackage(&packageRec, packageSend.orig, soquete);
    if( retorno == -1 ){
      exit(-1);
    } else if( retorno > 0 ){ 
      // caso seja timeout ou erro na paridade, envia NACK
      sendNACK(packageRec.orig, packageRec.dest, seq, soquete);
    } else 
    {
      // verifica se é pacote tipo conteúdo ls
      if( ehPack(&packageRec, LS_CONTENT) )
      {
        // verifica a sequência
        if( (packageRec.seq == seqEsperada) )
        {          
          printf("%s", packageRec.data);
          sendACK(packageRec.orig, packageRec.dest, seq, soquete);
          incrementaSeq(&seqEsperada);

        } else 
        {    
          sendNACK(packageRec.orig, packageRec.dest, seq, soquete);
        }
      }
    }

  }

  sendACK(packageRec.orig, packageRec.dest, seq, soquete);

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
  int retorno;

  packageSend.inicio = MARCA_INICIO;
  packageSend.dest = SERVER;
  packageSend.orig = CLIENT;
  packageSend.tam = strlen(arq);
  packageSend.seq = *seq;
  packageSend.tipo = 2;
  packageSend.data = malloc(packageSend.tam);
  memcpy(packageSend.data, arq, packageSend.tam);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  // enquanto a chegada não for um pacote de erro ou de conteúdo
  // continua enviando o pacote inicial
  iniciaPackage(&packageRec);
  while( !ehPack(&packageRec, FILE_CONTENT) && !ehPack(&packageRec, ERROR) && !ehPack(&packageRec, EOT) )
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
  sendACK(packageRec.orig, packageRec.dest, seq, soquete);

  // se não for conteúdo, sai da função
  if( !ehPack(&packageRec, FILE_CONTENT) ){
    // se for erro, imprime o erro
    if( ehPack(&packageRec, ERROR) )
      printError(&packageRec);
    // Libera memória
    resetPackage(&packageSend);
    resetPackage(&packageRec);
    return;
  }  

  // seta primeiro pacote da sequencia incrementa o esperado
  int seqEsperada = packageRec.seq;
  incrementaSeq(&seqEsperada);

  // imprime número da primeira linha
  int linha = 1;
  printf("%d ", linha++);
  // imprime o primeiro conteúdo recebido
  printf("%s", packageRec.data);
  if( packageRec.data[strlen( (char*) packageRec.data )-1] == '\n' )
    printf("%d ", linha++);

  // quando tipo = EOT, acabou a transmissão
  while( !ehPack(&packageRec, EOT) )
  {  
    resetPackage(&packageRec);

    // espera receber o pacote com conteúdo do arquivo
    retorno = receivePackage(&packageRec, packageSend.orig, soquete);
    if( retorno == -1 ){
      exit(-1);
    } else if( retorno > 0 ){ 
      // caso seja timeout ou erro na paridade, envia NACK
      sendNACK(packageRec.orig, packageRec.dest, seq, soquete);
    } else {    
      // verifica se é pacote tipo conteúdo arquivo
      if( ehPack(&packageRec, FILE_CONTENT) )
      {      
        // verifica a sequência 
        if( packageRec.seq == seqEsperada )
        {      
          #ifndef DEBUG
          printf("%s", packageRec.data);
          if( packageRec.data[strlen( (char*) packageRec.data )-1] == '\n' )
            printf("%d ", linha++); 
          #endif         

          sendACK(packageRec.orig, packageRec.dest, seq, soquete);

          incrementaSeq(&seqEsperada);
        } else {    
          sendNACK(packageRec.orig, packageRec.dest, seq, soquete);
        }
      }
    }    

  }

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
  int retorno;

  unsigned int linha;
  scanf("%d", &linha);
  
  char arq[TAM_DATA+1];
  scanf("%s", arq);
  arq[TAM_DATA] = '\0';

  packageSend.inicio = MARCA_INICIO;
  packageSend.dest = SERVER;
  packageSend.orig = CLIENT;
  packageSend.tam = strlen(arq);
  packageSend.seq = *seq;
  packageSend.tipo = 3;
  packageSend.data = malloc(packageSend.tam);
  memcpy(packageSend.data, arq, packageSend.tam);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  // quando tipo = ACK, sucesso no pacote inicial do comando linha
  // quando tipo = ERROR, houve erro
  iniciaPackage(&packageRec);
  while( !ehPack(&packageRec, ACK) && !ehPack(&packageRec, ERROR) )
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

  // caso tenha dado erro, finaliza a função
  if( ehPack(&packageRec, ERROR) )
  {
    printError(&packageRec);
    // Libera memória
    resetPackage(&packageSend);
    resetPackage(&packageRec);
    return;
  }

  // envia a linha desejada
  packageSend.inicio = MARCA_INICIO;
  packageSend.dest = SERVER;
  packageSend.orig = CLIENT;
  packageSend.tam = 2;    // tamanho necessário para armazenar UM inteiro
  packageSend.seq = *seq;
  packageSend.tipo = LINE_NUMBER;
  // aloca espaço para dados e insere os bytes dos numeros inteiros
  packageSend.data = malloc(packageSend.tam);
  // pega bytes da esquerda e direita e transforma em unsigned char
  packageSend.data[0] = (unsigned char) ((linha >> 8) & 0xff); 
  packageSend.data[1] = (unsigned char) (linha & 0xff);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  // enquanto a chegada não for um pacote de erro ou de conteúdo
  // continua enviando o pacote de linha
  resetPackage(&packageRec);
  while( !ehPack(&packageRec, FILE_CONTENT) && !ehPack(&packageRec, ERROR) && !ehPack(&packageRec, EOT) )
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
  sendACK(packageRec.orig, packageRec.dest, seq, soquete);

  // se não for conteúdo, sai da função
  if( !ehPack(&packageRec, FILE_CONTENT) ){
    // se for erro, imprime o erro
    if( ehPack(&packageRec, ERROR) )
      printError(&packageRec);
    // Libera memória
    resetPackage(&packageSend);
    resetPackage(&packageRec);
    return;
  }

  #ifndef DEBUG
  printf("%d %s", linha, packageRec.data);
  #endif 

  // seta sequencia esperada e incrementa
  int seqEsperada = packageRec.seq;
  incrementaSeq(&seqEsperada);

  // espera receber conteudo do arquivo
  // quando tipo = EOT, acabou a transmissão
  // quando tipo = ERROR, houve erro
  resetPackage(&packageRec);
  while( !ehPack(&packageRec, EOT) && !ehPack(&packageRec, ERROR) )
  {
    resetPackage(&packageRec);

    retorno = receivePackage(&packageRec, packageSend.orig, soquete);
    if( retorno == -1 ){
      exit(-1);
    } else if( retorno > 0 ){ 
      // caso seja timeout ou erro na paridade, envia NACK
      sendNACK(packageRec.orig, packageRec.dest, seq, soquete);
    } else {      
      // verifica se é pacote tipo conteúdo arquivo
      if( ehPack(&packageRec, FILE_CONTENT) )
      {
        // verifica a sequência 
        if( packageRec.seq == seqEsperada )
        {         
          #ifndef DEBUG
          printf("%s", packageRec.data);
          #endif         

          sendACK(packageRec.orig, packageRec.dest, seq, soquete);

          incrementaSeq(&seqEsperada);
        } else {
          sendNACK(packageRec.orig, packageRec.dest, seq, soquete);
        }
      }
    }

  }

  // verifica se o tipo é erro
  if( ehPack(&packageRec, ERROR) )
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
  int retorno;

  unsigned int linha_inicial;
  scanf("%d", &linha_inicial);

  unsigned int linha_final;
  scanf("%d", &linha_final);

  char arq[TAM_DATA+1];
  scanf("%s", arq);
  arq[TAM_DATA] = '\0';

  packageSend.inicio = MARCA_INICIO;
  packageSend.dest = SERVER;
  packageSend.orig = CLIENT;
  packageSend.tam = strlen(arq);
  packageSend.seq = *seq;
  packageSend.tipo = 4;
  packageSend.data = malloc(packageSend.tam);
  memcpy(packageSend.data, arq, packageSend.tam);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  // quando tipo = ACK, sucesso no pacote inicial do comando linha
  // quando tipo = ERROR, houve erro
  iniciaPackage(&packageRec);
  while( !ehPack(&packageRec, ACK) && !ehPack(&packageRec, ERROR) )
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

  // caso tenha dado erro, imprime o erro e finaliza a função
  if( ehPack(&packageRec, ERROR) )
  {
    printError(&packageRec);
    // Libera memória
    resetPackage(&packageSend);
    resetPackage(&packageRec);
    return;
  }

  // envia as linhas desejadas
  packageSend.inicio = MARCA_INICIO;
  packageSend.dest = SERVER;
  packageSend.orig = CLIENT;
  packageSend.tam = 4;    // tamanho necessário para armazenar DOIS inteiros
  packageSend.seq = *seq;
  packageSend.tipo = LINE_NUMBER;
  // aloca espaço para dados e insere os bytes dos numeros inteiros
  packageSend.data = malloc(packageSend.tam);
  // pega bytes da esquerda e direita e transforma em char
  packageSend.data[0] = (unsigned char) ((linha_inicial >> 8) & 0xff); 
  packageSend.data[1] = (unsigned char) (linha_inicial & 0xff);
  packageSend.data[2] = (unsigned char) ((linha_final >> 8) & 0xff); 
  packageSend.data[3] = (unsigned char) (linha_final & 0xff);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

  // enquanto a chegada não for um pacote de erro ou de conteúdo
  // continua enviando o pacote de linhas
  resetPackage(&packageRec);
  while( !ehPack(&packageRec, FILE_CONTENT) && !ehPack(&packageRec, ERROR) && !ehPack(&packageRec, EOT) )
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
  sendACK(packageRec.orig, packageRec.dest, seq, soquete);

  // se não for conteúdo, sai da função
  if( !ehPack(&packageRec, FILE_CONTENT) ){
    // se for erro, imprime o erro
    if( ehPack(&packageRec, ERROR) )
      printError(&packageRec);
    // Libera memória
    resetPackage(&packageSend);
    resetPackage(&packageRec);
    return;
  }

  int linha = linha_inicial, imprime_linha = 0;

  #ifndef DEBUG
  printf("%d %s", linha++, packageRec.data);
  #endif 

  // seta sequencia esperada e incrementa
  int seqEsperada = packageRec.seq;
  incrementaSeq(&seqEsperada);
  
  // espera receber conteudo do arquivo
  // quando tipo = EOT, acabou a transmissão
  // quando tipo = ERROR, houve erro
  resetPackage(&packageRec);
  while( !ehPack(&packageRec, EOT) && !ehPack(&packageRec, ERROR) )
  {
    resetPackage(&packageRec);    

    retorno = receivePackage(&packageRec, packageSend.orig, soquete);
    if( retorno == -1 ){
      exit(-1);
    } else if( retorno > 0 ){ 
      // caso seja timeout ou erro na paridade, envia NACK
      sendNACK(packageRec.orig, packageRec.dest, seq, soquete);
    } else {      
      // verifica se é pacote tipo conteúdo arquivo
      if( ehPack(&packageRec, FILE_CONTENT) )
      {        
        // verifica a sequência 
        if( packageRec.seq == seqEsperada )
        {         
          #ifndef DEBUG
          if( imprime_linha ){
            printf("%d ", linha++);
            imprime_linha = 0;
          }
          printf("%s", packageRec.data);
          #endif         

          sendACK(packageRec.orig, packageRec.dest, seq, soquete);

          incrementaSeq(&seqEsperada);
        } else {
          sendNACK(packageRec.orig, packageRec.dest, seq, soquete);
        }

        if( packageRec.data[packageRec.tam-1] == '\n' )
          imprime_linha = 1;
      }
    }    

  }

  // verifica se o tipo é erro
  if( ehPack(&packageRec, ERROR) )
    printError(&packageRec);
  else{
    sendACK(packageRec.orig, packageRec.dest, seq, soquete);
    printf("\n");
  }

  // Libera memória
  resetPackage(&packageSend);
  resetPackage(&packageRec);

}