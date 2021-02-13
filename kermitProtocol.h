/*  
  Feito por:
    Tiago Henrique Conte

  Redes de Computadores 1 - BCC UFPR
  Trabalho Prático - Editor de Texto Remoto

  Biblioteca Protocolo Kermit
*/

#ifndef __KERMITPROTOCOL__
#define __KERMITPROTOCOL__

#define TAM_PACKAGE 19  // em bytes

// Buffer kermit package
struct kermitBit
{
  unsigned char header[3];     // possui 24 bits
  char data[16];     // 15 bytes de dados + 1 byte de paridade
};

// Human readable kermit package
typedef struct kermitHuman
{
  unsigned int dest;
  unsigned int orig;
  unsigned int tam;
  unsigned int seq;
  unsigned int tipo;
  unsigned int par;
  char *data;
} kermitHuman;

void resetPackage(kermitHuman *package);

int verificaBits(unsigned char c, unsigned int num);

#endif