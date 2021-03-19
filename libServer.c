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

  fclose(arquivo);

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

  fclose(arquivo);

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

  fclose(arquivo);

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

// Comando edit - server side
// troca a linha <numero_linha> do arquivo <nome_arq>, que está no servidor, pelo texto <NOVO_TEXTO> que deve ser digitado entre aspas.
void comando_edit(kermitHuman *package, int *seq, int soquete)
{
  kermitHuman packageRec;

  FILE *arquivo; 
  tad_texto buffer; 
  
  // abre <ARQUIVO> no servidor
  arquivo = fopen((char*) package->data, "r");
  if (arquivo == NULL){
    sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
    return;
  }  

  // aloca o arquivo de texto na memória
  if (!aloca_arq(arquivo, &buffer)){
    sendError(package->orig, package->dest, seq, package->tipo, -2, soquete);
    return;
  }

  fclose(arquivo);

  sendACK(package->orig, package->dest, seq, soquete);

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

  unsigned int linha;
  // pega bytes da esquerda e direita e transforma em unsigned int
  linha = (unsigned int) ((packageRec.data[0] << 8) | (packageRec.data[1]));

  // se a linha for menor que 1 ou maior que o total do arquivo, ela não existe
  if( (linha < 1) || (linha > buffer.num_linhas))
  {
    sendError(package->orig, package->dest, seq, package->tipo, -1, soquete);
    // Libera memória
    resetPackage(&packageRec);
    return;
  }

  sendACK(package->orig, package->dest, seq, soquete);

  // MODIFICAR O ARQUIVO DE TEXTO

  // seta sequencia esperada e incrementa
  int seqEsperada = packageRec.seq;
  incrementaSeq(&seqEsperada);  

  // tamanho da linha
  int tam = 0;  

  // libera espaço na memória referente a linha
  free(buffer.linhas[linha-1]);
  buffer.linhas[linha-1] = NULL;

  // espera receber dados, parar quando receber EOT  
  resetPackage(&packageRec);
  while( !ehPack(&packageRec, EOT) )
  {
    resetPackage(&packageRec);

    retorno = receivePackage(&packageRec, SERVER, soquete);
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
          // realoca a linha para escrever novos dados
          buffer.linhas[linha-1] = (unsigned char *) realloc(buffer.linhas[linha-1], tam + packageRec.tam );
          memcpy(buffer.linhas[linha-1]+tam, packageRec.data, packageRec.tam);
          tam += packageRec.tam;
          buffer.linhas[linha-1][tam] = '\0';

          sendACK(packageRec.orig, packageRec.dest, seq, soquete);

          incrementaSeq(&seqEsperada);
        } else {
          sendNACK(packageRec.orig, packageRec.dest, seq, soquete);
        }
      }
    }
    
  }

  sendACK(packageRec.orig, packageRec.dest, seq, soquete);

  // reescreve ARQUIVO
  arquivo = fopen((char*) package->data, "w");

  for(int i = 0; i < buffer.num_linhas; i++)
  {
    fprintf(arquivo, "%s\n", buffer.linhas[i]);
    free(buffer.linhas[i]);
    buffer.linhas[i] = NULL;
  }
  free(buffer.linhas);
  buffer.linhas = NULL;
  fclose(arquivo);

  // Libera memória
  resetPackage(&packageRec);

}

// Aloca um arquivo de texto em um buffer
int aloca_arq(FILE *arquivo, tad_texto *buffer)
{
  int tam_str = 0, num_aloc = 1; // Registra o número de alocações feitas

  buffer->num_linhas = 0;
  buffer->linhas = NULL;

  // Alocação de espaço inicial na memória
  buffer->linhas = (unsigned char **) realloc(buffer->linhas, (num_aloc * TAM_ALOC) * sizeof(buffer->linhas));
  if(buffer == NULL)
  {
    #ifdef DEBUG
    fprintf(stderr, "Erro na alocação inicial do arquivo.\n");
    #endif
    return 0;
  }

  char str[TAM_LINHA+1];
  while(!feof(arquivo))
  {
    // Verifica se a alocação feita anteriormente ainda é suficiente para o número de linhas
    if(buffer->num_linhas == (num_aloc * TAM_ALOC))
    {
      num_aloc++;
      buffer->linhas = (unsigned char **) realloc(buffer->linhas, (num_aloc * TAM_ALOC) * sizeof(buffer->linhas));
    }

    fgets(str, TAM_LINHA, arquivo);
    // Aloca espaço para a palavra 
    tam_str = strlen(str);  
    buffer->linhas[buffer->num_linhas] = (unsigned char *) malloc(tam_str * sizeof(unsigned char));
    if(buffer->linhas[buffer->num_linhas] == NULL)
    {
      #ifdef DEBUG
      fprintf(stderr, "Erro ao alocar palavra.\n");
      #endif
      return 0;
    }
    str[tam_str-1] = '\0';
    // copia a linha para o buffer
    memcpy(buffer->linhas[buffer->num_linhas], str, tam_str);    
    buffer->num_linhas++;
  }

  // #ifdef DEBUG
  // for(int i = 0; i < buffer->num_linhas; i++)
  //   printf("%s\n", buffer->linhas[i]);
  // #endif

  return 1;
}