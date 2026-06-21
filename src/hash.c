#include "hash.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Algoritmo Hash djb2 criado por Dan Bernstein.
 * É excelente para strings, distribuindo bem os valores e reduzindo colisões.
 */
static unsigned long funcao_hash_djb2(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // equivale a: hash * 33 + c
    }
    return hash;
}

tabela_hash* criar_hash(size_t tamanho) {
    // Aloca a estrutura principal da tabela
    tabela_hash* tabela = (tabela_hash*)malloc(sizeof(tabela_hash));
    if (!tabela) return NULL;

    tabela->tamanho = tamanho;
    tabela->qtd_elementos = 0;

    // Aloca o vetor de ponteiros (baldes) inicializando todos com NULL (via calloc)
    tabela->baldes = (no**)calloc(tamanho, sizeof(no*));
    if (!tabela->baldes) {
        free(tabela); // Evita vazamento de memória se o calloc falhar
        return NULL;
    }

    return tabela;
}

void destruir_hash(tabela_hash* tabela) {
    if (!tabela) return;

    // Percorre todos os baldes da tabela
    for (size_t i = 0; i < tabela->tamanho; i++) {
        no* atual = tabela->baldes[i];
        
        // Libera a memória de cada nó na lista encadeada (encadeamento externo)
        while (atual != NULL) {
            no* temp = atual;
            atual = atual->proximo;
            free(temp);
        }
    }

    // Libera o vetor principal e a estrutura da tabela
    free(tabela->baldes);
    free(tabela);
}

bool inserir_hash(tabela_hash* tabela, const char* chave) {
    if (!tabela || !chave) return false;

    // Calcula o índice onde o elemento deve ser inserido
    unsigned long indice = funcao_hash_djb2(chave) % tabela->tamanho;

    // Passo 1: Verifica se o elemento já existe na lista encadeada desse índice
    no* atual = tabela->baldes[indice];
    while (atual != NULL) {
        if (strcmp(atual->chave, chave) == 0) {
            return false; // Usuário já está cadastrado, não insere duplicata
        }
        atual = atual->proximo;
    }

    // Passo 2: O elemento não existe, então criamos um novo nó
    no* novo_no = (no*)malloc(sizeof(no));
    if (!novo_no) return false;

    // Copia a string de forma segura limitando aos 11 caracteres previstos
    strncpy(novo_no->chave, chave, 11);
    novo_no->chave[11] = '\0'; // Garante o terminador nulo

    // Insere o novo nó no início da lista encadeada (operação O(1))
    novo_no->proximo = tabela->baldes[indice];
    tabela->baldes[indice] = novo_no;

    tabela->qtd_elementos++;
    return true;
}

bool buscar_hash(tabela_hash* tabela, const char* chave) {
    if (!tabela || !chave) return false;

    // Calcula o índice que o elemento deveria estar
    unsigned long indice = funcao_hash_djb2(chave) % tabela->tamanho;

    // Percorre a lista encadeada buscando pela chave exata
    no* atual = tabela->baldes[indice];
    while (atual != NULL) {
        if (strcmp(atual->chave, chave) == 0) {
            return true; // Encontrou o usuário (Match exato na Hash)
        }
        atual = atual->proximo;
    }

    return false; // Percorreu a lista e não encontrou (Possível falso positivo do Bloom)
}