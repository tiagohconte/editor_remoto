/*  
  Feito por:
    Tiago Henrique Conte

  Redes de Computadores 1 - BCC UFPR
  Trabalho Prático - Editor de Texto Remoto

  Biblioteca Editor Remoto - Client
*/

#ifndef __LIBCLIENT__
#define __LIBCLIENT__

/* COMANDOS LOCAIS */

// Comando change directory local
void comando_lcd();
// Comando ls local
void comando_lls();

/* COMANDOS EXTERNOS */

// Comando cd - client side
// Executa change directory no server
void comando_cd();
// Comando ls - client side
// Executa ls no server
void comando_ls(int *seq, int soquete);

#endif