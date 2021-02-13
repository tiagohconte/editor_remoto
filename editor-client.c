/*  
  Feito por:
    Tiago Henrique Conte

  Redes de Computadores 1 - BCC UFPR
  Trabalho Prático - Editor de Texto Remoto

  Editor Remoto - Client Side
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "rawSocket.h"
#include "kermitProtocol.h"
#include "libClient.h"

int main()
{
  char *device = "lo";
  int soquete;

  soquete = conexaoRawSocket(device);
  //printf("%d\n", soquete);

  char *buffer = (char*) malloc(TAM_PACKAGE);
  memset(buffer, 0, TAM_PACKAGE);

  char comando[10] = "";

  while(1) 
  {
    
    /*scanf("%s", comando);

    //  Verifica o comando dado pelo usuário
    if(strncmp(comando, "cd ", 2) == 0)
    {
      printf("cd\n");

    } else if (strncmp(comando, "lcd ", 3) == 0)
    {
      printf("lcd\n");

    } else if (strncmp(comando, "ls", 2) == 0)
    {
      comando_ls(&package);

    } else if (strncmp(comando, "lls", 3) == 0)
    {
      printf("lls\n");

    } else if (strncmp(comando, "ver", 3) == 0)
    {
      printf("ver\n");

    } else if (strncmp(comando, "linha", 5) == 0)
    {
      printf("linha\n");

    } else if (strncmp(comando, "linhas", 6) == 0)
    {
      printf("linhas\n");

    } else if (strncmp(comando, "edit", 4) == 0)
    {
      printf("edit\n");

    } else {
      printf("Comando inválido!\n");
    }*/

    // printf("Escreva uma mensagem: ");
    // scanf("%s", buffer); 


    /* TESTES */   

    struct kermitBit package;
    package.header[0] = '~';
    package.header[1] = 148;
    package.header[2] = 242;
    strncpy(package.data, "hola", 5);
    package.data[4] = 8;
    
    //  Coleta header
    strncpy(buffer, (char *) package.header, 3);
    strncpy(buffer+3, (char *) package.data, 5);

    /* FIM DOS TESTES */

    if( write(soquete, buffer, TAM_PACKAGE) == -1 )
    {
      fprintf(stderr, "Erro no envio: %s\n", strerror(errno));
      exit(-1);
    }

    // #if DEBUG
    // printf("Mensagem enviada com sucesso!\n");
    // #endif

    sleep(10);
  }

  return 0;
}