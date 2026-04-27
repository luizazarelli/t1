#include "libs.h"
#include "paths.h"

#ifndef SYSTEM_H
#define SYSTEM_H

/*
 * Modulo System
 *
 * Coordena a leitura dos arquivos de entrada (.geo, .pm, .qry),
 * a insercao de dados nos hashfiles e a geracao das saidas (SVG e TXT).
 */

/*
 * readParam: processa os argumentos de linha de comando e preenche a estrutura de caminhos.
 * Argumentos aceitos: -e <dir_entrada> -f <arquivo.geo> -o <dir_saida>
 *                     -pm <arquivo.pm> -q <cidade/query.qry>
 * entrada: argc, argv — argumentos do main; paths — estrutura criada por createAllPaths().
 * saida: nenhuma (paths e' preenchido internamente). Termina o programa se -f ou -o ausentes.
 */
void readParam(int argc, char** argv, void* paths);

/*
 * readPrintGeo: le o arquivo .geo, insere as quadras no hashfile e gera o SVG base da cidade.
 * entrada: paths — caminhos configurados; hash_quadras — hashfile aberto para escrita.
 * saida: nenhuma (SVG gerado em bsdgeosvg; quadras inseridas em hash_quadras).
 */
void readPrintGeo(void* paths, void* hash_quadras);

/*
 * readPm: le o arquivo .pm e insere/atualiza habitantes e seus enderecos no hashfile.
 * entrada: paths — caminhos configurados; hash_hab — hashfile aberto para escrita.
 * saida: nenhuma (habitantes inseridos/atualizados em hash_hab).
 */
void readPm(void* paths, void* hash_hab);

/*
 * readQry: processa o arquivo .qry, executa cada comando e gera o SVG e TXT de saida.
 * Comandos suportados: pq, rq, censo, h?, nasc, rip, mud, dspj.
 * entrada: paths — caminhos configurados; hash_quadras e hash_hab — hashfiles com os dados.
 * saida: nenhuma (SVG em bsdgeoqrysvg e TXT em bsdgeoqrytxt).
 */
void readQry(void* paths, void* hash_quadras, void* hash_hab);

#endif
