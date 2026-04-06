#ifndef HASH_EXTENSIVEL_H
#define HASH_EXTENSIVEL_H

#include <stdbool.h>

/*note que: um hash extensivel é um algoritmo de indexacao dinamica utilizado para organizar dados em arquivos de forma que a busca, insercao e remocao sejam extremamente
rapidas, geralmente exigindo apenas um ou dois acessos ao disco. Diferente do hash estatico, onde o tamanho da tabela é fixo, o hash extensivel cresce e diminui conforme
a necessidade, evitando reorganizacoes completas do arquivo.
*/

// struct do hash privada (opaca)
typedef struct HashExtensivel HashExtensivel;

/* 
 * hash_extensivel_abrir: abre o hash se os arquivos ja existirem, ou cria do zero se for a primeira vez.
 * entrada: 
 *   caminho_dados: nome/caminho do arquivo de buckets.
 *   caminho_dir: nome/caminho do arquivo do indice (diretorio).
 * saida: retorna o ponteiro pro hash aberto ou NULL se der erro de memoria/abertura.
 */
HashExtensivel* hash_extensivel_abrir(const char* caminho_dados, const char* caminho_dir);

/* 
 * hash_extensivel_inserir: coloca uma nova chave e um valor no hash (cuida da divisao de buckets se encher).
 * entrada: 
 *   hash: ponteiro da estrutura.
 *   chave: numero inteiro identificador.
 *   valor: string com os dados (ate 49 caracteres + \0).
 * saida: true se salvou ok, false se deu algum problema.
 */
bool hash_extensivel_inserir(HashExtensivel* hash, int chave, const char* valor);

/* 
 * hash_extensivel_buscar: tenta achar um registro pela chave.
 * entrada: 
 *   hash: ponteiro da estrutura.
 *   chave: o que esta sendo procurado.
 *   valor_out: onde vai copiar a string encontrada (precisa ter espaço de 50 bytes).
 * saida: true se achou, false se nao existir no arquivo.
 */
bool hash_extensivel_buscar(HashExtensivel* hash, int chave, char* valor_out);

/* 
 * hash_extensivel_remover: apaga um registro do arquivo via chave. 
 * entrada: 
 *   hash: ponteiro da estrutura.
 *   chave: identificador do que remover.
 * saida: true se removeu, false se não achou pra tirar.
 */
bool hash_extensivel_remover(HashExtensivel* hash, int chave);

/* 
 * hash_extensivel_fechar: fecha os arquivos, salva o diretorio atual e da free no que foi alocado.
 * entrada: 
 *   hash: o ponteiro da estrutura aberta.
 * saida: sem retorno.
 */
void hash_extensivel_fechar(HashExtensivel* hash);

#endif
