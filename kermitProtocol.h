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
  unsigned int inicio;
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
int waitPackage(kermitHuman *package, int soquete);

// Prepara e envia o buffer
int sendPackage(kermitHuman *package, int soquete);

// Recebe o buffer
int receivePackage(kermitHuman *package, int soquete);

// Envia mensagem de acknowledge
void sendACK(int dest, int orig, int *seq, int soquete);

// Envia mensagem de NOT acknowledge
void sendNACK(int dest, int orig, int *seq, int soquete);

// Envia mensagem de error
void sendError(int dest, int orig, int *seq, int tipo, int error, int soquete);

// Imprime mensagem de erro
void printError(kermitHuman *package);

// Incrementa sequencia
void incrementaSeq(int *seq);

#endif