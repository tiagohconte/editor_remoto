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
#include <string.h>
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

  int seq = 0;

  while(1) 
  {

    printf("client:%s > ", getcwd(cwd, 100));    
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
      comando_ver(&seq, soquete);

    } else if (strncmp(comando, "linhas", 6) == 0)
    {
      comando_linhas(&seq, soquete);

    } else if (strncmp(comando, "linha", 5) == 0)
    {
      comando_linha(&seq, soquete);

    } else if (strncmp(comando, "edit", 4) == 0)
    {
      comando_edit(&seq, soquete);

    } else if (strncmp(comando, "exit", 4) == 0)
    {
      printf("Finalizando client e server\n");
      comando_exit(&seq, soquete);
      return 1;
    } else {
      printf("%s é um comando inválido!\n", comando);
    }

  }

  return 0;
}