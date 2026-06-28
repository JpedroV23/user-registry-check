#  Sistema de Verificação de Cadastro com Hash Table e Bloom Filter

Trabalho Final — Laboratório de Estruturas de Dados II
Implementação de um sistema eficiente de verificação de cadastro de usuários combinando **Tabela Hash** e **Filtro de Bloom**, com interface gráfica construída com [Raylib](https://www.raylib.com/).

---

##  Sobre o Projeto

Imagine uma empresa com milhões de usuários cadastrados que precisa responder rapidamente à pergunta:

> *"Este usuário já está cadastrado?"*

Consultar diretamente uma tabela hash para cada requisição funciona, mas pode ser custoso em escala. A solução implementada neste projeto utiliza um **Filtro de Bloom** como porteiro: ele descarta imediatamente os usuários que definitivamente **não** existem, evitando acessos desnecessários à estrutura principal.

O fluxo de consulta segue a lógica:

```
Consulta → Filtro de Bloom
              │
              ├─ "Definitivamente NÃO existe" → Retorna imediatamente ✅
              │
              └─ "Possivelmente existe" → Consulta Tabela Hash
                                              │
                                              ├─ Encontrado → Usuário existe ✅
                                              └─ Não encontrado → Falso Positivo do Bloom ⚠️
```

### Estruturas implementadas

| Estrutura | Função | Arquivo |
|---|---|---|
| Tabela Hash | Armazenamento e busca exata com encadeamento externo | `hash.c` / `hash.h` |
| Filtro de Bloom | Filtragem probabilística de existência com vetor de bits | `bloom.c` / `bloom.h` |
| Interface Gráfica | Painel interativo com Raylib para operações e métricas | `main.c` |

---

##  Estrutura do Projeto

```
projeto/
├── assets/
│   └── calibri.ttf # Fonte utilizada na interface gráfica
│
├── src/
│   ├── main.c        # Interface gráfica e lógica principal
│   ├── hash.c        # Implementação da Tabela Hash
│   ├── hash.h
│   ├── bloom.c       # Implementação do Filtro de Bloom
│   └── bloom.h
│
├── data/             # Gerado automaticamente pelo programa
│   ├── usuarios_1k.txt
│   ├── usuarios_10k.txt
│   └── usuarios_100k.txt
|
├── testes/
│   └── relatorio.pdf # Relatório dos resultados do projeto
|
└── README.md
```

---

## 🐧 Como Compilar e Executar no Linux

### Pré-requisitos

Você precisará do GCC e da biblioteca **Raylib** instalados.

**Instalando as dependências**
**Opção #1 (Mint / Algumas distribuições Debian):**

```bash
sudo apt update
```
```bash
sudo apt install build-essential libraylib-dev
```
**Opção #2 (Ubuntu)**
```bash
sudo apt update
sudo apt install -y build-essential git cmake libasound2-dev mesa-common-dev libx11-dev libxrandr-dev libxi-dev xorg-dev libgl1-mesa-dev libglu1-mesa-dev
```
```bash
git clone [https://github.com/raysan5/raylib.git](https://github.com/raysan5/raylib.git) raylib
cd raylib/src/
make PLATFORM=PLATFORM_DESKTOP
sudo make install
cd ../../
```

### Clonar repositório

```bash
git clone https://github.com/JpedroV23/user-registry-check.git
```

### Compilação

Após clonado, vá para o `src` do repositório
```bash
cd user-registry-check/src
```

Dentro da pasta `src/`, execute:

```bash
gcc main.c bloom.c hash.c -o sistema_bloom -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

### Execução

```bash
./sistema_bloom
```

> **Atenção:** execute o binário sempre a partir da pasta `src/`, pois o programa cria e lê os arquivos de dados no caminho relativo `../data/`.

---

##  Funcionalidades da Interface

O sistema conta com um painel gráfico interativo que centraliza todas as operações:

### 1. Gerar Arquivos
Cria automaticamente os três conjuntos de dados para os experimentos na pasta `data/`:
- `usuarios_1k.txt` — 1.000 usuários
- `usuarios_10k.txt` — 10.000 usuários
- `usuarios_100k.txt` — 100.000 usuários

Cada usuário segue o formato `[8 letras][3 números]`, por exemplo: `islaifda122`.

### 2. Executar Testes (RF03 — Estatísticas)
Roda os experimentos de carga nos três cenários e exibe uma tabela comparativa com:
- Tempo de busca **sem** Bloom Filter
- Tempo de busca **com** Bloom Filter
- Quantidade e percentual de falsos positivos

### 3. Ver Gráficos / Ver Tabela
Alterna a visualização dos resultados entre a tabela de dados e um gráfico de barras comparativo.

### Operações Unitárias

| Operação | Descrição |
|---|---|
| **Inserir (RF01)** | Insere um usuário digitado no Bloom Filter, na Tabela Hash e no arquivo selecionado (`1k`, `10k` ou `100k`) |
| **Consultar (RF02)** | Executa o fluxo completo de consulta: Bloom → Hash, exibindo o resultado e indicando se houve falso positivo |

---

##  Detalhes Técnicos

### Tabela Hash (`hash.c`)

- **Função hash:** algoritmo **djb2** de Dan Bernstein — excelente distribuição para strings
- **Tratamento de colisões:** encadeamento externo (lista encadeada por balde)
- **Tamanho:** 200.003 posições (número primo para minimizar colisões)
- **Chave:** strings de até 11 caracteres no formato `[8 letras][3 números]`

### Filtro de Bloom (`bloom.c`)

- **Vetor de bits** alocado dinamicamente, dimensionado pela fórmula:

$$m = -\frac{n \cdot \ln(p)}{(\ln 2)^2}$$

- **Número de funções hash** calculado como:

$$k = \frac{m}{n} \cdot \ln 2$$

- **Funções hash utilizadas:** combinação de **djb2** e **sdbm** com double hashing:

$$\text{pos}_i = (h_1 + i \cdot h_2) \mod m$$

- **Taxa de falsos positivos configurada:** 1% (`p = 0.01`)

---

##  Formato dos Dados de Entrada

Os arquivos de usuários seguem o formato: uma entrada por linha, com 11 caracteres (`8 letras minúsculas` + `3 dígitos`).

```
islaifda122
djskalsa297
fjkldsaf881
...
```

Você pode também carregar arquivos próprios, desde que respeitem esse formato.

---

## 📌 Observações

- Não foram utilizadas bibliotecas prontas de Hash Table ou Bloom Filter — todas as estruturas foram implementadas do zero em C.
- O programa lida com falsos positivos e os exibe explicitamente na interface, tanto nas consultas unitárias quanto nos experimentos em lote.
- A pasta `data/` é criada automaticamente caso não exista.
