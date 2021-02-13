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
#include "kermitProtocol.h"
#include "libServer.h"

//  Comando ls do client side
/*void comando_ls(kermit *package)
{  
  FILE *retorno;
  char str[100];
  retorno = popen("ls", "r");
  if (retorno == NULL){
    fprintf(stderr, "Erro na execução do comando!\n");
  }

  while(!feof(retorno)){
    fgets(str, 100, retorno);
    printf("%s", str);
  }  

}*/