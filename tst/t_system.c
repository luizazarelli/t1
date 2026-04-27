#define _POSIX_C_SOURCE 200809L
#include "unity.h"
#include "system.h"
#include "paths.h"
#include "hashfile.h"
#include "quadra.h"
#include "habitante.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void setUp(void)    {}
void tearDown(void) {}

void test_readParam_caminhos_basicos(void) {
    char* argv[] = { "ted", "-e", "/tmp/in/", "-f", "c1.geo", "-o", "/tmp/out/" };
    int argc = 7;
    void* paths = createAllPaths();
    readParam(argc, argv, paths);
    TEST_ASSERT_EQUAL_STRING("c1.geo", getGeoArq(paths));
    TEST_ASSERT_EQUAL_STRING("c1",     getGeoName(paths));
    TEST_ASSERT_NOT_NULL(getBsd(paths));
    TEST_ASSERT_NOT_NULL(getBedGeo(paths));
    freeAllPaths(paths);
}

void test_readParam_sem_pm_sem_qry(void) {
    char* argv[] = { "ted", "-f", "mapa.geo", "-o", "/tmp/" };
    int argc = 5;
    void* paths = createAllPaths();
    readParam(argc, argv, paths);
    TEST_ASSERT_NULL(getPmArq(paths));
    TEST_ASSERT_NULL(getQryArq(paths));
    freeAllPaths(paths);
}

void test_readParam_com_pm_e_qry(void) {
    char* argv[] = { "ted", "-f", "c2.geo", "-o", "/tmp/",
                     "-pm", "povo.pm", "-q", "sub/q1.qry" };
    int argc = 9;
    void* paths = createAllPaths();
    readParam(argc, argv, paths);
    TEST_ASSERT_EQUAL_STRING("povo.pm", getPmArq(paths));
    TEST_ASSERT_EQUAL_STRING("povo",    getPmName(paths));
    TEST_ASSERT_EQUAL_STRING("q1",      getQryName(paths));
    freeAllPaths(paths);
}

void test_readPrintGeo_insere_quadra(void) {
    FILE* f = fopen("/tmp/t_sys.geo", "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "q b01 10.0 20.0 100.0 80.0\n");
    fclose(f);

    void* paths = createAllPaths();
    setBedGeo(paths, strdup("/tmp/t_sys.geo"));
    setBsdGeoSvg(paths, strdup("/tmp/t_sys.svg"));

    Hashfile hq = createHashfile("/tmp/t_sys_geo.hf", getQuadraSize(), 4);
    readPrintGeo(paths, hq);

    void* q = getHashfile(hq, "b01");
    TEST_ASSERT_NOT_NULL(q);
    free(q);

    closeHashfile(hq);
    freeAllPaths(paths);
}

void test_readPrintGeo_geo_inexistente_nao_trava(void) {
    void* paths = createAllPaths();
    setBedGeo(paths, strdup("/tmp/nao_existe.geo"));
    setBsdGeoSvg(paths, strdup("/tmp/nao_existe.svg"));

    Hashfile hq = createHashfile("/tmp/t_sys_geo2.hf", getQuadraSize(), 4);
    readPrintGeo(paths, hq);

    void* q = getHashfile(hq, "qualquer");
    TEST_ASSERT_NULL(q);

    closeHashfile(hq);
    freeAllPaths(paths);
}

void test_readPm_insere_habitante(void) {
    FILE* f = fopen("/tmp/t_sys.pm", "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "p 111.111.111-11 Joao Silva M 01/01/1990\n");
    fclose(f);

    void* paths = createAllPaths();
    setBedPm(paths, strdup("/tmp/t_sys.pm"));

    Hashfile hh = createHashfile("/tmp/t_sys_hab.hf", getHabitanteSize(), 4);
    readPm(paths, hh);

    void* h = getHashfile(hh, "111.111.111-11");
    TEST_ASSERT_NOT_NULL(h);
    free(h);

    closeHashfile(hh);
    freeAllPaths(paths);
}

void test_readPm_moradia_torna_morador(void) {
    FILE* f = fopen("/tmp/t_sys2.pm", "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "p 222.222.222-22 Maria Souza F 05/05/1985\n");
    fprintf(f, "m 222.222.222-22 b01 N 50.0 apto3\n");
    fclose(f);

    void* paths = createAllPaths();
    setBedPm(paths, strdup("/tmp/t_sys2.pm"));

    Hashfile hh = createHashfile("/tmp/t_sys_hab2.hf", getHabitanteSize(), 4);
    readPm(paths, hh);

    Habitante h = (Habitante)getHashfile(hh, "222.222.222-22");
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_TRUE(isMorador(h));
    TEST_ASSERT_EQUAL_STRING("b01", getHabitanteEndCep(h));
    free(h);

    closeHashfile(hh);
    freeAllPaths(paths);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_readParam_caminhos_basicos);
    RUN_TEST(test_readParam_sem_pm_sem_qry);
    RUN_TEST(test_readParam_com_pm_e_qry);
    RUN_TEST(test_readPrintGeo_insere_quadra);
    RUN_TEST(test_readPrintGeo_geo_inexistente_nao_trava);
    RUN_TEST(test_readPm_insere_habitante);
    RUN_TEST(test_readPm_moradia_torna_morador);
    return UNITY_END();
}
