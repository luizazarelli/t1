#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "hash_extensivel.h"

HashExtensivel* meu_hash = NULL;

void setUp(void) {
    remove("dados.bin");
    remove("diretorio.bin");
    meu_hash = hash_extensivel_abrir("dados.bin", "diretorio.bin");
}

void tearDown(void) {
    if (meu_hash) {
        hash_extensivel_fechar(meu_hash);
        meu_hash = NULL;
    }
}

void testa_insercao_simples(void) {
    TEST_ASSERT_NOT_NULL(meu_hash);
    TEST_ASSERT_TRUE(hash_extensivel_inserir(meu_hash, 1, "Joao"));
    TEST_ASSERT_TRUE(hash_extensivel_inserir(meu_hash, 2, "Maria"));
    TEST_ASSERT_TRUE(hash_extensivel_inserir(meu_hash, 3, "Jose"));
}

void testa_busca_registros(void) {
    char saida[50];
    
    hash_extensivel_inserir(meu_hash, 10, "Produto X");
    TEST_ASSERT_TRUE(hash_extensivel_buscar(meu_hash, 10, saida));
    TEST_ASSERT_EQUAL_STRING("Produto X", saida);

    TEST_ASSERT_FALSE(hash_extensivel_buscar(meu_hash, 999, saida));
}

void testa_remocao_registro(void) {
    char saida[50];
    
    hash_extensivel_inserir(meu_hash, 5, "Remover Este");

    TEST_ASSERT_TRUE(hash_extensivel_remover(meu_hash, 5));
    TEST_ASSERT_FALSE(hash_extensivel_buscar(meu_hash, 5, saida));
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(testa_insercao_simples);
    RUN_TEST(testa_busca_registros);
    RUN_TEST(testa_remocao_registro);
    
    return UNITY_END();
}
