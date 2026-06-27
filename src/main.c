#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h> // Para o mkdir funcionar de forma nativa no Linux
#include <raylib.h>
#include "bloom.h"
#include "hash.h"

// Configurações do experimento e do sistema interativo
#define TAMANHO_HASH 200003 
#define BLOOM_ERRO 0.01 

// Estrutura para armazenar os resultados dos testes de carga
typedef struct {
    int cenario;
    double tempo_sem;
    double tempo_com;
    int fp_qtd;
    double fp_pct;
} ResultadoExperimento;

ResultadoExperimento resultados[3];
bool resultados_prontos = false;
bool mostrar_aviso_geracao = false;
float tempo_aviso = 0.0f;

// --- ESTRUTURAS PERSISTENTES PARA RF01 E RF02 ---
tabela_hash* sistema_hash = NULL;
filtrobloom* sistema_bloom = NULL;
char input_usuario[12] = "\0"; 
int total_letras = 0;
char mensagem_status[100] = "Inicializando sistema...";

// Variável de controle para seleção do arquivo alvo no RF01 (0 = 1k, 1 = 10k, 2 = 100k)
int arquivo_alvo_selecionado = 0; 

// Cores globais do tema visual
Color bgCustom = (Color){ 30, 30, 30, 255 };
Color btnNormal = (Color){ 70, 70, 70, 255 };
Color btnHover = (Color){ 1, 77, 78, 255 }; // Cor padrão de destaque (Verde suave)
Color corBarraSem = (Color){ 200, 80, 80, 255 }; 
Color corBarraCom = (Color){ 80, 200, 80, 255 }; 

// Fonte Customizada Global
Font fonteGlobal;

// --- FUNÇÃO AUXILIAR DE RENDERIZAÇÃO DE TEXTO ---
void DesenharTextoCustom(const char *texto, float posX, float posY, float tamanho, Color cor) {
    DrawTextEx(fonteGlobal, texto, (Vector2){posX, posY}, tamanho, 1.0f, cor);
}

// --- FUNÇÕES DE GERAÇÃO DE DADOS ---
void gerar_nome_aleatorio(char *buffer) {
    char letras[] = "abcdefghijklmnopqrstuvwxyz";
    char numeros[] = "0123456789";
    for (int i = 0; i < 8; i++) {
        buffer[i] = letras[rand() % 26];
    }
    for (int i = 8; i < 11; i++) {
        buffer[i] = numeros[rand() % 10];
    }
    buffer[11] = '\0';
}

void criar_arquivo_teste(const char *nome_arquivo, int quantidade) {
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) return;

    char usuario[12];
    for (int i = 0; i < quantidade; i++) {
        gerar_nome_aleatorio(usuario);
        fprintf(arquivo, "%s\n", usuario);
    }
    fclose(arquivo);
}

void gerar_todos_arquivos() {
    mkdir("../data", 0777);
    criar_arquivo_teste("../data/usuarios_1k.txt", 1000);
    criar_arquivo_teste("../data/usuarios_10k.txt", 10000);
    criar_arquivo_teste("../data/usuarios_100k.txt", 100000);
}

// --- FUNÇÃO PARA SINCROZINAR O ARQUIVO SELECIONADO COM A MEMÓRIA DE BUSCA (RF01/RF02) ---
void carregar_dados_sistema(int idx) {
    int cenarios[] = {1000, 10000, 100000};
    char arquivos[][30] = {"../data/usuarios_1k.txt", "../data/usuarios_10k.txt", "../data/usuarios_100k.txt"};
    const char* nomes_arquivos[] = {"1k", "10k", "100k"};

    if (sistema_hash != NULL) destruir_hash(sistema_hash);
    if (sistema_bloom != NULL) destruir(sistema_bloom);

    sistema_hash = criar_hash(TAMANHO_HASH);
    sistema_bloom = criar(cenarios[idx] + 2000, BLOOM_ERRO);

    FILE* f = fopen(arquivos[idx], "r");
    if (!f) {
        mkdir("../data", 0777);
        criar_arquivo_teste(arquivos[idx], cenarios[idx]);
        f = fopen(arquivos[idx], "r");
    }

    if (f) {
        char usuario[12];
        while (fscanf(f, "%11s", usuario) == 1) {
            inserir(sistema_bloom, usuario);
            inserir_hash(sistema_hash, usuario);
        }
        fclose(f);
        strcpy(mensagem_status, TextFormat("Memoria sincronizada com o arquivo_%s!", nomes_arquivos[idx]));
    } else {
        strcpy(mensagem_status, "Erro de E/S ao carregar dados na memoria.");
    }
}

// --- FUNÇÃO DO EXPERIMENTO ---
void executar_experimentos() {
    int cenarios[] = {1000, 10000, 100000};
    char arquivos[][30] = {"../data/usuarios_1k.txt", "../data/usuarios_10k.txt", "../data/usuarios_100k.txt"};
    
    mkdir("../data", 0777);

    for (int i = 0; i < 3; i++) {
        tabela_hash* h = criar_hash(TAMANHO_HASH);
        filtrobloom* b = criar(cenarios[i], BLOOM_ERRO);
        
        FILE* f = fopen(arquivos[i], "r");
        if (!f) {
            criar_arquivo_teste(arquivos[i], cenarios[i]);
            f = fopen(arquivos[i], "r");
        }

        char usuario[12];
        char (*array_teste)[12] = malloc(cenarios[i] * sizeof(*array_teste));
        int idx = 0;
        
        while (fscanf(f, "%11s", usuario) == 1 && idx < cenarios[i]) {
            inserir(b, usuario);
            inserir_hash(h, usuario);
            strcpy(array_teste[idx], usuario);
            idx++;
        }
        fclose(f);

        for(int j = 0; j < cenarios[i] / 2; j++) {
            array_teste[j][0] = 'Z'; 
            array_teste[j][1] = 'Z';
        }

        clock_t inicio_sem = clock();
        for (int j = 0; j < cenarios[i]; j++) {
            buscar_hash(h, array_teste[j]);
        }
        clock_t fim_sem = clock();
        double tempo_sem = (double)(fim_sem - inicio_sem) / CLOCKS_PER_SEC;

        int fp_local = 0;
        clock_t inicio_com = clock();
        for (int j = 0; j < cenarios[i]; j++) {
            if (consultar(b, array_teste[j])) { 
                if (!buscar_hash(h, array_teste[j])) { 
                    fp_local++;
                }
            }
        }
        clock_t fim_com = clock();
        double tempo_com = (double)(fim_com - inicio_com) / CLOCKS_PER_SEC;

        double fp_pct = ((double)fp_local / (cenarios[i] / 2.0)) * 100.0;

        resultados[i].cenario = cenarios[i];
        resultados[i].tempo_sem = tempo_sem;
        resultados[i].tempo_com = tempo_com;
        resultados[i].fp_qtd = fp_local;
        resultados[i].fp_pct = fp_pct;

        free(array_teste);
        destruir_hash(h);
        destruir(b);
    }
    resultados_prontos = true;
}

// --- INTERFACE OPERACIONAL RAYLIB ---
int main() {
    srand(time(NULL));

    // Nova resolução ajustada conforme solicitado (+/- 1050 por 600)
    const int screenWidth = 1050;
    const int screenHeight = 600; 
    InitWindow(screenWidth, screenHeight, "Simulador: Bloom Filter vs Hash Table");
    SetTargetFPS(60);

    fonteGlobal = LoadFontEx("../assets/calibri.ttf", 252, 0, 250);

    carregar_dados_sistema(arquivo_alvo_selecionado);

    // Ajuste fino do posicionamento dos botões superiores (centralizados na largura 1050)
    Rectangle btnGerar = { 315, 65, 200, 35 };
    Rectangle btnRodar = { 535, 65, 200, 35 };
    
    // Ajuste fino das operações unitárias
    Rectangle caixaTexto = { 40, 155, 180, 35 };
    Rectangle btnRadio1k = { 310, 157, 45, 30 };
    Rectangle btnRadio10k = { 365, 157, 55, 30 };
    Rectangle btnRadio100k = { 430, 157, 65, 30 };

    Rectangle btnInserir = { 515, 155, 85, 35 };
    Rectangle btnConsultar = { 610, 155, 95, 35 };
    
    while (!WindowShouldClose()) {
        Vector2 mousePoint = GetMousePosition();
        
        bool hoverGerar = CheckCollisionPointRec(mousePoint, btnGerar);
        bool hoverRodar = CheckCollisionPointRec(mousePoint, btnRodar);
        
        bool hoverTexto = CheckCollisionPointRec(mousePoint, caixaTexto);
        bool hoverInserir = CheckCollisionPointRec(mousePoint, btnInserir);
        bool hoverConsultar = CheckCollisionPointRec(mousePoint, btnConsultar);

        bool hover1k = CheckCollisionPointRec(mousePoint, btnRadio1k);
        bool hover10k = CheckCollisionPointRec(mousePoint, btnRadio10k);
        bool hover100k = CheckCollisionPointRec(mousePoint, btnRadio100k);

        if (hover1k && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && arquivo_alvo_selecionado != 0) {
            arquivo_alvo_selecionado = 0;
            carregar_dados_sistema(0);
        }
        if (hover10k && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && arquivo_alvo_selecionado != 1) {
            arquivo_alvo_selecionado = 1;
            carregar_dados_sistema(1);
        }
        if (hover100k && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && arquivo_alvo_selecionado != 2) {
            arquivo_alvo_selecionado = 2;
            carregar_dados_sistema(2);
        }

        if (hoverGerar && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            gerar_todos_arquivos();
            carregar_dados_sistema(arquivo_alvo_selecionado); 
            mostrar_aviso_geracao = true;
            tempo_aviso = 3.0f; 
        }
        if (hoverRodar && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            executar_experimentos();
        }

        if (hoverTexto) {
            SetMouseCursor(MOUSE_CURSOR_IBEAM);
            int caractere = GetCharPressed();
            while (caractere > 0) {
                if ((caractere >= 32) && (caractere <= 125) && (total_letras < 11)) {
                    input_usuario[total_letras] = (char)caractere;
                    input_usuario[total_letras + 1] = '\0';
                    total_letras++;
                }
                caractere = GetCharPressed();
            }

            if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_V)) {
                const char* textoAreaTransferencia = GetClipboardText();
                if (textoAreaTransferencia != NULL) {
                    int tamanho_colagem = strlen(textoAreaTransferencia);
                    for (int i = 0; i < tamanho_colagem && total_letras < 11; i++) {
                        if (textoAreaTransferencia[i] >= 33 && textoAreaTransferencia[i] <= 125) {
                            input_usuario[total_letras] = textoAreaTransferencia[i];
                            total_letras++;
                        }
                    }
                    input_usuario[total_letras] = '\0';
                }
            }

            if (IsKeyPressed(KEY_BACKSPACE)) {
                total_letras--;
                if (total_letras < 0) total_letras = 0;
                input_usuario[total_letras] = '\0';
            }
        } else {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }

        if (hoverInserir && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && total_letras > 0) {
            bool inserido_memoria = inserir_hash(sistema_hash, input_usuario);
            if (inserido_memoria) {
                inserir(sistema_bloom, input_usuario);
                const char* caminhos[] = {"../data/usuarios_1k.txt", "../data/usuarios_10k.txt", "../data/usuarios_100k.txt"};
                const char* nomes_arquivos[] = {"1k", "10k", "100k"};
                
                FILE* arq = fopen(caminhos[arquivo_alvo_selecionado], "a");
                if (arq != NULL) {
                    fprintf(arq, "%s\n", input_usuario);
                    fclose(arq);
                    strcpy(mensagem_status, TextFormat("RF01: Salvo no arquivo_%s!", nomes_arquivos[arquivo_alvo_selecionado]));
                } else {
                    strcpy(mensagem_status, "Erro de E/S: Diretorio 'data/' nao encontrado.");
                }
            } else {
                strcpy(mensagem_status, "Erro, usuario ja existente no sistema!");
            }
            input_usuario[0] = '\0';
            total_letras = 0;
        }

        if (hoverConsultar && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && total_letras > 0) {
            if (!consultar(sistema_bloom, input_usuario)) {
                strcpy(mensagem_status, TextFormat("RF02: '%s' NAO existe. (Barrado pelo Filtro)", input_usuario));
            } else {
                if (buscar_hash(sistema_hash, input_usuario)) {
                    strcpy(mensagem_status, TextFormat("RF02: '%s' ENCONTRADO na Hash!", input_usuario));
                } else {
                    strcpy(mensagem_status, TextFormat("RF02: '%s' NAO existe. (Falso Positivo!)", input_usuario));
                }
            }
        }

        if (mostrar_aviso_geracao) {
            tempo_aviso -= GetFrameTime();
            if (tempo_aviso <= 0.0f) mostrar_aviso_geracao = false;
        }

        // --- RENDERIZAÇÃO GRÁFICA ---
        BeginDrawing();
        ClearBackground(bgCustom);

        DesenharTextoCustom("PAINEL DE ANÁLISE: HASH E BLOOM FILTER", screenWidth/2 - 220, 25, 22, LIGHTGRAY);

        // Renderização dos Botões do Menu Superior
        DrawRectangleRec(btnGerar, hoverGerar ? btnHover : btnNormal);
        DesenharTextoCustom("1. Gerar Arquivos", btnGerar.x + 20, btnGerar.y + 9, 16, WHITE);

        DrawRectangleRec(btnRodar, hoverRodar ? btnHover : btnNormal);
        DesenharTextoCustom("2. Executar Testes", btnRodar.x + 20, btnRodar.y + 9, 16, WHITE);

        if (mostrar_aviso_geracao) {
            DesenharTextoCustom("Arquivos gerados em '../data/'!", screenWidth/2 - 110, 110, 15, btnHover);
        }

        // --- SEÇÃO INTERATIVA REORGANIZADA ---
        DrawLine(40, 130, screenWidth - 40, 130, btnHover); 
        DesenharTextoCustom("OPERAÇÕES UNITÁRIAS (RF01 e RF02)", 40, 135, 16, LIGHTGRAY);

        // Caixa de texto ID
        DrawRectangleRec(caixaTexto, hoverTexto ? Fade(btnHover, 0.2f) : DARKGRAY);
        DrawRectangleLines((int)caixaTexto.x, (int)caixaTexto.y, (int)caixaTexto.width, (int)caixaTexto.height, hoverTexto ? btnHover : GRAY);
        
        if (total_letras == 0) {
            DesenharTextoCustom("Inserir ID...", caixaTexto.x + 10, caixaTexto.y + 10, 15, GRAY);
        } else {
            DesenharTextoCustom(input_usuario, caixaTexto.x + 10, caixaTexto.y + 10, 16, WHITE);
        }

        // Seletor Dinâmico
        DesenharTextoCustom("Destino:", 240, 163, 15, LIGHTGRAY);
        
        DrawRectangleRec(btnRadio1k, arquivo_alvo_selecionado == 0 ? btnHover : (hover1k ? Fade(btnNormal, 1.5f) : btnNormal));
        DesenharTextoCustom("1k", btnRadio1k.x + 13, btnRadio1k.y + 7, 14, WHITE);
        
        DrawRectangleRec(btnRadio10k, arquivo_alvo_selecionado == 1 ? btnHover : (hover10k ? Fade(btnNormal, 1.5f) : btnNormal));
        DesenharTextoCustom("10k", btnRadio10k.x + 13, btnRadio10k.y + 7, 14, WHITE);
        
        DrawRectangleRec(btnRadio100k, arquivo_alvo_selecionado == 2 ? btnHover : (hover100k ? Fade(btnNormal, 1.5f) : btnNormal));
        DesenharTextoCustom("100k", btnRadio100k.x + 13, btnRadio100k.y + 7, 14, WHITE);

        // Botões de Ação
        DrawRectangleRec(btnInserir, hoverInserir ? btnHover : btnNormal);
        DesenharTextoCustom("Inserir", btnInserir.x + 16, btnInserir.y + 9, 15, WHITE);

        DrawRectangleRec(btnConsultar, hoverConsultar ? btnHover : btnNormal);
        DesenharTextoCustom("Consultar", btnConsultar.x + 12, btnConsultar.y + 9, 15, WHITE);

        // Feedback
        DesenharTextoCustom(mensagem_status, 40, 205, 15, btnHover);
        DrawLine(40, 230, screenWidth - 40, 230, btnHover); 

        // --- EXIBIÇÃO DE PERFORMANCE (LADO A LADO) ---
        if (resultados_prontos) {
            
            // ---------------- ESQUERDA: TABELA REAJUSTADA ---------------- //
            int tableX = 40;
            int tableY = 255;
            DrawRectangle(tableX, tableY, 480, 150, Fade(BLACK, 0.4f));
            
            DesenharTextoCustom("Cenário", tableX + 10, tableY + 15, 15, btnHover);
            DesenharTextoCustom("S/ Bloom", tableX + 110, tableY + 15, 15, btnHover);
            DesenharTextoCustom("C/ Bloom", tableX + 220, tableY + 15, 15, btnHover);
            DesenharTextoCustom("Falsos Pos.", tableX + 340, tableY + 15, 15, btnHover);
            
            DrawLine(tableX, tableY + 40, tableX + 480, tableY + 40, btnHover);

            for (int i = 0; i < 3; i++) {
                int rowY = tableY + 55 + (i * 30);
                DesenharTextoCustom(TextFormat("%d", resultados[i].cenario), tableX + 10, rowY, 15, WHITE);
                DesenharTextoCustom(TextFormat("%.4f s", resultados[i].tempo_sem), tableX + 110, rowY, 15, WHITE);
                DesenharTextoCustom(TextFormat("%.4f s", resultados[i].tempo_com), tableX + 220, rowY, 15, WHITE);
                DesenharTextoCustom(TextFormat("%d (%.2f%%)", resultados[i].fp_qtd, resultados[i].fp_pct), tableX + 340, rowY, 15, WHITE);
            }

            // ---------------- DIREITA: GRÁFICO REAJUSTADO ---------------- //
            int graphX = 560;
            int baseY = 420; 
            int maxBarHeight = 130;
            
            double max_tempo = 0;
            for (int i = 0; i < 3; i++) {
                if (resultados[i].tempo_sem > max_tempo) max_tempo = resultados[i].tempo_sem;
                if (resultados[i].tempo_com > max_tempo) max_tempo = resultados[i].tempo_com;
            }

            DesenharTextoCustom("Tempo de Busca (Menor e melhor)", graphX, 255, 15, LIGHTGRAY);

            for (int i = 0; i < 3; i++) {
                int heightSem = max_tempo > 0 ? (int)((resultados[i].tempo_sem / max_tempo) * maxBarHeight) : 2;
                int heightCom = max_tempo > 0 ? (int)((resultados[i].tempo_com / max_tempo) * maxBarHeight) : 2;

                if (heightSem < 2) heightSem = 2;
                if (heightCom < 2) heightCom = 2;

                int posX = graphX + 20 + (i * 140);

                DrawRectangle(posX, baseY - heightSem, 35, heightSem, corBarraSem);
                DrawRectangle(posX + 40, baseY - heightCom, 35, heightCom, corBarraCom);

                DesenharTextoCustom(TextFormat("%d itens", resultados[i].cenario), posX + 5, baseY + 12, 14, WHITE);
            }

            // Legenda do gráfico centralizada horizontalmente na direita
            DrawRectangle(graphX + 40, 480, 15, 15, corBarraSem);
            DesenharTextoCustom("Apenas Hash", graphX + 65, 480, 14, WHITE);
            DrawRectangle(graphX + 200, 480, 15, 15, corBarraCom);
            DesenharTextoCustom("Bloom + Hash", graphX + 225, 480, 14, WHITE);

        } else {
            DesenharTextoCustom("Aguardando execucao dos experimentos em massa...", screenWidth/2 - 200, 380, 16, GRAY);
        }

        EndDrawing();
    }

    UnloadFont(fonteGlobal);
    if (sistema_hash != NULL) destruir_hash(sistema_hash);
    if (sistema_bloom != NULL) destruir(sistema_bloom);

    CloseWindow();
    return 0;
}