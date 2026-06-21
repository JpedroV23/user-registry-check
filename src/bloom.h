#ifndef BLOOM_H
#define BLOOM_H
#include <stdbool.h>
#include <stdio.h>

//estrutura de dados + espaço de guardar o vetor de bits e dimensões

typedef struct {
    unsigned char *v_bits;
    size_t tam_bits; //m
    size_t num_hash; // k
} filtrobloom;

//criação e dimensionamento. com N: quantidades de elementos. P: probabilidade aceitavel
filtrobloom* criar (size_t n, double p);

//liberar a memoria
void destruir (filtrobloom*filtrado);

//inserir o identificador do usuario
void inserir (filtrobloom*filtro, const char *texto);

//consultar o filtro, localizar se ja foi inserido retornando o true;
bool consultar (filtrobloom * filtro, const char *texto);

#endif