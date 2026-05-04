#include "system.h"
#include "quadra.h"
#include "habitante.h"
#include "hashfile.h"
#include "list.h"
#include <math.h>

/* =========================================================
 * Utilitários internos
 * ========================================================= */

static char extractFace(const char* fStr) {
    if (!fStr) return 'N';
    const char* dot = strchr(fStr, '.');
    return dot ? dot[1] : fStr[0];
}

/* Calcula bounding box do arquivo .geo */
static void computeGeoBounds(const char* geoPath,
                              double* minX, double* minY,
                              double* maxX, double* maxY) {
    *minX = *minY =  1e9;
    *maxX = *maxY = -1e9;
    FILE* geo = fopen(geoPath, "r");
    if (!geo) return;
    char linha[512];
    while (fgets(linha, sizeof(linha), geo)) {
        char cmd[32];
        if (sscanf(linha, "%31s", cmd) != 1) continue;
        if (strcmp(cmd, "q") == 0) {
            char cep[64]; double x, y, w, h;
            sscanf(linha, "%*s %63s %lf %lf %lf %lf", cep, &x, &y, &w, &h);
            if (x     < *minX) *minX = x;
            if (y     < *minY) *minY = y;
            if (x + w > *maxX) *maxX = x + w;
            if (y + h > *maxY) *maxY = y + h;
        }
    }
    fclose(geo);
}

/* Escreve uma quadra no SVG (dentro de svg:g) */
static void svgQuadra(FILE* svg,
                      const char* cep, double x, double y, double w, double h,
                      const char* cfill, const char* cstrk, const char* sw) {
    fprintf(svg,
        "   <svg:rect id=\"%s\" x=\"%lf\" y=\"%lf\" width=\"%lf\" height=\"%lf\" "
        "fill=\"%s\" stroke=\"%s\" fill-opacity=\"0.8\" stroke-width=\"%s\" />\r\n\r\n",
        cep, x, y, w, h, cfill, cstrk, sw);
    fprintf(svg,
        "   <svg:text x=\"%lf\" y=\"%lf\" fill=\"%s\" stroke=\"black\" font-size=\"12\">%s</svg:text>\r\n",
        x + 5.0, y + 9.0, cstrk, cep);
}

/* Escreve uma quadra no SVG base (formato gabarito, sem prefixo svg:) */
static void svgQuadraPlain(FILE* svg,
                            const char* cep, double x, double y, double w, double h,
                            const char* cfill, const char* cstrk, const char* sw) {
    fprintf(svg,
        "   <rect id=\"%s\"  x=\"%lf\" y=\"%lf\" width=\"%lf\" height=\"%lf\" "
        "fill=\"%s\" stroke=\"%s\" fill-opacity=\"0.8\" stroke-width=\"%s\"/>\r\n\r\n",
        cep, x, y, w, h, cfill, cstrk, sw);
    fprintf(svg,
        "   <text  x=\"%lf\"  y=\"%lf\"   fill=\"%s\"  stroke=\"black\"  font-size=\"12\"  >%s</text>\r\n",
        x + 5.0, y + 9.0, cstrk, cep);
}

/* Reproduz as quadras do .geo num arquivo SVG aberto (dentro de svg:g) */
static void replayGeo(FILE* svg, const char* geoPath) {
    FILE* geo = fopen(geoPath, "r");
    if (!geo) return;
    char linha[512];
    char sw[32] = "1.0px", cfill[32] = "red", cstrk[32] = "black";
    while (fgets(linha, sizeof(linha), geo)) {
        char cmd[32];
        if (sscanf(linha, "%31s", cmd) != 1) continue;
        if (strcmp(cmd, "cq") == 0)
            sscanf(linha, "%*s %31s %31s %31s", sw, cfill, cstrk);
        else if (strcmp(cmd, "q") == 0) {
            char cep[64]; double x, y, w, h;
            sscanf(linha, "%*s %63s %lf %lf %lf %lf", cep, &x, &y, &w, &h);
            svgQuadra(svg, cep, x, y, w, h, cfill, cstrk, sw);
        }
    }
    fclose(geo);
}

/* =========================================================
 * Leitura de parâmetros
 * ========================================================= */

void readParam(int argc, char** argv, void* paths) {
    char* e_arg = NULL, *f_arg = NULL, *o_arg = NULL;
    char* q_arg = NULL, *pm_arg = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc)
            e_arg = argv[++i];
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc)
            f_arg = argv[++i];
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
            o_arg = argv[++i];
        else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc)
            q_arg = argv[++i];
        else if (strcmp(argv[i], "-pm") == 0 && i + 1 < argc)
            pm_arg = argv[++i];
    }

    if (!f_arg) { fprintf(stderr, "Erro: -f obrigatorio.\n"); exit(EXIT_FAILURE); }
    if (!o_arg) { fprintf(stderr, "Erro: -o obrigatorio.\n"); exit(EXIT_FAILURE); }

    char* bed = strdup(e_arg ? e_arg : "./");
    assert(bed != NULL);
    bed = prepareDir(bed);
    setBed(paths, bed);

    char* bsd = strdup(o_arg);
    assert(bsd != NULL);
    bsd = prepareDir(bsd);
    setBsd(paths, bsd);

    char* geoarq = strdup(f_arg);
    assert(geoarq != NULL);
    setGeoArq(paths, geoarq);

    char* geoname = calloc(MAX_NAME_LEN, sizeof(char));
    assert(geoname != NULL);
    prepareDoc(f_arg, geoname);
    setGeoName(paths, geoname);

    char* bedgeo = calloc(MAX_PATH_LEN, sizeof(char));
    assert(bedgeo != NULL);
    snprintf(bedgeo, MAX_PATH_LEN, "%s%s", bed, f_arg);
    setBedGeo(paths, bedgeo);

    char* bsdgeosvg = calloc(MAX_PATH_LEN, sizeof(char));
    assert(bsdgeosvg != NULL);
    snprintf(bsdgeosvg, MAX_PATH_LEN, "%s%s.svg", bsd, geoname);
    setBsdGeoSvg(paths, bsdgeosvg);

    if (pm_arg) {
        char* pmarq = strdup(pm_arg);
        assert(pmarq != NULL);
        setPmArq(paths, pmarq);

        char* pmname = calloc(MAX_NAME_LEN, sizeof(char));
        assert(pmname != NULL);
        prepareDoc(pm_arg, pmname);
        setPmName(paths, pmname);

        char* bedpm = calloc(MAX_PATH_LEN, sizeof(char));
        assert(bedpm != NULL);
        snprintf(bedpm, MAX_PATH_LEN, "%s%s", bed, pm_arg);
        setBedPm(paths, bedpm);
    }

    if (q_arg) {
        char* qryarq = strdup(q_arg);
        assert(qryarq != NULL);
        setQryArq(paths, qryarq);

        char* qryname = calloc(MAX_NAME_LEN, sizeof(char));
        assert(qryname != NULL);
        prepareDoc(q_arg, qryname);
        setQryName(paths, qryname);

        char* bedqry = calloc(MAX_PATH_LEN, sizeof(char));
        assert(bedqry != NULL);
        snprintf(bedqry, MAX_PATH_LEN, "%s%s", bed, q_arg);
        setBedQry(paths, bedqry);

        char* bsdgeoqrysvg = calloc(MAX_PATH_LEN, sizeof(char));
        assert(bsdgeoqrysvg != NULL);
        snprintf(bsdgeoqrysvg, MAX_PATH_LEN, "%s%s-%s.svg", bsd, geoname, qryname);
        setBsdGeoQrySvg(paths, bsdgeoqrysvg);

        char* bsdgeoqrytxt = calloc(MAX_PATH_LEN, sizeof(char));
        assert(bsdgeoqrytxt != NULL);
        snprintf(bsdgeoqrytxt, MAX_PATH_LEN, "%s%s-%s.txt", bsd, geoname, qryname);
        setBsdGeoQryTxt(paths, bsdgeoqrytxt);
    }
}

/* =========================================================
 * Leitura e geração do .geo + SVG base
 * ========================================================= */

void readPrintGeo(void* paths, void* hash_quadras) {
    if (!paths || !hash_quadras) return;

    const char* geoPath = getBedGeo(paths);

    /* Passo 1: bounding box */
    double minX, minY, maxX, maxY;
    computeGeoBounds(geoPath, &minX, &minY, &maxX, &maxY);

    /* Passo 2: SVG com cabeçalho no formato gabarito */
    FILE* svg = fopen(getBsdGeoSvg(paths), "w");
    if (svg) {
        if (minX < 150.0)
            fprintf(svg,
                "<svg viewBox=\"0 0 %.1f %.1f\"  "
                "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
                "xmlns=\"http://www.w3.org/2000/svg\" >\r\n",
                maxX + 130.0, maxY + 130.0);
        else
            fprintf(svg,
                "<svg xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
                "xmlns=\"http://www.w3.org/2000/svg\" >\r\n");
        fprintf(svg, "<g id=\"fig\">\r\n");
    }

    /* Passo 3: lê .geo, insere no hashfile e escreve SVG */
    FILE* geo = fopen(geoPath, "r");
    if (!geo) { if (svg) { fprintf(svg, "</g>\r\n</svg>"); fclose(svg); } return; }

    char linha[512];
    char sw[32] = "1.0px", cfill[32] = "red", cstrk[32] = "black";

    while (fgets(linha, sizeof(linha), geo)) {
        char cmd[32];
        if (sscanf(linha, "%31s", cmd) != 1) continue;

        if (strcmp(cmd, "cq") == 0) {
            sscanf(linha, "%*s %31s %31s %31s", sw, cfill, cstrk);
        } else if (strcmp(cmd, "q") == 0) {
            char cep[64]; double x, y, w, h;
            sscanf(linha, "%*s %63s %lf %lf %lf %lf", cep, &x, &y, &w, &h);

            Quadra q = createQuadra(cep, x, y, w, h, sw, cfill, cstrk);
            insertHashfile(hash_quadras, cep, q);
            freeQuadra(q);

            if (svg) svgQuadraPlain(svg, cep, x, y, w, h, cfill, cstrk, sw);
        }
    }
    fclose(geo);

    if (svg) {
        fprintf(svg, "</g>\r\n</svg>");
        fclose(svg);
    }
}

/* =========================================================
 * Leitura do .pm
 * ========================================================= */

void readPm(void* paths, void* hash_hab) {
    if (!paths || !hash_hab) return;

    const char* pmPath = getBedPm(paths);
    if (!pmPath) return;

    FILE* pm = fopen(pmPath, "r");
    if (!pm) { printf("Aviso: .pm nao encontrado: %s\n", pmPath); return; }

    char linha[512];
    while (fgets(linha, sizeof(linha), pm)) {
        char cmd[32];
        if (sscanf(linha, "%31s", cmd) != 1) continue;

        if (strcmp(cmd, "p") == 0) {
            char cpf[64], nome[64], sobrenome[64], nasc[32];
            char sexo;
            sscanf(linha, "%*s %63s %63s %63s %c %31s", cpf, nome, sobrenome, &sexo, nasc);
            Habitante h = createHabitante(cpf, nome, sobrenome, sexo, nasc);
            insertHashfile(hash_hab, cpf, h);
            freeHabitante(h);
        } else if (strcmp(cmd, "m") == 0) {
            char cpf[64], cep[64], face_str[32], compl[64];
            double num;
            sscanf(linha, "%*s %63s %63s %31s %lf %63s", cpf, cep, face_str, &num, compl);
            Habitante h = (Habitante)getHashfile(hash_hab, cpf);
            if (h) {
                setHabitanteEndereco(h, cep, extractFace(face_str), num, compl);
                removeHashfile(hash_hab, cpf);
                insertHashfile(hash_hab, cpf, h);
                freeHabitante(h);
            }
        }
    }
    fclose(pm);
}

/* =========================================================
 * Callbacks para operações com o hashfile de habitants
 * ========================================================= */

/* --- pq: conta moradores por face em um dado CEP --- */
typedef struct {
    char cep[64];
    int cN, cS, cL, cO, total;
} PqStats;

static void pqCountCb(char* key, void* data, void* extra1, void* extra2) {
    (void)key; (void)extra2;
    PqStats* st = (PqStats*)extra1;
    Habitante h = (Habitante)data;
    if (!isMorador(h)) return;
    if (strcmp(getHabitanteEndCep(h), st->cep) != 0) return;
    st->total++;
    char f = getHabitanteEndFace(h);
    if      (f == 'N')             st->cN++;
    else if (f == 'S')             st->cS++;
    else if (f == 'L' || f == 'W') st->cL++;
    else if (f == 'O')             st->cO++;
}

/* rq: coleta CPFs de moradores de um CEP – usa dois extras */
typedef struct { char cep[64]; Lista cpfs; } RqExtra;

static void rqCollectCb2(char* key, void* data, void* extra1, void* extra2) {
    (void)key; (void)extra2;
    RqExtra* ex = (RqExtra*)extra1;
    Habitante h = (Habitante)data;
    if (!isMorador(h)) return;
    if (strcmp(getHabitanteEndCep(h), ex->cep) != 0) return;
    char* cpfcopy = strdup(getHabitanteCpf(h));
    insertAtTail(ex->cpfs, cpfcopy);
}

/* --- censo --- */
typedef struct {
    int total_hab;
    int moradores, sem_teto;
    int hab_h, hab_m;
    int mor_h, mor_m;
    int sem_h, sem_m;
} CensoStats;

static void censoCb(char* key, void* data, void* extra1, void* extra2) {
    (void)key; (void)extra2;
    CensoStats* st = (CensoStats*)extra1;
    Habitante h = (Habitante)data;
    st->total_hab++;
    char s = getHabitanteSexo(h);
    if (s == 'M' || s == 'm') st->hab_h++; else st->hab_m++;
    if (isMorador(h)) {
        st->moradores++;
        if (s == 'M' || s == 'm') st->mor_h++; else st->mor_m++;
    } else {
        st->sem_teto++;
        if (s == 'M' || s == 'm') st->sem_h++; else st->sem_m++;
    }
}

/* =========================================================
 * SVG helpers para anotações do .qry
 * ========================================================= */

/* Escreve um rect de anotação fora do svg:g.
 * Se *ann_count == 0, o elemento se afixa diretamente em </svg:g> (sem indentação).
 * Caso contrário, imprime "   " antes do elemento. Incrementa *ann_count. */
static void svgAnnRect(FILE* svg, double x, double y, double w, double h,
                       const char* attrs, int* ann_count) {
    if ((*ann_count)++ == 0)
        fprintf(svg, "<svg:rect width=\"%lf\" height=\"%lf\" x=\"%lf\" y=\"%lf\" %s />\r\n\r\n",
                w, h, x, y, attrs);
    else
        fprintf(svg, "   <svg:rect width=\"%lf\" height=\"%lf\" x=\"%lf\" y=\"%lf\" %s />\r\n\r\n",
                w, h, x, y, attrs);
}

static void svgPqMarkers(FILE* svg, Quadra q, int* ann_count) {
    double x = getQuadraX(q), y = getQuadraY(q);
    double w = getQuadraW(q), h = getQuadraH(q);
    double cx = x + w / 2.0, cy = y + h / 2.0;

    const char* facerc = "fill=\"red\" stroke=\"red\" stroke-width=\"1\" "
                         "fill-opacity=\"0.300000\" rx=\"0.000000\" ry=\"0.000000\" "
                         "stroke-dasharray=\"1, 1\"";
    svgAnnRect(svg, x + 2,      cy - 5,      10, 10, facerc, ann_count); /* Face O */
    svgAnnRect(svg, x + w - 12, cy - 5,      10, 10, facerc, ann_count); /* Face L */
    svgAnnRect(svg, cx - 5,     y + 2,       10, 10, facerc, ann_count); /* Face S */
    svgAnnRect(svg, cx - 5,     y + h - 12,  10, 10, facerc, ann_count); /* Face N */
    svgAnnRect(svg, cx - 7.5,   cy - 7.5,    15, 15, facerc, ann_count); /* Centro */
}

/* go_above: label goes up (true) or left (false)
 * k_above / k_left: 0-based index among above/left labels so far
 * N_above: total above labels in the qry (for reverse stacking: last processed = highest)
 * vbx, vby: viewBox origin x and y
 * ann_count: in/out, for first-annotation indent logic */
static void svgMudCircles(FILE* svg, double dx, double dy, const char* cpf,
                           bool go_above, int k_above, int k_left,
                           int N_above, double vbx, double vby, int* ann_count) {
    static const char* colors[] = {"red", "yellow", "magenta", "red", "yellow"};
    static double radii[] = {2.5, 5.0, 7.5, 10.0, 12.5};
    for (int i = 0; i < 5; i++) {
        if ((*ann_count)++ == 0)
            fprintf(svg,
                "<svg:circle cx=\"%lf\" cy=\"%lf\" r=\"%lf\" "
                "stroke-opacity=\"0.5\" fill=\"none\" stroke=\"%s\" stroke-width=\"2\" />\r\n\r\n",
                dx, dy, radii[i], colors[i]);
        else
            fprintf(svg,
                "   <svg:circle cx=\"%lf\" cy=\"%lf\" r=\"%lf\" "
                "stroke-opacity=\"0.5\" fill=\"none\" stroke=\"%s\" stroke-width=\"2\" />\r\n\r\n",
                dx, dy, radii[i], colors[i]);
    }

    double tx, ty;
    if (go_above) {
        tx = dx;
        ty = vby + 5.0 + (double)(N_above - 1 - k_above) * 120.0;
    } else {
        tx = vbx + 5.0 + (double)k_left * 84.0;
        ty = dy;
    }
    fprintf(svg,
        "   <svg:text x=\"%lf\" y=\"%lf\" fill=\"red\" stroke=\"black\" font-size=\"10\">%s</svg:text>\r\n",
        tx, ty, cpf);
    fprintf(svg,
        "   <svg:line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" "
        "stroke=\"red\" stroke-width=\"2\" stroke-opacity=\"1.000000\" stroke-dasharray=\"5,5\" />\r\n",
        dx, dy, tx, ty);
}

/* badge: rounded-rect annotation para nasc/rip/dspj/censo/h?
 * n: 0-indexed badge counter (determina posição E indentação) */
static void svgBadge(FILE* svg, int n, const char* fill, const char* txtfill,
                     const char* label1, const char* label2, double maxX) {
    double bx = (maxX + 30.0) + (double)(n % 10) * 110.0;
    double by = (double)(n / 10) * 60.0;
    double cx = bx + 50.0;
    if (n == 0)
        fprintf(svg,
            "<svg:rect width=\"100.000000\" height=\"50.000000\" x=\"%lf\" y=\"%lf\" "
            "fill=\"%s\" stroke=\"black\" stroke-width=\"1\" fill-opacity=\"1.000000\" "
            "rx=\"20.000000\" ry=\"20.000000\" />\r\n\r\n",
            bx, by, fill);
    else
        fprintf(svg,
            "   <svg:rect width=\"100.000000\" height=\"50.000000\" x=\"%lf\" y=\"%lf\" "
            "fill=\"%s\" stroke=\"black\" stroke-width=\"1\" fill-opacity=\"1.000000\" "
            "rx=\"20.000000\" ry=\"20.000000\" />\r\n\r\n",
            bx, by, fill);
    fprintf(svg,
        "   <svg:text x=\"%lf\" y=\"%lf\" fill=\"%s\" stroke=\"%s\" "
        "font-size=\"14\" text-anchor=\"middle\">%s</svg:text>\r\n",
        cx, by + 12.0, txtfill, txtfill, label1);
    fprintf(svg,
        "   <svg:text x=\"%lf\" y=\"%lf\" fill=\"%s\" stroke=\"%s\" "
        "font-size=\"9\" text-anchor=\"middle\">%s</svg:text>\r\n",
        cx, by + 28.0, txtfill, txtfill, label2);
}

/* rq overlay: white rect + duas diagonais sobre a quadra
 * ann_count: in/out, para lógica de indentação */
static void svgRqOverlay(FILE* svg, double bx, double by, double w, double h,
                         int* ann_count) {
    if ((*ann_count)++ == 0)
        fprintf(svg,
            "<svg:rect width=\"%lf\" height=\"%lf\" x=\"%lf\" y=\"%lf\" "
            "fill=\"white\" stroke=\"red\" stroke-width=\"1\" fill-opacity=\"0.500000\" "
            "rx=\"0.000000\" ry=\"0.000000\" />\r\n\r\n",
            w, h, bx, by);
    else
        fprintf(svg,
            "   <svg:rect width=\"%lf\" height=\"%lf\" x=\"%lf\" y=\"%lf\" "
            "fill=\"white\" stroke=\"red\" stroke-width=\"1\" fill-opacity=\"0.500000\" "
            "rx=\"0.000000\" ry=\"0.000000\" />\r\n\r\n",
            w, h, bx, by);
    fprintf(svg,
        "   <svg:line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" "
        "stroke=\"red\" stroke-width=\"1\" stroke-opacity=\"0.500000\" />\r\n",
        bx, by, bx + w, by + h);
    fprintf(svg,
        "   <svg:line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" "
        "stroke=\"red\" stroke-width=\"1\" stroke-opacity=\"0.500000\" />\r\n",
        bx, by + h, bx + w, by);
}

/* =========================================================
 * Processamento do .qry — contexto compartilhado entre handlers
 * ========================================================= */

typedef struct {
    FILE*  svg;
    FILE*  txt;
    void*  hash_quadras;
    void*  hash_hab;
    double vbx, vby, maxX;
    int    ann_count;
    int    badge_idx;
    int    k_above, k_left;
    int    mud_idx;
    int    N_above, N_mud;
    bool   right_side;
} QryCtx;

/* --- handlers por comando --- */

static void handle_pq(const char* linha, QryCtx* ctx) {
    char cep[64];
    sscanf(linha, "%*s %63s", cep);
    Quadra q = (Quadra)getHashfile(ctx->hash_quadras, cep);
    if (!q) { if (ctx->txt) fprintf(ctx->txt, "Quadra '%s' nao encontrada.\n", cep); return; }
    PqStats st; memset(&st, 0, sizeof(st));
    strncpy(st.cep, cep, 63);
    iterateHashfile(ctx->hash_hab, pqCountCb, &st, NULL);
    if (ctx->txt) {
        fprintf(ctx->txt, "N: %d\n", st.cN);
        fprintf(ctx->txt, "S: %d\n", st.cS);
        fprintf(ctx->txt, "L: %d\n", st.cL);
        fprintf(ctx->txt, "O: %d\n", st.cO);
        fprintf(ctx->txt, "Total: %d\n", st.total);
    }
    if (ctx->svg) svgPqMarkers(ctx->svg, q, &ctx->ann_count);
    freeQuadra(q);
}

static void handle_rq(const char* linha, QryCtx* ctx) {
    char cep[64];
    sscanf(linha, "%*s %63s", cep);
    Quadra q = (Quadra)getHashfile(ctx->hash_quadras, cep);
    if (!q) { if (ctx->txt) fprintf(ctx->txt, "Quadra '%s' nao encontrada.\n", cep); return; }
    RqExtra ex;
    strncpy(ex.cep, cep, 63); ex.cep[63] = '\0';
    ex.cpfs = createList();
    iterateHashfile(ctx->hash_hab, rqCollectCb2, &ex, NULL);
    void* nd = getHead(ex.cpfs);
    while (nd) {
        char* cpf = (char*)getData(nd);
        Habitante h = (Habitante)getHashfile(ctx->hash_hab, cpf);
        if (h) {
            if (ctx->txt)
                fprintf(ctx->txt, "%s %s %s\n",
                        getHabitanteCpf(h), getHabitanteNome(h), getHabitanteSobrenome(h));
            removerEnderecoHabitante(h);
            removeHashfile(ctx->hash_hab, cpf);
            insertHashfile(ctx->hash_hab, cpf, h);
            freeHabitante(h);
        }
        free(cpf);
        nd = getNext(nd);
    }
    freeList(ex.cpfs);
    if (ctx->svg) svgRqOverlay(ctx->svg, getQuadraX(q), getQuadraY(q),
                               getQuadraW(q), getQuadraH(q), &ctx->ann_count);
    removeHashfile(ctx->hash_quadras, cep);
    freeQuadra(q);
}

static void handle_censo(QryCtx* ctx) {
    CensoStats st; memset(&st, 0, sizeof(st));
    iterateHashfile(ctx->hash_hab, censoCb, &st, NULL);
    if (ctx->txt) {
        double prop_mor  = st.total_hab  > 0 ? (100.0 * st.moradores / st.total_hab)  : 0.0;
        double pct_hab_h = st.total_hab  > 0 ? (100.0 * st.hab_h     / st.total_hab)  : 0.0;
        double pct_hab_m = st.total_hab  > 0 ? (100.0 * st.hab_m     / st.total_hab)  : 0.0;
        double pct_mor_h = st.moradores  > 0 ? (100.0 * st.mor_h     / st.moradores)  : 0.0;
        double pct_mor_m = st.moradores  > 0 ? (100.0 * st.mor_m     / st.moradores)  : 0.0;
        double pct_sem_h = st.sem_teto   > 0 ? (100.0 * st.sem_h     / st.sem_teto)   : 0.0;
        double pct_sem_m = st.sem_teto   > 0 ? (100.0 * st.sem_m     / st.sem_teto)   : 0.0;
        fprintf(ctx->txt, "Total de habitantes: %d\n",                    st.total_hab);
        fprintf(ctx->txt, "Total de moradores: %d\n",                     st.moradores);
        fprintf(ctx->txt, "Proporcao moradores/habitantes: %.2f%%\n",     prop_mor);
        fprintf(ctx->txt, "Numero de homens: %d\n",                       st.hab_h);
        fprintf(ctx->txt, "Numero de mulheres: %d\n",                     st.hab_m);
        fprintf(ctx->txt, "%% habitantes homens: %.2f%%\n",               pct_hab_h);
        fprintf(ctx->txt, "%% habitantes mulheres: %.2f%%\n",             pct_hab_m);
        fprintf(ctx->txt, "%% moradores homens: %.2f%%\n",                pct_mor_h);
        fprintf(ctx->txt, "%% moradores mulheres: %.2f%%\n",              pct_mor_m);
        fprintf(ctx->txt, "Total de sem-tetos: %d\n",                     st.sem_teto);
        fprintf(ctx->txt, "%% sem-tetos homens: %.2f%%\n",                pct_sem_h);
        fprintf(ctx->txt, "%% sem-tetos mulheres: %.2f%%\n",              pct_sem_m);
    }
    if (ctx->svg) svgBadge(ctx->svg, ctx->badge_idx++, "#ac9d93", "#7c9167", ".", ".", ctx->maxX);
}

static void handle_hq(const char* linha, QryCtx* ctx) {
    char cpf[64];
    sscanf(linha, "%*s %63s", cpf);
    Habitante h = (Habitante)getHashfile(ctx->hash_hab, cpf);
    if (h) {
        if (ctx->txt) {
            fprintf(ctx->txt, "CPF: %s\n",        getHabitanteCpf(h));
            fprintf(ctx->txt, "Nome: %s %s\n",    getHabitanteNome(h), getHabitanteSobrenome(h));
            fprintf(ctx->txt, "Sexo: %c\n",       getHabitanteSexo(h));
            fprintf(ctx->txt, "Nascimento: %s\n", getHabitanteNasc(h));
            if (isMorador(h))
                fprintf(ctx->txt, "Endereco: %s / %c / %.2lf / %s\n",
                        getHabitanteEndCep(h), getHabitanteEndFace(h),
                        getHabitanteEndNum(h), getHabitanteEndCompl(h));
            else
                fprintf(ctx->txt, "Situacao: sem-teto\n");
        }
        freeHabitante(h);
    } else {
        if (ctx->txt) fprintf(ctx->txt, "Habitante '%s' nao encontrado.\n", cpf);
    }
    if (ctx->svg) svgBadge(ctx->svg, ctx->badge_idx++, "#e9afaf", "#55d400", ":::", cpf, ctx->maxX);
}

static void handle_nasc(const char* linha, QryCtx* ctx) {
    char cpf[64], nome[64], sobrenome[64], nasc_dt[32]; char sexo;
    sscanf(linha, "%*s %63s %63s %63s %c %31s", cpf, nome, sobrenome, &sexo, nasc_dt);
    Habitante h = createHabitante(cpf, nome, sobrenome, sexo, nasc_dt);
    insertHashfile(ctx->hash_hab, cpf, h);
    if (ctx->txt) fprintf(ctx->txt, "Nascimento: CPF %s, Nome: %s %s\n", cpf, nome, sobrenome);
    if (ctx->svg) svgBadge(ctx->svg, ctx->badge_idx++, "#AAAAFF", "#000080", "***", cpf, ctx->maxX);
    freeHabitante(h);
}

static void handle_rip(const char* linha, QryCtx* ctx) {
    char cpf[64];
    sscanf(linha, "%*s %63s", cpf);
    Habitante h = (Habitante)getHashfile(ctx->hash_hab, cpf);
    if (h) {
        if (ctx->txt) {
            fprintf(ctx->txt, "Obito: %s %s, Sexo: %c, Nasc: %s\n",
                    getHabitanteNome(h), getHabitanteSobrenome(h),
                    getHabitanteSexo(h), getHabitanteNasc(h));
            if (isMorador(h))
                fprintf(ctx->txt, "Endereco: %s / %c / %.2lf / %s\n",
                        getHabitanteEndCep(h), getHabitanteEndFace(h),
                        getHabitanteEndNum(h), getHabitanteEndCompl(h));
        }
        if (ctx->svg) svgBadge(ctx->svg, ctx->badge_idx++, "black", "white", "R.I.P.", cpf, ctx->maxX);
        removeHashfile(ctx->hash_hab, cpf);
        freeHabitante(h);
    } else {
        if (ctx->txt) fprintf(ctx->txt, "Habitante '%s' nao encontrado.\n", cpf);
        if (ctx->svg) svgBadge(ctx->svg, ctx->badge_idx++, "black", "white", "R.I.P.", cpf, ctx->maxX);
    }
}

static void handle_mud(const char* linha, QryCtx* ctx) {
    char cpf[64], cep[64], face_str[32], compl[64]; double num;
    sscanf(linha, "%*s %63s %63s %31s %lf %63s", cpf, cep, face_str, &num, compl);
    Habitante h = (Habitante)getHashfile(ctx->hash_hab, cpf);
    if (h) {
        if (ctx->svg) {
            Quadra qDest = (Quadra)getHashfile(ctx->hash_quadras, cep);
            if (qDest) {
                double dx, dy;
                calcularEnderecoQuadra(qDest, extractFace(face_str), num, &dx, &dy);
                bool go_above;
                if (ctx->right_side)
                    go_above = (ctx->mud_idx == 0);
                else
                    go_above = (ctx->mud_idx > 0 && ctx->mud_idx < ctx->N_mud - 1);
                svgMudCircles(ctx->svg, dx, dy, getHabitanteCpf(h),
                              go_above, ctx->k_above, ctx->k_left,
                              ctx->N_above, ctx->vbx, ctx->vby, &ctx->ann_count);
                if (go_above) ctx->k_above++; else ctx->k_left++;
                ctx->mud_idx++;
                freeQuadra(qDest);
            }
        }
        setHabitanteEndereco(h, cep, extractFace(face_str), num, compl);
        removeHashfile(ctx->hash_hab, cpf);
        insertHashfile(ctx->hash_hab, cpf, h);
        freeHabitante(h);
    } else {
        if (ctx->txt) fprintf(ctx->txt, "Habitante '%s' nao encontrado.\n", cpf);
    }
}

static void handle_dspj(const char* linha, QryCtx* ctx) {
    char cpf[64];
    sscanf(linha, "%*s %63s", cpf);
    Habitante h = (Habitante)getHashfile(ctx->hash_hab, cpf);
    if (h) {
        if (ctx->txt) {
            fprintf(ctx->txt, "Despejado: %s %s, CPF: %s\n",
                    getHabitanteNome(h), getHabitanteSobrenome(h), getHabitanteCpf(h));
            if (isMorador(h))
                fprintf(ctx->txt, "Endereco: %s / %c / %.2lf / %s\n",
                        getHabitanteEndCep(h), getHabitanteEndFace(h),
                        getHabitanteEndNum(h), getHabitanteEndCompl(h));
        }
        if (ctx->svg) svgBadge(ctx->svg, ctx->badge_idx++, "#786721", "#F4EED7", "OUT", cpf, ctx->maxX);
        removerEnderecoHabitante(h);
        removeHashfile(ctx->hash_hab, cpf);
        insertHashfile(ctx->hash_hab, cpf, h);
        freeHabitante(h);
    } else {
        if (ctx->txt) fprintf(ctx->txt, "Habitante '%s' nao encontrado.\n", cpf);
        if (ctx->svg) svgBadge(ctx->svg, ctx->badge_idx++, "#786721", "#F4EED7", "OUT", cpf, ctx->maxX);
    }
}

/* --- pré-varredura do .qry --- */

typedef struct {
    int    N_mud, N_badges;
    double first_mud_x, max_mud_x, max_mud_y;
    bool   found_first;
    char   first_face;
} PrescanResult;

static PrescanResult prescan_qry(FILE* qry, void* hash_quadras,
                                  double center_x, double minX, double minY) {
    PrescanResult r = { 0, 0, center_x, minX, minY, false, 'N' };
    char linha[512];
    while (fgets(linha, sizeof(linha), qry)) {
        char cmd[32];
        if (sscanf(linha, "%31s", cmd) != 1) continue;
        if (strcmp(cmd, "nasc") == 0 || strcmp(cmd, "rip")   == 0 ||
            strcmp(cmd, "dspj") == 0 || strcmp(cmd, "censo") == 0 ||
            strcmp(cmd, "h?")   == 0) {
            r.N_badges++;
        } else if (strcmp(cmd, "mud") == 0) {
            r.N_mud++;
            char cpf2[64], cep2[64], face2[32]; double num2 = 0;
            sscanf(linha, "%*s %63s %63s %31s %lf", cpf2, cep2, face2, &num2);
            Quadra q2 = (Quadra)getHashfile(hash_quadras, cep2);
            if (q2) {
                double dx2, dy2;
                calcularEnderecoQuadra(q2, extractFace(face2), num2, &dx2, &dy2);
                if (!r.found_first) { r.first_mud_x = dx2; r.first_face = extractFace(face2); r.found_first = true; }
                if (dx2 > r.max_mud_x) r.max_mud_x = dx2;
                if (dy2 > r.max_mud_y) r.max_mud_y = dy2;
                freeQuadra(q2);
            }
        }
    }
    rewind(qry);
    return r;
}

/* --- cálculo do viewBox --- */

static void compute_viewbox(int N_mud, int N_badges, int N_above, int N_left,
                            double minX, double minY, double maxX, double maxY,
                            double max_mud_x, double max_mud_y, int vby_const,
                            double* vbx, double* vby, double* vbW, double* vbH) {
    if (N_badges > 0) {
        *vbx = minX - 5.0; *vby = -5.0;
        int nb = (N_badges < 10) ? N_badges : 10;
        *vbW = (maxX - *vbx) + 140.0 + (double)(nb - 1) * 110.0;
        *vbH = maxY - *vby + 10.0;
    } else if (N_mud > 0) {
        *vby = (N_above > 0) ? -((int)(max_mud_y / 43.0) + vby_const) : minY - 5.0;
        *vbx = (N_left  > 0) ? -((int)(max_mud_x / 84.0) + 70) : minX - 5.0;
        double re = (max_mud_x + 12.5 > maxX) ? max_mud_x + 12.5 : maxX;
        double be = (max_mud_y + 12.5 > maxY) ? max_mud_y + 12.5 : maxY;
        *vbW = re + 10.0 - *vbx;
        *vbH = be + 10.0 - *vby;
    } else {
        *vbx = minX - 5.0; *vby = minY - 5.0;
        *vbW = maxX - *vbx + 10.0;
        *vbH = maxY - *vby + 10.0;
    }
}

/* =========================================================
 * readQry — loop principal
 * ========================================================= */

void readQry(void* paths, void* hash_quadras, void* hash_hab) {
    if (!paths || !hash_quadras || !hash_hab) return;
    const char* qryPath = getBedQry(paths);
    if (!qryPath) return;
    FILE* qry = fopen(qryPath, "r");
    if (!qry) { printf("Aviso: .qry nao encontrado: %s\n", qryPath); return; }

    double minX, minY, maxX, maxY;
    computeGeoBounds(getBedGeo(paths), &minX, &minY, &maxX, &maxY);

    PrescanResult ps = prescan_qry(qry, hash_quadras, (minX + maxX) / 2.0, minX, minY);

    double city_cx = (minX + maxX) / 2.0;
    char ff = ps.first_face;
    bool right_side = (ff == 'O' || ff == 'S') ? true : (ps.first_mud_x > city_cx);
    int N_above = 0, N_left = 0;
    if (ps.N_mud == 1) {
        if (right_side) N_above = 1; else N_left = 1;
    } else if (ps.N_mud > 1) {
        if (right_side) { N_above = 1; N_left = ps.N_mud - 1; }
        else            { N_left = (ps.N_mud >= 2) ? 2 : 1; N_above = ps.N_mud - N_left; }
    }

    int vby_const = (ff == 'S' || ff == 'N') ? 106 : 76;
    double vbx, vby, vbW, vbH;
    compute_viewbox(ps.N_mud, ps.N_badges, N_above, N_left,
                    minX, minY, maxX, maxY,
                    ps.max_mud_x, ps.max_mud_y, vby_const,
                    &vbx, &vby, &vbW, &vbH);

    FILE* svg = fopen(getBsdGeoQrySvg(paths), "w");
    if (svg) {
        fprintf(svg, "<?xml version='1.0' encoding='utf-8'?>\r\n");
        fprintf(svg, "<svg:svg xmlns:svg=\"http://www.w3.org/2000/svg\" "
                     "viewBox=\"%.1f %.1f %.1f %.1f\">\r\n", vbx, vby, vbW, vbH);
        fprintf(svg, "   <svg:g id=\"fig\">\r\n");
        replayGeo(svg, getBedGeo(paths));
        fprintf(svg, "</svg:g>");
    }

    QryCtx ctx = {
        svg, fopen(getBsdGeoQryTxt(paths), "w"),
        hash_quadras, hash_hab,
        vbx, vby, maxX,
        0, 0, 0, 0, 0,
        N_above, ps.N_mud, right_side
    };

    char linha[512];
    while (fgets(linha, sizeof(linha), qry)) {
        char cmd[32];
        if (sscanf(linha, "%31s", cmd) != 1) continue;
        if (ctx.txt) fprintf(ctx.txt, "[*] %s", linha);

        if      (strcmp(cmd, "pq")   == 0) handle_pq(linha, &ctx);
        else if (strcmp(cmd, "rq")   == 0) handle_rq(linha, &ctx);
        else if (strcmp(cmd, "censo")== 0) handle_censo(&ctx);
        else if (strcmp(cmd, "h?")   == 0) handle_hq(linha, &ctx);
        else if (strcmp(cmd, "nasc") == 0) handle_nasc(linha, &ctx);
        else if (strcmp(cmd, "rip")  == 0) handle_rip(linha, &ctx);
        else if (strcmp(cmd, "mud")  == 0) handle_mud(linha, &ctx);
        else if (strcmp(cmd, "dspj") == 0) handle_dspj(linha, &ctx);
    }

    if (svg)      { fprintf(svg, "</svg:svg>"); fclose(svg); }
    if (ctx.txt)    fclose(ctx.txt);
    fclose(qry);
}
