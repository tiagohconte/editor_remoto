# Editor Remoto
Editor Remoto desenvolvido como trabalho final da disciplina Redes de Computadores 1 do BCC na UFPR.

## Instruções
1. Clone o repositório e execute o comando `make`
2. Abra dois terminais em modo root e, em ambos, vá até o diretório em que o repositório foi clonado.
3. Em um dos terminais, execute `./editor-server`
4. No outro terminal, execute `./editor-client`
5. O programa está pronto para ser utilizado!

## Comandos disponíveis
Os comandos devem ser executados no _cliente_  
- `cd <nome_dir>`: efetua a troca de diretório no servidor.
- `lcd <nome_dir>`: efetua a troca de diretório no cliente.
- `ls`: lista os arquivos do diretório corrente do servidor.
- `lls`: lista os arquivos do diretório corrente do cliente.
- `exit`: finaliza o cliente.

## Comandos à adicionar
- `ver <nome_arq>` - mostra o conteúdo do arquivo texto do servidor na tela do cliente.
- `linha <numero_linha> <nome_arq>` - mostra a linha _numero_linha_ do arquivo
_nome_arq_, que está no servidor, na tela do cliente.
- `linhas <numero_linha_inicial> <numero_linha_final> <nome_arq>` - mostra as linhas
entre a _numero_linha_inicial_ e _numero_linha_final_ do arquivo _nome_arq_, que
está no servidor, na tela do cliente.
- `edit <numero_linha> <nome_arq> “<NOVO_TEXTO>”` – troca a linha _numero_linha_ do
arquivo _nome_arq_, que está no servidor, pelo texto _NOVO_TEXTO_ que deve ser
digitado entre aspas.