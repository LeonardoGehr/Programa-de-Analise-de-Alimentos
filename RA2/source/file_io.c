#include "file_reading.h"
#include "file_format.h"
#include "data_structures.h" // Necessário para NoCategoria e a função inserirAlimento
#include "formatter.h"      // Funções utilitárias como removerQuebraLinha

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#define TAMANHO_LINHA 512

// --- Implementação das Funções P1 (CSV <-> BIN) ---

/* Lê e ignora a primeira linha (cabeçalho) do CSV */
void ignorarPrimeiraLinha(FILE *arquivo_csv) {
    char linha[TAMANHO_LINHA];
    if (fgets(linha, sizeof(linha), arquivo_csv) == NULL) {
        printf("Aviso: O arquivo CSV está vazio ou o cabeçalho não foi lido.\n");
    }
}

/* Converte CSV → BIN, retornando o número de registros salvos ou -1 em caso de erro. */
int lerCSVpraBIN(const char *arquivoCSV, const char *arquivoBIN) {
    Alimento alimento;
    int contador = 0;
    FILE *arquivo_csv = NULL, *arquivo_binario = NULL;
    char linha[TAMANHO_LINHA];
    const char *delimitador = ";";

    // 1. Abertura do CSV
    arquivo_csv = fopen(arquivoCSV, "r");
    if (arquivo_csv == NULL) {
        printf("Erro: Não foi possível abrir o arquivo %s\n", arquivoCSV);
        return -1;
    }
    ignorarPrimeiraLinha(arquivo_csv);

    // 2. Criação do BIN
    arquivo_binario = fopen(arquivoBIN, "wb");
    if (arquivo_binario == NULL) {
        printf("Erro: Não foi possível criar o arquivo %s\n", arquivoBIN);
        fclose(arquivo_csv);
        return -1;
    }

    // Loop: Leitura linha por linha
    while (fgets(linha, sizeof(linha), arquivo_csv) != NULL) {
        removerQuebraLinha(linha);
        
        char *token;
        int campo_atual = 0;
        int campos_lidos = 0; 
        
        // Zera a estrutura para garantir limpeza
        memset(&alimento, 0, sizeof(Alimento));
        alimento.categoria = CAT_INVALIDA; // Padrão
        alimento.codigo = 0; 

        // Usa uma cópia da linha para `strtok`
        char linha_copia[TAMANHO_LINHA];
        strncpy(linha_copia, linha, TAMANHO_LINHA - 1);
        linha_copia[TAMANHO_LINHA - 1] = '\0';

        char *token_csv = strtok(linha_copia, delimitador);

        // Loop: Tokenização da linha
        while (token_csv) {
            
            // Cria uma cópia temporária do token para manipulação, preservando o original se necessário
            char token_temp[TAMANHO_LINHA];
            strncpy(token_temp, token_csv, TAMANHO_LINHA - 1);
            token_temp[TAMANHO_LINHA - 1] = '\0';

            removerQuebraLinha(token_temp);
            removerAspas(token_temp);

            if (campo_atual == 0) { // CÓDIGO
                alimento.codigo = atoi(token_temp);
                if (alimento.codigo > 0) campos_lidos++;
            } else if (campo_atual == 1) { // NOME
                strncpy(alimento.nome, token_temp, sizeof(alimento.nome) - 1);
                alimento.nome[sizeof(alimento.nome) - 1] = '\0';
                campos_lidos++;
            } else if (campo_atual == 2) { // CALORIAS (ENERGIA)
                substituirVirgula(token_temp);
                alimento.calorias = atof(token_temp);
                campos_lidos++;
            } else if (campo_atual == 3) { // PROTEÍNA
                substituirVirgula(token_temp);
                alimento.proteinas = atof(token_temp);
                campos_lidos++;
            } else if (campo_atual == 4) { // CATEGORIA
                // Assume que o 5º campo do CSV é o nome da categoria.
                alimento.categoria = nomeParaCategoria(token_temp);
                if (alimento.categoria != CAT_INVALIDA) {
                    campos_lidos++;
                }
            }
            
            token_csv = strtok(NULL, delimitador);
            campo_atual++;
        }

        // Condicional: Se leu pelo menos 4 campos principais (ID, Nome, Calorias, Proteinas)
        if (campos_lidos >= 4 && alimento.codigo > 0) {
            if (fwrite(&alimento, sizeof(Alimento), 1, arquivo_binario) != 1) {
                printf("Erro ao escrever dados no arquivo binário\n");
                break;
            }
            contador++;
        }
    }
    
    fclose(arquivo_csv);
    fclose(arquivo_binario);
    
    return contador;
}

/* Verifica o conteúdo do arquivo binário gerado (Função P1) */
void verificarArquivoBinario(const char* arquivoBinario) {
    FILE *arquivo = fopen(arquivoBinario, "rb");
    if (arquivo == NULL) {
        printf("Erro: Não foi possível abrir o arquivo binário %s\n", arquivoBinario);
        return;
    }

    Alimento alimento;
    int contador = 0;

    printf("\n=== Amostra do Conteúdo do arquivo binário ===\n");
    printf("+------+-------------------------------------+---------+----------+--------------------------------------+\n");
    printf("| Cód. | Nome                                | Energia | Proteína | Categoria                            |\n");
    printf("+------+-------------------------------------+---------+----------+--------------------------------------+\n");

    // Lê e verifica o conteúdo (limita a 10 para amostra)
    while (fread(&alimento, sizeof(Alimento), 1, arquivo) == 1 && contador < 10) { 
        printf("| %4d | %-35.35s | %7.2f | %8.2f | %-36.36s |\n",
               alimento.codigo, alimento.nome, alimento.calorias, alimento.proteinas, obterNomeCategoria(alimento.categoria));
        contador++;
    }

    printf("+------+-------------------------------------+---------+----------+--------------------------------------+\n");
    
    // Contagem total
    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    long total_registros = tamanho / sizeof(Alimento);
    
    if (total_registros > 10) {
        printf("\nTotal de registros salvos: %ld (Apenas 10 foram mostrados acima).\n", total_registros);
    } else {
        printf("Total de registros lidos: %d\n", contador);
    }
    
    fclose(arquivo);
}

// --- Implementação das Funções P2 (BIN <-> Memória) ---

/* Lê o arquivo BIN e popula a Lista Ligada de Categorias na memória (Requisito C.a) */
int lerBINparaMemoria(const char *arquivoBIN, NoCategoria **lista_categorias) {
    FILE *arquivo = fopen(arquivoBIN, "rb");
    if (arquivo == NULL) {
        printf("Erro ao abrir %s para leitura em memória. Certifique-se de que o P1 foi executado.\n", arquivoBIN);
        return 0; // 0 alimentos lidos
    }

    Alimento alimento_lido;
    int contador = 0;
    
    // Loop: lê cada registro do arquivo binário
    while (fread(&alimento_lido, sizeof(Alimento), 1, arquivo) == 1) {
        // Insere o alimento na estrutura de memória 
        if (inserirAlimento(lista_categorias, &alimento_lido)) {
            contador++;
        } else {
            // Nota: Se inserirAlimento falhar, pode ser por alocação ou ID duplicado.
        }
    }

    fclose(arquivo);
    return contador;
}

/* Salva o conteúdo da Lista Ligada de Categorias para o arquivo BIN (Requisito C.c.9) */
int escreverMemoriaParaBIN(const char *arquivoBIN, NoCategoria *lista_categorias) {
    FILE *arquivo = fopen(arquivoBIN, "wb");
    if (arquivo == NULL) {
        printf("Erro: Não foi possível criar/abrir o arquivo binário para escrita: %s\n", arquivoBIN);
        return -1;
    }

    NoCategoria *cat_atual = lista_categorias;
    int contador = 0;

    // Loop externo: percorre a lista de categorias
    while (cat_atual != NULL) {
        NoAlimento *ali_atual = cat_atual->alimentos;
        // Loop interno: percorre a lista de alimentos de cada categoria
        while (ali_atual != NULL) {
            // Escreve os dados brutos do alimento (o que está na struct Alimento)
            if (fwrite(&(ali_atual->dados), sizeof(Alimento), 1, arquivo) != 1) {
                printf("Erro ao escrever alimento %s no arquivo binário\n", ali_atual->dados.nome);
                fclose(arquivo);
                return -1;
            }
            contador++;
            ali_atual = ali_atual->proximo;
        }
        cat_atual = cat_atual->proximo;
    }

    fclose(arquivo);
    return contador;
}