#include "unity.h"
#include "libs.h"
#include "quadra.h"

void setUp(void) {}
void tearDown(void) {}

/* --- createQuadra / getters --- */

void test_create_and_get(void) {
    Quadra q = createQuadra("b01.1", 100.0, 200.0, 120.0, 80.0,
                             "1.0px", "gold", "black");
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_STRING("b01.1", getQuadraCep(q));
    TEST_ASSERT_EQUAL_FLOAT(100.0, getQuadraX(q));
    TEST_ASSERT_EQUAL_FLOAT(200.0, getQuadraY(q));
    TEST_ASSERT_EQUAL_FLOAT(120.0, getQuadraW(q));
    TEST_ASSERT_EQUAL_FLOAT(80.0,  getQuadraH(q));
    TEST_ASSERT_EQUAL_STRING("1.0px", getQuadraSw(q));
    TEST_ASSERT_EQUAL_STRING("gold",  getQuadraCfill(q));
    TEST_ASSERT_EQUAL_STRING("black", getQuadraCstrk(q));
    freeQuadra(q);
}

void test_defaults_when_null_strings(void) {
    Quadra q = createQuadra(NULL, 0.0, 0.0, 50.0, 50.0, NULL, NULL, NULL);
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_STRING("",      getQuadraCep(q));
    TEST_ASSERT_EQUAL_STRING("1.0px", getQuadraSw(q));
    TEST_ASSERT_EQUAL_STRING("red",   getQuadraCfill(q));
    TEST_ASSERT_EQUAL_STRING("black", getQuadraCstrk(q));
    freeQuadra(q);
}

/* --- calcularEnderecoQuadra --- */

void test_face_N(void) {
    Quadra q = createQuadra("t", 10.0, 20.0, 100.0, 60.0, NULL, NULL, NULL);
    double x, y;
    calcularEnderecoQuadra(q, 'N', 30.0, &x, &y);
    TEST_ASSERT_EQUAL_FLOAT(10.0 + 30.0, x); /* x + num */
    TEST_ASSERT_EQUAL_FLOAT(20.0 + 60.0, y); /* y + h   */
    freeQuadra(q);
}

void test_face_S(void) {
    /* Face S nos .qry aponta para face leste (x+w, y+num) – compatibilidade com gabarito */
    Quadra q = createQuadra("t", 10.0, 20.0, 100.0, 60.0, NULL, NULL, NULL);
    double x, y;
    calcularEnderecoQuadra(q, 'S', 30.0, &x, &y);
    TEST_ASSERT_EQUAL_FLOAT(10.0 + 100.0, x); /* x + w   */
    TEST_ASSERT_EQUAL_FLOAT(20.0 + 30.0,  y); /* y + num */
    freeQuadra(q);
}

void test_face_L(void) {
    Quadra q = createQuadra("t", 10.0, 20.0, 100.0, 60.0, NULL, NULL, NULL);
    double x, y;
    calcularEnderecoQuadra(q, 'L', 15.0, &x, &y);
    TEST_ASSERT_EQUAL_FLOAT(10.0 + 100.0, x); /* x + w   */
    TEST_ASSERT_EQUAL_FLOAT(20.0 + 15.0,  y); /* y + num */
    freeQuadra(q);
}

void test_face_O(void) {
    Quadra q = createQuadra("t", 10.0, 20.0, 100.0, 60.0, NULL, NULL, NULL);
    double x, y;
    calcularEnderecoQuadra(q, 'O', 15.0, &x, &y);
    TEST_ASSERT_EQUAL_FLOAT(10.0,        x); /* x       */
    TEST_ASSERT_EQUAL_FLOAT(20.0 + 15.0, y); /* y + num */
    freeQuadra(q);
}

void test_face_W_same_as_L(void) {
    /* Face W nos .qry aponta para face Leste (x+w, y+num) */
    Quadra q = createQuadra("t", 10.0, 20.0, 100.0, 60.0, NULL, NULL, NULL);
    double xL, yL, xW, yW;
    calcularEnderecoQuadra(q, 'L', 40.0, &xL, &yL);
    calcularEnderecoQuadra(q, 'W', 40.0, &xW, &yW);
    TEST_ASSERT_EQUAL_FLOAT(xL, xW);
    TEST_ASSERT_EQUAL_FLOAT(yL, yW);
    freeQuadra(q);
}

void test_getQuadraSize(void) {
    TEST_ASSERT_GREATER_THAN(0, (int)getQuadraSize());
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_and_get);
    RUN_TEST(test_defaults_when_null_strings);
    RUN_TEST(test_face_N);
    RUN_TEST(test_face_S);
    RUN_TEST(test_face_L);
    RUN_TEST(test_face_O);
    RUN_TEST(test_face_W_same_as_L);
    RUN_TEST(test_getQuadraSize);
    return UNITY_END();
}
