#include "unity.h"
#include "hash_extensivel.h"
#include <stdio.h>

#define ARQ_DADOS "/tmp/t_he_dados.bin"
#define ARQ_DIR   "/tmp/t_he_dir.bin"

void setUp(void) {
    remove(ARQ_DADOS);
    remove(ARQ_DIR);
}

void tearDown(void) {
    remove(ARQ_DADOS);
    remove(ARQ_DIR);
}

void test_abrir_cria_novo(void) {
    HashExtensivel* h = hash_extensivel_abrir(ARQ_DADOS, ARQ_DIR);
    TEST_ASSERT_NOT_NULL(h);
    hash_extensivel_fechar(h);
}

void test_abrir_reabre_existente(void) {
    HashExtensivel* h = hash_extensivel_abrir(ARQ_DADOS, ARQ_DIR);
    hash_extensivel_inserir(h, 7, "persistido");
    hash_extensivel_fechar(h);

    h = hash_extensivel_abrir(ARQ_DADOS, ARQ_DIR);
    TEST_ASSERT_NOT_NULL(h);
    char val[50];
    TEST_ASSERT_TRUE(hash_extensivel_buscar(h, 7, val));
    TEST_ASSERT_EQUAL_STRING("persistido", val);
    hash_extensivel_fechar(h);
}

void test_inserir_e_buscar(void) {
    HashExtensivel* h = hash_extensivel_abrir(ARQ_DADOS, ARQ_DIR);
    TEST_ASSERT_TRUE(hash_extensivel_inserir(h, 1, "valor_um"));
    char val[50];
    TEST_ASSERT_TRUE(hash_extensivel_buscar(h, 1, val));
    TEST_ASSERT_EQUAL_STRING("valor_um", val);
    hash_extensivel_fechar(h);
}

void test_inserir_multiplos_distintos(void) {
    HashExtensivel* h = hash_extensivel_abrir(ARQ_DADOS, ARQ_DIR);
    hash_extensivel_inserir(h, 10, "dez");
    hash_extensivel_inserir(h, 20, "vinte");
    hash_extensivel_inserir(h, 30, "trinta");
    char val[50];
    TEST_ASSERT_TRUE(hash_extensivel_buscar(h, 10, val));
    TEST_ASSERT_EQUAL_STRING("dez",    val);
    TEST_ASSERT_TRUE(hash_extensivel_buscar(h, 20, val));
    TEST_ASSERT_EQUAL_STRING("vinte",  val);
    TEST_ASSERT_TRUE(hash_extensivel_buscar(h, 30, val));
    TEST_ASSERT_EQUAL_STRING("trinta", val);
    hash_extensivel_fechar(h);
}

void test_buscar_inexistente(void) {
    HashExtensivel* h = hash_extensivel_abrir(ARQ_DADOS, ARQ_DIR);
    char val[50];
    TEST_ASSERT_FALSE(hash_extensivel_buscar(h, 999, val));
    hash_extensivel_fechar(h);
}

void test_remover_existente(void) {
    HashExtensivel* h = hash_extensivel_abrir(ARQ_DADOS, ARQ_DIR);
    hash_extensivel_inserir(h, 5, "cinco");
    TEST_ASSERT_TRUE(hash_extensivel_remover(h, 5));
    char val[50];
    TEST_ASSERT_FALSE(hash_extensivel_buscar(h, 5, val));
    hash_extensivel_fechar(h);
}

void test_remover_inexistente(void) {
    HashExtensivel* h = hash_extensivel_abrir(ARQ_DADOS, ARQ_DIR);
    TEST_ASSERT_FALSE(hash_extensivel_remover(h, 888));
    hash_extensivel_fechar(h);
}

void test_split_apos_bucket_cheio(void) {
    HashExtensivel* h = hash_extensivel_abrir(ARQ_DADOS, ARQ_DIR);
    TEST_ASSERT_TRUE(hash_extensivel_inserir(h, 0, "zero"));
    TEST_ASSERT_TRUE(hash_extensivel_inserir(h, 1, "um"));
    TEST_ASSERT_TRUE(hash_extensivel_inserir(h, 2, "dois"));
    TEST_ASSERT_TRUE(hash_extensivel_inserir(h, 3, "tres"));
    char val[50];
    TEST_ASSERT_TRUE(hash_extensivel_buscar(h, 0, val)); TEST_ASSERT_EQUAL_STRING("zero", val);
    TEST_ASSERT_TRUE(hash_extensivel_buscar(h, 1, val)); TEST_ASSERT_EQUAL_STRING("um",   val);
    TEST_ASSERT_TRUE(hash_extensivel_buscar(h, 2, val)); TEST_ASSERT_EQUAL_STRING("dois", val);
    TEST_ASSERT_TRUE(hash_extensivel_buscar(h, 3, val)); TEST_ASSERT_EQUAL_STRING("tres", val);
    hash_extensivel_fechar(h);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_abrir_cria_novo);
    RUN_TEST(test_abrir_reabre_existente);
    RUN_TEST(test_inserir_e_buscar);
    RUN_TEST(test_inserir_multiplos_distintos);
    RUN_TEST(test_buscar_inexistente);
    RUN_TEST(test_remover_existente);
    RUN_TEST(test_remover_inexistente);
    RUN_TEST(test_split_apos_bucket_cheio);
    return UNITY_END();
}
