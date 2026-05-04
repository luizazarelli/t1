#include "quadra.h"

typedef struct quadra_info {
    char   cep[32];
    double x;
    double y;
    double w;
    double h;
    char   sw[16];
    char   cfill[32];
    char   cstrk[32];
} quadra_info;

Quadra createQuadra(char* cep, double x, double y, double w, double h, 
                    char* sw, char* cfill, char* cstrk) {
    quadra_info* q = calloc(1, sizeof(quadra_info));
    assert(q != NULL);

    if (cep) strncpy(q->cep, cep, 31);
    q->x = x;
    q->y = y;
    q->w = w;
    q->h = h;

    if (sw) strncpy(q->sw, sw, 15); else strcpy(q->sw, "1.0px");
    if (cfill) strncpy(q->cfill, cfill, 31); else strcpy(q->cfill, "red");
    if (cstrk) strncpy(q->cstrk, cstrk, 31); else strcpy(q->cstrk, "black");

    return q;
}

void freeQuadra(Quadra q) {
    if (!q) return;
    free(q);
}

char* getQuadraCep(Quadra q)    { return ((quadra_info*)q)->cep; }
double getQuadraX(Quadra q)     { return ((quadra_info*)q)->x; }
double getQuadraY(Quadra q)     { return ((quadra_info*)q)->y; }
double getQuadraW(Quadra q)     { return ((quadra_info*)q)->w; }
double getQuadraH(Quadra q)     { return ((quadra_info*)q)->h; }
char* getQuadraSw(Quadra q)     { return ((quadra_info*)q)->sw; }
char* getQuadraCfill(Quadra q)  { return ((quadra_info*)q)->cfill; }
char* getQuadraCstrk(Quadra q)  { return ((quadra_info*)q)->cstrk; }

// De acordo com o doc:
// Ancora: Canto Inferior Esquerdo (tradicional SVG/Geometrico). Se basearmos no PDF:
// Face S: y, variando x
// Face N: y+h, variando x
// Face L: x+w, variando y
// Face O: x, variando y
void calcularEnderecoQuadra(Quadra q, char face, double num, double* out_x, double* out_y) {
    if (!q || !out_x || !out_y) return;
    quadra_info* q_info = (quadra_info*)q;

    switch (face) {
        case 'N':
            *out_x = q_info->x + num;
            *out_y = q_info->y + q_info->h;
            break;
        case 'S':
            *out_x = q_info->x + q_info->w;
            *out_y = q_info->y + num;
            break;
        case 'L':
        case 'W':
            *out_x = q_info->x + q_info->w;
            *out_y = q_info->y + num;
            break;
        case 'O':
            *out_x = q_info->x + q_info->w;
            *out_y = q_info->y + num;
            break;
        default:
            *out_x = q_info->x;
            *out_y = q_info->y;
            break;
    }
}

size_t getQuadraSize() { 
    return sizeof(quadra_info); 
}
