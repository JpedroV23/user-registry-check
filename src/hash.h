#ifndef HASH_H
#define HASH_H

#include <stdbool.h>
#include <stddef.h>

/* * Estrutura do nó para a lista encadeada (Encadeamento Externo).
 * Como o formato exigido é [8caractere] + [3numeros], temos 11 caracteres.
 * Alocamos 12 espaços para incluir o caractere nulo '\0' finalizador de string em C.
 */
typedef struct no {
    char chave[12]; 
    struct no* proximo; // Ponteiro para o próximo nó em caso de colisão
} no;

/* * Estrutura principal da Tabela Hash.
 */
typedef struct {
    no** baldes;          // Vetor de ponteiros (os "buckets" da tabela)
    size_t tamanho;       // Quantidade de posições na tabela
    size_t qtd_elementos; // Quantidade atual de usuários cadastrados
} tabela_hash;

// Protótipos das funções

// Cria e dimensiona a tabela hash na memória
tabela_hash* criar_hash(size_t tamanho);

// Libera toda a memória alocada para a tabela e suas listas
void destruir_hash(tabela_hash* tabela);

// Insere um novo usuário. Retorna 'true' se inserido com sucesso, ou 'false' se já existir
bool inserir_hash(tabela_hash* tabela, const char* chave);

// Busca por um usuário. Retorna 'true' se encontrado, ou 'false' se inexistente
bool buscar_hash(tabela_hash* tabela, const char* chave);

#endif // HASH_H