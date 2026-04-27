#include "libs.h"
#include "hashfile.h"
#include "unity.h"

typedef struct {
    int id;
    double value;
} DummyData;

void setUp(void) {
    remove("test_hash.hf");
    remove("test_hash.hfd");
}

void tearDown(void) {
    remove("test_hash.hf");
    remove("test_hash.hfd");
}

void test_Criar_E_Abrir_Hashfile(void) {
    Hashfile hf = createHashfile("test_hash.hf", sizeof(DummyData), 3);
    TEST_ASSERT_NOT_NULL(hf);
    closeHashfile(hf);

    Hashfile hf2 = openHashfile("test_hash.hf");
    TEST_ASSERT_NOT_NULL(hf2);
    closeHashfile(hf2);
}

void test_Inserir_E_Buscar_Basico(void) {
    Hashfile hf = createHashfile("test_hash.hf", sizeof(DummyData), 3);
    
    DummyData d1 = {10, 3.14};
    insertHashfile(hf, "CHAVE_1", &d1);

    DummyData* rd1 = (DummyData*)getHashfile(hf, "CHAVE_1");
    TEST_ASSERT_NOT_NULL(rd1);
    TEST_ASSERT_EQUAL_INT(10, rd1->id);
    
    free(rd1);
    closeHashfile(hf);
}

void test_Inserir_Ate_Causar_Overflow(void) {
    Hashfile hf = createHashfile("test_hash.hf", sizeof(DummyData), 2);
    for (int i = 0; i < 300; i++) {
        char key[20];
        snprintf(key, 20, "KEY_%d", i);
        DummyData d = {i, (double)i * 2.5};
        insertHashfile(hf, key, &d);
    }

    DummyData* rd = (DummyData*)getHashfile(hf, "KEY_150");
    TEST_ASSERT_NOT_NULL(rd);
    TEST_ASSERT_EQUAL_INT(150, rd->id);
    free(rd);

    dumpHashfileTxt(hf, "test_hash.hfd");

    closeHashfile(hf);
    FILE* f = fopen("test_hash.hfd", "r");
    TEST_ASSERT_NOT_NULL(f);
    fclose(f);
}

void test_Remocao_Logica(void) {
    Hashfile hf = createHashfile("test_hash.hf", sizeof(DummyData), 3);
    
    DummyData d1 = {42, 99.9};
    insertHashfile(hf, "CHAVE_REMOVER", &d1);
    
    DummyData* r1 = (DummyData*)getHashfile(hf, "CHAVE_REMOVER");
    TEST_ASSERT_NOT_NULL(r1);
    free(r1);
    bool success = removeHashfile(hf, "CHAVE_REMOVER");
    TEST_ASSERT_TRUE(success);

    DummyData* r2 = (DummyData*)getHashfile(hf, "CHAVE_REMOVER");
    TEST_ASSERT_NULL(r2);

    closeHashfile(hf);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_Criar_E_Abrir_Hashfile);
    RUN_TEST(test_Inserir_E_Buscar_Basico);
    RUN_TEST(test_Inserir_Ate_Causar_Overflow);
    RUN_TEST(test_Remocao_Logica);
    return UNITY_END();
}
