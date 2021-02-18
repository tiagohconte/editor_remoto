/*  
  Feito por:
    Tiago Henrique Conte

  Redes de Computadores 1 - BCC UFPR
  Trabalho Prático - Editor de Texto Remoto

  Editor Remoto - Client Side
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "rawSocket.h"
#include "kermitProtocol.h"
#include "libClient.h"

int main()
{
  char *device = "lo";
  int soquete;

  soquete = conexaoRawSocket(device);

  char cwd[100];
  char comando[8] = "";

  kermitHuman package;
  int seq = 0;

  while(1) 
  {
    resetPackage(&package);

    printf("%s > ", getcwd(cwd, 100));    
    scanf("%s", comando);

    //  Verifica o comando dado pelo usuário
    if(strncmp(comando, "cd", 2) == 0)
    {
      comando_cd(&seq, soquete);

    } else if (strncmp(comando, "lcd", 3) == 0)
    {
      comando_lcd();      

    } else if (strncmp(comando, "ls", 2) == 0)
    {
      comando_ls(&seq, soquete);

    } else if (strncmp(comando, "lls", 3) == 0)
    {
      comando_lls();

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
      printf("Finalizando client\n");
      return 1;
    } else {
      printf("%s é um comando inválido!\n", comando);
    }

  }

  return 0;
}