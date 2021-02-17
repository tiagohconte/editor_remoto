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
  package->inicio = -1;
  package->dest = -1;
  package->orig = -1;
  package->tam = -1;
  package->seq = -1;
  package->tipo = -1;
  package->par = -1;
  package->data = NULL;
}

// Prepara e envia o buffer
int sendPackage(kermitHuman *package, int soquete)
{
  char buffer[TAM_PACKAGE];
  memset(buffer, 0, TAM_PACKAGE);
  struct kermitBit packageBit;

  // insere marcador de início do pacote
  packageBit.header[0] = (unsigned char) package->inicio;

  // insere destino, origem e tamanho no 2° byte
  packageBit.header[1] = (unsigned char) ((package->dest << 6) | (package->orig << 4) | package->tam);

  // insere sequencia e tipo no 3° byte
  packageBit.header[2] = (unsigned char) ((package->seq << 4) | package->tipo);

  if( package->tam > 0 )
  {
    strncpy(packageBit.data, package->data, package->tam);    
  }
  packageBit.data[package->tam] = (unsigned char) package->par;

  strncpy(buffer, (char *) packageBit.header, 3);
  strncpy(buffer+3, (char *) packageBit.data, package->tam+1);

  if( write(soquete, buffer, TAM_PACKAGE) == -1 )
  {
    fprintf(stderr, "Erro no envio: %s\n", strerror(errno));
    return(-1);
  }

  read(soquete, buffer, TAM_PACKAGE);

  #ifdef DEBUG
  printf("\nENVIADO\n");
  printf("dest: %d, orig: %d, tam: %d\n", package->dest, package->orig, package->tam);
  printf("seq: %d, tipo: %d\n", package->seq, package->tipo);
  printf("data: %s, par: %d\n", package->data, package->par);
  printf("FIM ENVIADO\n");
  #endif

  return 1;
}

// Recebe o buffer
int receivePackage(kermitHuman *package, int soquete)
{  
  char buffer[TAM_PACKAGE];
  memset(buffer, 0, TAM_PACKAGE);

  int tamBuffer = read(soquete, buffer, TAM_PACKAGE);
  if( tamBuffer == -1 )
  {
    fprintf(stderr, "Erro no recebimento: %s\n", strerror(errno));
    return(-1);
  }  

  struct kermitBit *packageBit = (struct kermitBit *) buffer;

  // coleta os 1° byte, onde está o marcador de inicio
  package->inicio = (unsigned char) (packageBit->header[0]); 
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

  read(soquete, buffer, TAM_PACKAGE);

  #ifdef DEBUG
  if(package->dest != 0){
    printf("\nRECEBIDO\n");
    printf("dest: %d, orig: %d, tam: %d\n", package->dest, package->orig, package->tam);
    printf("seq: %d, tipo: %d\n", package->seq, package->tipo);
    printf("data: %s, par: %d\n", package->data, package->par);
    printf("FIM RECEBIDO\n");
  }
  #endif

  return 1;
}

// Envia mensagem de acknowledge
void sendACK(int dest, int orig, int soquete)
{  
  kermitHuman package;

  package.inicio = 126;
  package.dest = dest;
  package.orig = orig;
  package.tam = 0;
  package.seq = 0;
  package.tipo = 8;
  package.par = 0;
  package.data = NULL;
  
  if( sendPackage(&package, soquete) < 0 )
    exit(-1);

}

// Envia mensagem de NOT acknowledge
void sendNACK(int dest, int orig, int soquete)
{  
  kermitHuman package;

  package.inicio = 126;
  package.dest = dest;
  package.orig = orig;
  package.tam = 0;
  package.seq = 0;
  package.tipo = 9;
  package.par = 0;
  package.data = NULL;
  
  if( sendPackage(&package, soquete) < 0 )
    exit(-1);

}

// Espera o pacote com o inicio 01111110 ser recebido
int waitPackage(kermitHuman *package, int soquete)
{
  resetPackage(package);

  while( package->inicio != 126 )
  {
    if( receivePackage(package, soquete) < 0 )
      return(-1);
  }
  
  return 1;

}