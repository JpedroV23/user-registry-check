#include "bloom.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h> 
//funçoes base

//algoritmo de djb2 para fazer o espalhamento de strings
static unsigned long hash_djb2 (const char *str){
    unsigned long hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + *str; // hash*33 + caracter
        str++;
    } return hash;
}

//algoritmo de sdbm para a aleatoriedade
static unsigned long hash_sdbm (const char *str){
    unsigned long hash =0;
    while (*str){
        hash = *str + (hash << 6) + (hash << 16) - hash;
        str ++;
    } return hash;
}

//funçoes principais 

filtrobloom* criar (size_t n, double p){
    filtrobloom *filtro = (filtrobloom*) malloc(sizeof(filtrobloom));
    if (!filtro) return NULL; 
    //dimensionar vetor de bits
    double m = -((double)n * log(p))/ (log (2)*log(2));
    filtro -> tam_bits = (size_t) ceil (m);

    //dimensionar  quantidade de funçoes hash
    double k = ((double)filtro->tam_bits / (double)n) *log(2);
    filtro ->num_hash = (size_t) round (k);

    //seleção de ao menos 1 função para o funcionamento
    if (filtro -> num_hash ==0){
        filtro -> num_hash=1;
    }

//alocação do vetor de bits
size_t bytes_ne = (filtro ->tam_bits + 7) /8;
filtro ->v_bits = (unsigned char*) calloc (bytes_ne, sizeof(unsigned char));
    if (!filtro -> v_bits){
        free (filtro);
        return NULL;
    }
return filtro;
}

void destruir (filtrobloom * filtro){
    if(filtro != NULL){
        free (filtro -> v_bits);
        filtro -> v_bits = NULL;
        free (filtro);
        filtro = NULL;
    }
}

void inserir (filtrobloom * filtro, const char *texto){
    unsigned long h1 = hash_djb2 (texto);
    unsigned long h2 = hash_sdbm (texto);
//ligar k bits ao vetor
    for (size_t i=0; i<filtro->num_hash; i++){
        //double hashing
        size_t posicao_bit = (h1+i * h2) % filtro->tam_bits;

        //acha qual byte contem o bit e logo em seguida a posiçao exata dentro do byte
        size_t indice = posicao_bit/8;
        size_t resto = posicao_bit%8;

        //operação or com deslocamento p/ ligar apenas o bit especifico
        filtro -> v_bits[indice] |= (1<< resto);

    }
}

bool consultar (filtrobloom *filtro, const char *texto){
    unsigned long h1 = hash_djb2(texto);
    unsigned long h2 = hash_sdbm (texto);

    //verificação dos mesmos bits gerados na inserção
    for (size_t i =0; i< filtro ->num_hash;i++){
        size_t posicao_bit = (h1+i * h2) %filtro -> tam_bits;

        size_t indice = posicao_bit /8;
        size_t resto= posicao_bit%8;

        //operação and p/ checar se o bit é igual a 0
        if ((filtro->v_bits[indice] & (1 <<resto)) == 0){
            return false;
        }
    }
    //se o laço terminar sem zeros todos os bits estavam em 1 e ele possivelmente existe
    return true;
}
