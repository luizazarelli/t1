#ifndef HASH_EXTENSIVEL_H
#define HASH_EXTENSIVEL_H

#include <stdbool.h>

typedef struct HashExtensivel HashExtensivel;

HashExtensivel* hash_extensivel_abrir(const char* caminho_dados, const char* caminho_dir);

bool hash_extensivel_inserir(HashExtensivel* hash, int chave, const char* valor);
bool hash_extensivel_buscar(HashExtensivel* hash, int chave, char* valor_out);
bool hash_extensivel_remover(HashExtensivel* hash, int chave);

void hash_extensivel_fechar(HashExtensivel* hash);

#endif
