/*  
  Feito por:
    Tiago Henrique Conte

  Redes de Computadores 1 - BCC UFPR
  Trabalho Prático - Editor de Texto Remoto

  Biblioteca Editor Remoto - Server
*/

#ifndef __LIBSERVER__
#define __LIBSERVER__

// define a quantidade máxima de caracteres em uma linha em um arquivo de texto
#define TAM_LINHA 100
// define o quanto de memória será alocado por operação de alocação
#define TAM_ALOC 1000

/*Definição do tipo dicionario*/
typedef struct tad_texto {
  unsigned int num_linhas;
  unsigned char **linhas;
} tad_texto;

// Comando cd - server side
// Executa change directory no server
void comando_cd(kermitHuman *package, int *seq, int soquete);

// Comando ls - server side
// Executa ls no server
void comando_ls(int *seq, int soquete);

// Comando ver - server side
// Mostra o conteúdo do arquivo texto do servidor na tela do cliente
void comando_ver(kermitHuman *package, int *seq, int soquete);

// Comando linha - server side
// Mostra a linha <numero_linha> do arquivo <nome_arq> que esta no servidor na tela do cliente.
void comando_linha(kermitHuman *package, int *seq, int soquete);

// Comando linhas - server side
// Mostra as linhas entre a <numero_linha_inicial> e <numero_linha_final> do arquivo <nome_arq>, que está no servidor, na tela do cliente.
void comando_linhas(kermitHuman *package, int *seq, int soquete);

// Comando edit - server side
// troca a linha <numero_linha> do arquivo <nome_arq>, que está no servidor, pelo texto <NOVO_TEXTO> que deve ser digitado entre aspas.
void comando_edit(kermitHuman *package, int *seq, int soquete);

// Aloca um arquivo de texto em um buffer
int aloca_arq(FILE *arquivo, tad_texto *buffer);

#endif