#include "hash_extensivel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUCKET_SIZE 3

typedef struct {
    int chave;
    char valor[50];
} Registro;

typedef struct {
    int profundidade_local;
    int contagem;
    Registro registros[BUCKET_SIZE];
} Bucket;

typedef struct {
    int profundidade_global;
    int tamanho;
    long *offsets;
} Diretorio;

struct HashExtensivel {
    FILE *arq_dados;
    char caminho_dir[256];
    Diretorio dir;
};

static int hash_func(int chave) { return chave; }

static int obter_indice(int chave, int prof) { return hash_func(chave) & ((1 << prof) - 1); }

HashExtensivel* hash_extensivel_abrir(const char* caminho_dados, const char* caminho_dir) {
    HashExtensivel* hash = (HashExtensivel*)malloc(sizeof(HashExtensivel));
    if (!hash) return NULL;

    strncpy(hash->caminho_dir, caminho_dir, 255);
    hash->caminho_dir[255] = '\0';

    FILE* f_dir = fopen(caminho_dir, "rb");
    if (f_dir) {
        hash->arq_dados = fopen(caminho_dados, "rb+");
        fread(&hash->dir.profundidade_global, sizeof(int), 1, f_dir);
        fread(&hash->dir.tamanho, sizeof(int), 1, f_dir);
        hash->dir.offsets = (long*)malloc(sizeof(long) * hash->dir.tamanho);
        fread(hash->dir.offsets, sizeof(long), hash->dir.tamanho, f_dir);
        fclose(f_dir);
    } else {
        hash->arq_dados = fopen(caminho_dados, "wb+");
        hash->dir.profundidade_global = 0;
        hash->dir.tamanho = 1;
        hash->dir.offsets = (long*)malloc(sizeof(long));
        hash->dir.offsets[0] = 0;

        Bucket b = {0, 0};
        fwrite(&b, sizeof(Bucket), 1, hash->arq_dados);
    }
    return hash;
}

static void dividir_bucket(HashExtensivel* hash, int indice_split) {
    long offset_antigo = hash->dir.offsets[indice_split];
    Bucket b_velha;
    
    fseek(hash->arq_dados, offset_antigo, SEEK_SET);
    fread(&b_velha, sizeof(Bucket), 1, hash->arq_dados);

    if (b_velha.profundidade_local == hash->dir.profundidade_global) {
        int tam_antigo = hash->dir.tamanho;
        hash->dir.tamanho *= 2;
        hash->dir.offsets = (long*)realloc(hash->dir.offsets, sizeof(long) * hash->dir.tamanho);
        
        for (int i = 0; i < tam_antigo; i++) {
            hash->dir.offsets[i + tam_antigo] = hash->dir.offsets[i];
        }
        hash->dir.profundidade_global++;
    }

    Bucket b_nova;
    b_nova.profundidade_local = b_velha.profundidade_local + 1;
    b_nova.contagem = 0;
    b_velha.profundidade_local++;

    fseek(hash->arq_dados, 0, SEEK_END);
    long offset_novo = ftell(hash->arq_dados);
    fwrite(&b_nova, sizeof(Bucket), 1, hash->arq_dados);

    int bit_verificacao = 1 << (b_velha.profundidade_local - 1);
    for (int i = 0; i < hash->dir.tamanho; i++) {
        if (hash->dir.offsets[i] == offset_antigo) {
            if ((i & bit_verificacao) != 0) {
                hash->dir.offsets[i] = offset_novo;
            }
        }
    }

    Registro temp_regs[BUCKET_SIZE];
    int qtd_antiga = b_velha.contagem;
    memcpy(temp_regs, b_velha.registros, sizeof(Registro) * qtd_antiga);
    
    b_velha.contagem = 0;
    fseek(hash->arq_dados, offset_antigo, SEEK_SET);
    fwrite(&b_velha, sizeof(Bucket), 1, hash->arq_dados);

    for (int i = 0; i < qtd_antiga; i++) {
        hash_extensivel_inserir(hash, temp_regs[i].chave, temp_regs[i].valor);
    }
}

bool hash_extensivel_inserir(HashExtensivel* hash, int chave, const char* valor) {
    if (!hash) return false;

    int idx = obter_indice(chave, hash->dir.profundidade_global);
    Bucket b;
    fseek(hash->arq_dados, hash->dir.offsets[idx], SEEK_SET);
    fread(&b, sizeof(Bucket), 1, hash->arq_dados);

    if (b.contagem < BUCKET_SIZE) {
        b.registros[b.contagem].chave = chave;
        strncpy(b.registros[b.contagem].valor, valor, 49);
        b.registros[b.contagem].valor[49] = '\0';
        b.contagem++;
        
        fseek(hash->arq_dados, hash->dir.offsets[idx], SEEK_SET);
        fwrite(&b, sizeof(Bucket), 1, hash->arq_dados);
        return true;
    } else {
        dividir_bucket(hash, idx);
        return hash_extensivel_inserir(hash, chave, valor); 
    }
}

bool hash_extensivel_buscar(HashExtensivel* hash, int chave, char* valor_out) {
    if (!hash) return false;

    int idx = obter_indice(chave, hash->dir.profundidade_global);
    Bucket b;
    fseek(hash->arq_dados, hash->dir.offsets[idx], SEEK_SET);
    fread(&b, sizeof(Bucket), 1, hash->arq_dados);

    for (int i = 0; i < b.contagem; i++) {
        if (b.registros[i].chave == chave) {
            if (valor_out) strcpy(valor_out, b.registros[i].valor);
            return true;
        }
    }
    return false;
}

bool hash_extensivel_remover(HashExtensivel* hash, int chave) {
    if (!hash) return false;

    int idx = obter_indice(chave, hash->dir.profundidade_global);
    Bucket b;
    fseek(hash->arq_dados, hash->dir.offsets[idx], SEEK_SET);
    fread(&b, sizeof(Bucket), 1, hash->arq_dados);

    for (int i = 0; i < b.contagem; i++) {
        if (b.registros[i].chave == chave) {
            b.registros[i] = b.registros[--b.contagem];
            fseek(hash->arq_dados, hash->dir.offsets[idx], SEEK_SET);
            fwrite(&b, sizeof(Bucket), 1, hash->arq_dados);
            return true;
        }
    }
    return false;
}

void hash_extensivel_fechar(HashExtensivel* hash) {
    if (!hash) return;
    
    FILE *f_dir = fopen(hash->caminho_dir, "wb");
    if (f_dir) {
        fwrite(&hash->dir.profundidade_global, sizeof(int), 1, f_dir);
        fwrite(&hash->dir.tamanho, sizeof(int), 1, f_dir);
        fwrite(hash->dir.offsets, sizeof(long), hash->dir.tamanho, f_dir);
        fclose(f_dir);
    }

    if (hash->arq_dados) fclose(hash->arq_dados);
    if (hash->dir.offsets) free(hash->dir.offsets);
    free(hash);
}
