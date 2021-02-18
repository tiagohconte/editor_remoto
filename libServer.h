/*  
  Feito por:
    Tiago Henrique Conte

  Redes de Computadores 1 - BCC UFPR
  Trabalho Prático - Editor de Texto Remoto

  Biblioteca Editor Remoto - Server
*/

#ifndef __LIBSERVER__
#define __LIBSERVER__

// Comando cd - server side
// Executa change directory no server
void comando_cd(kermitHuman *package, int *seq, int soquete);

// Comando ls - server side
// Executa ls no server
void comando_ls(int *seq, int soquete);

#endif