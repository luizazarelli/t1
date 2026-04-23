#include "unity.h"
#include "libs.h"
#include "habitante.h"

void setUp(void) {}
void tearDown(void) {}

/* --- createHabitante / getters básicos --- */

void test_create_and_get(void) {
    Habitante h = createHabitante("000.001.001-01", "Ana", "Silva", 'F', "01/01/1990");
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_EQUAL_STRING("000.001.001-01", getHabitanteCpf(h));
    TEST_ASSERT_EQUAL_STRING("Ana",   getHabitanteNome(h));
    TEST_ASSERT_EQUAL_STRING("Silva", getHabitanteSobrenome(h));
    TEST_ASSERT_EQUAL_CHAR('F', getHabitanteSexo(h));
    TEST_ASSERT_EQUAL_STRING("01/01/1990", getHabitanteNasc(h));
    freeHabitante(h);
}

/* --- isMorador: recém criado é sem-teto --- */

void test_new_habitante_is_sem_teto(void) {
    Habitante h = createHabitante("000.002.002-02", "Carlos", "Souza", 'M', "15/06/1985");
    TEST_ASSERT_FALSE(isMorador(h));
    freeHabitante(h);
}

/* --- setHabitanteEndereco torna morador --- */

void test_set_endereco_makes_morador(void) {
    Habitante h = createHabitante("000.003.003-03", "Lucia", "Pereira", 'F', "20/03/1970");
    setHabitanteEndereco(h, "b01.1", 'N', 30.0, "Apto1");
    TEST_ASSERT_TRUE(isMorador(h));
    TEST_ASSERT_EQUAL_STRING("b01.1", getHabitanteEndCep(h));
    TEST_ASSERT_EQUAL_CHAR('N', getHabitanteEndFace(h));
    TEST_ASSERT_EQUAL_FLOAT(30.0, getHabitanteEndNum(h));
    TEST_ASSERT_EQUAL_STRING("Apto1", getHabitanteEndCompl(h));
    freeHabitante(h);
}

/* --- removerEnderecoHabitante volta a sem-teto --- */

void test_remover_endereco_volta_sem_teto(void) {
    Habitante h = createHabitante("000.004.004-04", "Pedro", "Costa", 'M', "10/10/1995");
    setHabitanteEndereco(h, "b02.3", 'L', 10.0, "Casa");
    TEST_ASSERT_TRUE(isMorador(h));
    removerEnderecoHabitante(h);
    TEST_ASSERT_FALSE(isMorador(h));
    freeHabitante(h);
}

/* --- mudar de endereco --- */

void test_change_endereco(void) {
    Habitante h = createHabitante("000.005.005-05", "Maria", "Lima", 'F', "05/05/2000");
    setHabitanteEndereco(h, "b01.1", 'S', 5.0, "ap1");
    setHabitanteEndereco(h, "b03.2", 'O', 20.0, "ap2");
    TEST_ASSERT_EQUAL_STRING("b03.2", getHabitanteEndCep(h));
    TEST_ASSERT_EQUAL_CHAR('O', getHabitanteEndFace(h));
    TEST_ASSERT_EQUAL_FLOAT(20.0, getHabitanteEndNum(h));
    freeHabitante(h);
}

/* --- getHabitanteSize > 0 --- */

void test_getHabitanteSize(void) {
    TEST_ASSERT_GREATER_THAN(0, (int)getHabitanteSize());
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_and_get);
    RUN_TEST(test_new_habitante_is_sem_teto);
    RUN_TEST(test_set_endereco_makes_morador);
    RUN_TEST(test_remover_endereco_volta_sem_teto);
    RUN_TEST(test_change_endereco);
    RUN_TEST(test_getHabitanteSize);
    return UNITY_END();
}
