/*  
  Feito por:
    Tiago Henrique Conte

  Redes de Computadores 1 - BCC UFPR
  Trabalho Prático - Editor de Texto Remoto

  Biblioteca Protocolo Kermit
*/

#include "kermitProtocol.h"

void resetPackage(kermitHuman *package)
{
  package->dest = -1;
  package->orig = -1;
  package->tam = -1;
  package->seq = -1;
  package->tipo = -1;
  package->par = -1;
}

int verificaBits(unsigned char c, unsigned int num)
{
  return ( (c^num) == 0 );
}
