#include "hash_extensivel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUCKET_SIZE 3

typedef struct {
    int id;
    char dado[50];
} Registro;

typedef struct {
    int p_local;
    int ocupacao;
    Registro itens[BUCKET_SIZE];
} Bucket;

typedef struct {
    int p_global;
    int n_entradas;
    long *enderecos;
} Diretorio;

struct HashExtensivel {
    FILE *dados;
    char path_dir[256];
    Diretorio dir;
};

static int h(int k) { return k; }

static int calcula_indice(int k, int p) { 
    return h(k) & ((1 << p) - 1); 
}

HashExtensivel* hash_extensivel_abrir(const char* caminho_dados, const char* caminho_dir) {
    HashExtensivel* h_ext = (HashExtensivel*)malloc(sizeof(HashExtensivel));
    if (!h_ext) return NULL;

    strncpy(h_ext->path_dir, caminho_dir, 255);
    h_ext->path_dir[255] = '\0';

    FILE* f_ind = fopen(caminho_dir, "rb");
    if (f_ind) {
        h_ext->dados = fopen(caminho_dados, "rb+");
        fread(&h_ext->dir.p_global, sizeof(int), 1, f_ind);
        fread(&h_ext->dir.n_entradas, sizeof(int), 1, f_ind);
        h_ext->dir.enderecos = (long*)malloc(sizeof(long) * h_ext->dir.n_entradas);
        fread(h_ext->dir.enderecos, sizeof(long), h_ext->dir.n_entradas, f_ind);
        fclose(f_ind);
    } else {
        h_ext->dados = fopen(caminho_dados, "wb+");
        h_ext->dir.p_global = 0;
        h_ext->dir.n_entradas = 1;
        h_ext->dir.enderecos = (long*)malloc(sizeof(long));
        h_ext->dir.enderecos[0] = 0;

        Bucket aux_b;
        memset(&aux_b, 0, sizeof(Bucket));
        fwrite(&aux_b, sizeof(Bucket), 1, h_ext->dados);
    }
    return h_ext;
}

static void split(HashExtensivel* h_ext, int idx_split) {
    long off_velho = h_ext->dir.enderecos[idx_split];
    Bucket b_velha;
    
    fseek(h_ext->dados, off_velho, SEEK_SET);
    fread(&b_velha, sizeof(Bucket), 1, h_ext->dados);

    if (b_velha.p_local == h_ext->dir.p_global) {
        int tam_atual = h_ext->dir.n_entradas;
        h_ext->dir.n_entradas *= 2;
        h_ext->dir.enderecos = (long*)realloc(h_ext->dir.enderecos, sizeof(long) * h_ext->dir.n_entradas);
        
        for (int i = 0; i < tam_atual; i++) {
            h_ext->dir.enderecos[i + tam_atual] = h_ext->dir.enderecos[i];
        }
        h_ext->dir.p_global++;
    }

    Bucket b_nova;
    b_nova.p_local = b_velha.p_local + 1;
    b_nova.ocupacao = 0;
    b_velha.p_local++;

    fseek(h_ext->dados, 0, SEEK_END);
    long off_novo = ftell(h_ext->dados);
    fwrite(&b_nova, sizeof(Bucket), 1, h_ext->dados);

    int bit = 1 << (b_velha.p_local - 1);
    for (int i = 0; i < h_ext->dir.n_entradas; i++) {
        if (h_ext->dir.enderecos[i] == off_velho) {
            if ((i & bit) != 0) {
                h_ext->dir.enderecos[i] = off_novo;
            }
        }
    }

    Registro copia_itens[BUCKET_SIZE];
    int n_itens = b_velha.ocupacao;
    memcpy(copia_itens, b_velha.itens, sizeof(Registro) * n_itens);
    
    b_velha.ocupacao = 0;
    fseek(h_ext->dados, off_velho, SEEK_SET);
    fwrite(&b_velha, sizeof(Bucket), 1, h_ext->dados);

    for (int i = 0; i < n_itens; i++) {
        hash_extensivel_inserir(h_ext, copia_itens[i].id, copia_itens[i].dado);
    }
}

bool hash_extensivel_inserir(HashExtensivel* h_ext, int chave, const char* valor) {
    if (!h_ext) return false;

    int pos = calcula_indice(chave, h_ext->dir.p_global);
    Bucket aux_b;
    fseek(h_ext->dados, h_ext->dir.enderecos[pos], SEEK_SET);
    fread(&aux_b, sizeof(Bucket), 1, h_ext->dados);

    if (aux_b.ocupacao < BUCKET_SIZE) {
        aux_b.itens[aux_b.ocupacao].id = chave;
        strncpy(aux_b.itens[aux_b.ocupacao].dado, valor, 49);
        aux_b.itens[aux_b.ocupacao].dado[49] = '\0';
        aux_b.ocupacao++;
        
        fseek(h_ext->dados, h_ext->dir.enderecos[pos], SEEK_SET);
        fwrite(&aux_b, sizeof(Bucket), 1, h_ext->dados);
        return true;
    } else {
        split(h_ext, pos);
        return hash_extensivel_inserir(h_ext, chave, valor); 
    }
}

bool hash_extensivel_buscar(HashExtensivel* h_ext, int chave, char* valor_out) {
    if (!h_ext) return false;

    int pos = calcula_indice(chave, h_ext->dir.p_global);
    Bucket aux_b;
    fseek(h_ext->dados, h_ext->dir.enderecos[pos], SEEK_SET);
    fread(&aux_b, sizeof(Bucket), 1, h_ext->dados);

    for (int i = 0; i < aux_b.ocupacao; i++) {
        if (aux_b.itens[i].id == chave) {
            if (valor_out) strcpy(valor_out, aux_b.itens[i].dado);
            return true;
        }
    }
    return false;
}

bool hash_extensivel_remover(HashExtensivel* h_ext, int chave) {
    if (!h_ext) return false;

    int pos = calcula_indice(chave, h_ext->dir.p_global);
    Bucket aux_b;
    fseek(h_ext->dados, h_ext->dir.enderecos[pos], SEEK_SET);
    fread(&aux_b, sizeof(Bucket), 1, h_ext->dados);

    for (int i = 0; i < aux_b.ocupacao; i++) {
        if (aux_b.itens[i].id == chave) {
            aux_b.itens[i] = aux_b.itens[--aux_b.ocupacao];
            fseek(h_ext->dados, h_ext->dir.enderecos[pos], SEEK_SET);
            fwrite(&aux_b, sizeof(Bucket), 1, h_ext->dados);
            return true;
        }
    }
    return false;
}

void hash_extensivel_fechar(HashExtensivel* h_ext) {
    if (!h_ext) return;
    
    FILE *f_dir = fopen(h_ext->path_dir, "wb");
    if (f_dir) {
        fwrite(&h_ext->dir.p_global, sizeof(int), 1, f_dir);
        fwrite(&h_ext->dir.n_entradas, sizeof(int), 1, f_dir);
        fwrite(h_ext->dir.enderecos, sizeof(long), h_ext->dir.n_entradas, f_dir);
        fclose(f_dir);
    }

    if (h_ext->dados) fclose(h_ext->dados);
    if (h_ext->dir.enderecos) free(h_ext->dir.enderecos);
    free(h_ext);
}
