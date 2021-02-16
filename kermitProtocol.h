/*  
  Feito por:
    Tiago Henrique Conte

  Redes de Computadores 1 - BCC UFPR
  Trabalho Prático - Editor de Texto Remoto

  Biblioteca Protocolo Kermit
*/

#ifndef __KERMITPROTOCOL__
#define __KERMITPROTOCOL__

#define TAM_PACKAGE 19  // tamanho máximo do pacote, em bytes
#define TIMEOUT 5 // tentativas até timeout

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

// Espera o pacote ser recebido
int waitPackage(char *buffer, int soquete);

int waitACK(char *buffer, int soquete);

// Prepara e envia o buffer
int sendBuffer(char *buffer, struct kermitBit *packageBit, int tam_data, int soquete);

// Recebe o buffer
int receiveBuffer(char *buffer, int soquete);

int verificaBits(unsigned char c, unsigned int num);

void readPackageBit(kermitHuman *package, struct kermitBit *packageBit);

void writePackageBit(struct kermitBit *packageBit, kermitHuman *package);

// Envia mensagem de acknowledge
void sendACK(char *buffer, int *seq, int soquete, int dest, int orig);

// Envia mensagem de NOT acknowledge
void sendNACK(char *buffer, int *seq, int soquete, int dest, int orig);

#endif