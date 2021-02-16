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

  char *buffer = (char*) malloc(TAM_PACKAGE);
  memset(buffer, 0, TAM_PACKAGE);

  char comando[10] = "";

  kermitHuman package;
  int seq = 0;

  while(1) 
  {
    resetPackage(&package);
    
    scanf("%s", comando);

    //  Verifica o comando dado pelo usuário
    if(strncmp(comando, "cd ", 2) == 0)
    {
      printf("cd\n");

    } else if (strncmp(comando, "lcd ", 3) == 0)
    {
      printf("lcd\n");

    } else if (strncmp(comando, "ls", 2) == 0)
    {
      comando_ls(buffer, &seq, soquete);
      
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

    } else if (strncmp(comando, "exit", 4) == 0)
    {
      printf("Finalizando client.\n");
      return 1;
    } else {
      printf("Comando inválido!\n");
    }

    // writePackageBit(&packageBit, &package);

    /* TESTES */  
    
    /*packageBit.header[0] = '~';
    packageBit.header[1] = 148;
    packageBit.header[2] = 242;
    strncpy(packageBit.data, "hola", 5);
    packageBit.data[4] = 8;*/
    
    //  Bufferiza header e dados
    // strncpy(buffer, (char *) packageBit.header, 3);
    // strncpy(buffer+3, (char *) packageBit.data, package.tam+1);

    /* FIM DOS TESTES */

    /*if( write(soquete, buffer, TAM_PACKAGE) == -1 )
    {
      fprintf(stderr, "Erro no envio: %s\n", strerror(errno));
      exit(-1);
    }*/

    // #if DEBUG
    // printf("Mensagem enviada com sucesso!\n");
    // #endif

    // sleep(10);
  }

  return 0;
}