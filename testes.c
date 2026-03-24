#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hash_extensivel.h"

int main() {
    remove("teste_dados.bin");
    remove("teste_dir.bin");

    HashExtensivel* hash = hash_extensivel_abrir("teste_dados.bin", "teste_dir.bin");
    assert(hash != NULL);

    assert(hash_extensivel_inserir(hash, 10, "Registro A"));
    assert(hash_extensivel_inserir(hash, 20, "Registro B"));
    assert(hash_extensivel_inserir(hash, 30, "Registro C"));
    assert(hash_extensivel_inserir(hash, 40, "Registro D")); 
    
    char buffer[50];
    
    assert(hash_extensivel_buscar(hash, 20, buffer) == true);
    assert(strcmp(buffer, "Registro B") == 0);

    assert(hash_extensivel_buscar(hash, 99, buffer) == false);

    assert(hash_extensivel_remover(hash, 20) == true);
    assert(hash_extensivel_buscar(hash, 20, buffer) == false);
    assert(hash_extensivel_remover(hash, 99) == false);

    hash_extensivel_fechar(hash);
    printf("Todos os testes passaram com sucesso.\n");

    return 0;
}
