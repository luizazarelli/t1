#include "libs.h"
#include "paths.h"
#include "system.h"
#include "hashfile.h"
#include "quadra.h"
#include "habitante.h"

int main(int argc, char** argv) {
    // Inicializa a estrutura de caminhos e le do argv
    void* paths = createAllPaths();
    readParam(argc, argv, paths);

    // Monta caminhos baseados nos arquivos do disk para persistencia
    char quadrasHf[MAX_PATH_LEN];
    snprintf(quadrasHf, MAX_PATH_LEN, "%s%s.hf", getBsd(paths), getGeoName(paths));
    
    char habitantesHf[MAX_PATH_LEN];
    snprintf(habitantesHf, MAX_PATH_LEN, "%shabitantes.hf", getBsd(paths));

    // Inicializa os Hashfiles binários
    // Supondo ~10.000 quadras, buckets de 10 quadras, e capacity 1009
    size_t Q_SIZE = getQuadraSize();
    size_t H_SIZE = getHabitanteSize();

    void* hash_quadras = createHashfile(quadrasHf, Q_SIZE, 10);
    void* hash_hab     = createHashfile(habitantesHf, H_SIZE, 10);

    // 1. Processa .geo (E cria SVG)
    readPrintGeo(paths, hash_quadras);

    // 2. Processa .pm
    if (getPmArq(paths)) {
        readPm(paths, hash_hab);
    }

    // 3. Processa Consulta
    if (getQryArq(paths)) {
        readQry(paths, hash_quadras, hash_hab);
    }

    // Dumpa HFD para checagens
    char hfdQuadras[MAX_PATH_LEN];
    snprintf(hfdQuadras, MAX_PATH_LEN, "%s%s.hfd", getBsd(paths), getGeoName(paths));
    dumpHashfileTxt(hash_quadras, hfdQuadras);

    char hfdHab[MAX_PATH_LEN];
    snprintf(hfdHab, MAX_PATH_LEN, "%shabitantes.hfd", getBsd(paths));
    dumpHashfileTxt(hash_hab, hfdHab);

    // Limpa memoria
    closeHashfile(hash_quadras);
    closeHashfile(hash_hab);

    freeAllPaths(paths);
    
    printf("Execucao finalizada.\n");
    return 0;
}
