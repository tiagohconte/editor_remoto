/*  
  Feito por:
    Tiago Henrique Conte

  Redes de Computadores 1 - BCC UFPR
  Trabalho Prático - Editor de Texto Remoto

  Biblioteca Editor Remoto - Client
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "kermitProtocol.h"
#include "libClient.h"

//  Comando ls do client side
/*void comando_ls(kermit *package)
{  
  package->inicio = "01111110";
  package->end_dest = "10";
  package->end_orig = "01";
  package->tam = "0000";
  package->seq = "0000";
  package->tipo = "0001";
  package->dados = "";
  package->par = "00000001";
}*/