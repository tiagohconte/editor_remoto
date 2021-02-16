/*  
  Feito por:
    Tiago Henrique Conte

  Redes de Computadores 1 - BCC UFPR
  Trabalho Prático - Editor de Texto Remoto

  Biblioteca Protocolo Kermit
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "kermitProtocol.h"

void resetPackage(kermitHuman *package)
{
  package->dest = -1;
  package->orig = -1;
  package->tam = -1;
  package->seq = -1;
  package->tipo = -1;
  package->par = -1;
  package->data = NULL;
}

// Espera o pacote com o inicio 01111110 ser recebido
int waitPackage(char *buffer, int soquete)
{
  memset(buffer, 0, TAM_PACKAGE);

  // sleep(1);

  while ( (buffer[0] != '~') )
  {
    receiveBuffer(buffer, soquete);
  }

  if( buffer[0] == '~' )
    return 1;

  return -1;

}

// Espera por pacote de ACK ou NACK
// Retorna 1 em ACK, 2 em NACK e -1 para nenhum
int waitACK(char *buffer, int soquete)
{
  kermitHuman package;
  memset(buffer, 0, TAM_PACKAGE);

  while ( (buffer[0] != '~') )
  {
    receiveBuffer(buffer, soquete);
  }

  struct kermitBit *packageBit = (struct kermitBit *) buffer;

  readPackageBit(&package, packageBit);

  if( package.tipo == 8 )
    return 1;
  if( package.tipo == 9 )
    return 2;

  return -1;

}

// Prepara e envia o buffer
int sendBuffer(char *buffer, struct kermitBit *packageBit, int tam_data, int soquete)
{
  memset(buffer, 0, TAM_PACKAGE);
  
  strncpy(buffer, (char *) packageBit->header, 3);
  strncpy(buffer+3, (char *) packageBit->data, tam_data+1);

  if( write(soquete, buffer, TAM_PACKAGE) == -1 )
  {
    fprintf(stderr, "Erro no envio: %s\n", strerror(errno));
    return(-1);
  }

  return 1;
}

// Recebe o buffer
int receiveBuffer(char *buffer, int soquete)
{  
  int tamBuffer = read(soquete, buffer, TAM_PACKAGE);
  if( tamBuffer == -1 )
  {
    fprintf(stderr, "Erro no recebimento: %s\n", strerror(errno));
    return(-1);
  }  

  read(soquete, buffer, TAM_PACKAGE);

  return 1;
}

int verificaBits(unsigned char c, unsigned int num)
{
  return ( (c^num) == 0 );
}

void readPackageBit(kermitHuman *package, struct kermitBit *packageBit)
{
  // coleta os dois bits mais significativos do 2° byte, onde está o end. destino
  package->dest = (unsigned char) (packageBit->header[1] & 0xc0) >> 6; 
  // coleta os 3° e 4° bits mais significativos do 2° byte, onde está o end. origem
  package->orig = (unsigned char) (packageBit->header[1] & 0x30) >> 4;
  // coleta os 4 bits menos significativos do 2° byte, onde está o tamanho
  package->tam = (unsigned char) (packageBit->header[1] & 0x0F);

  // coleta os 4 bits mais significativos do 3° byte, onde está a sequencia
  package->seq = (unsigned char) (packageBit->header[2] & 0xF0) >> 4;
  // coleta os 4 bits menos significativos do 3° byte, onde está o tipo
  package->tipo = (unsigned char) packageBit->header[2] & 0x0F;

  package->data = (char *) malloc(package->tam);
  strncpy(package->data, packageBit->data, package->tam);

  package->par = (unsigned char) packageBit->data[package->tam];

  // printf("dest: %d, orig: %d, tam: %d\n", package->dest, package->orig, package->tam);
  // printf("seq: %d, tipo: %d\n", package->seq, package->tipo);
  // printf("data: %s, par: %d\n", package->data, package->par);
}

void writePackageBit(struct kermitBit *packageBit, kermitHuman *package)
{
  // insere marcador de início do pacote
  packageBit->header[0] = '~';

  // insere destino, origem e tamanho no 2° byte
  packageBit->header[1] = (unsigned char) ((package->dest << 6) | (package->orig << 4) | package->tam);

  // insere sequencia e tipo no 3° byte
  packageBit->header[2] = (unsigned char) ((package->seq << 4) | package->tipo);

  strncpy(packageBit->data, package->data, package->tam);
  packageBit->data[package->tam] = (unsigned char) package->par;
}

// Envia mensagem de acknowledge
void sendACK(char *buffer, int *seq, int soquete, int dest, int orig)
{  
  kermitHuman package;
  struct kermitBit packageBit;

  package.dest = dest;
  package.orig = orig;
  package.tam = 0;
  package.seq = *seq;
  package.tipo = 8;
  package.par = 0;
  package.data = NULL;

  writePackageBit(&packageBit, &package);
  
  if( sendBuffer(buffer, &packageBit, package.tam, soquete) < 0 )
    exit(-1);

}

// Envia mensagem de NOT acknowledge
void sendNACK(char *buffer, int *seq, int soquete, int dest, int orig)
{  
  kermitHuman package;
  struct kermitBit packageBit;

  package.dest = dest;
  package.orig = orig;
  package.tam = 0;
  package.seq = *seq;
  package.tipo = 9;
  package.par = 0;
  package.data = NULL;

  writePackageBit(&packageBit, &package);
  
  if( sendBuffer(buffer, &packageBit, package.tam, soquete) < 0 )
    exit(-1);

}