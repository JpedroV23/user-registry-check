# Sistema de Consulta Eficiente: Tabela Hash & Filtro de Bloom

Este repositório contém o projeto final desenvolvido para a disciplina de **Laboratório de Algoritmo e Estrutura de Dados II (LAED II)** da **Universidade Federal Rural do Semi-Árido (UFERSA)**. O objetivo do projeto é projetar, implementar e analisar o impacto de desempenho, consumo de memória e taxa de falsos positivos ao combinar uma estrutura de dados probabilística (Filtro de Bloom) com uma estrutura de armazenamento exato (Tabela Hash) para a verificação de cadastro de usuários em larga escala.

---

## Sobre o Projeto

O sistema simula um ambiente corporativo de verificação de usuários onde milhões de registros precisam ser consultados rapidamente. Para otimizar o tempo de pesquisa e evitar acessos desnecessários à estrutura principal (Tabela Hash), utiliza-se um **Filtro de Bloom** como uma camada de verificação prévia.

### Fluxo Obrigatório de Consulta

Sempre que uma consulta por um identificador único de usuário é realizada, o sistema segue rigorosamente o seguinte fluxo lógico:
1. **Consulta o Filtro de Bloom:**
   - Se o filtro indicar que o usuário *"definitivamente não existe"*, o sistema retorna o resultado imediatamente sem acessar a Tabela Hash.
   - Se o filtro indicar que o usuário *"possivelmente existe"*, o sistema avança para o passo seguinte.
2. **Consulta a Tabela Hash:**
   - Realiza a busca exata para confirmar a presença do usuário.
   - Se o usuário for encontrado na Hash: retorna **Usuário Encontrado**.
   - Se o usuário não for encontrado na Hash: contabiliza um **Falso Positivo** gerado pelo Filtro de Bloom.

---

## Arquitetura e Estrutura do Código

O projeto foi estritamente desenvolvido na **Linguagem C**, sem a utilização de bibliotecas prontas para as estruturas de dados, garantindo o controle total sobre a alocação de memória e implementação manual dos algoritmos de hashing.

```text
projeto/
├── data/
│   └── usuarios.txt       # Arquivos de texto para carga de dados e testes em lote
├── src/
│   ├── hash.c / hash.h    # Implementação manual da Tabela Hash e tratamento de colisões
│   ├── bloom.c / bloom.h  # Implementação manual do Filtro de Bloom (vetor de bits e hashes)
│   └── main.c             # Integração do sistema, menu interativo, leitura de arquivos e métricas
├── testes/
│   └── relatorio.pdf      # Relatório científico com a análise experimental dos cenários
└── README.md              # Documentação de compilação e utilização
