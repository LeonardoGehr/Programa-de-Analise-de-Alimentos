#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_structures.h"
#include "file_format.h"

// -------------------- Estruturas locais --------------------

typedef struct NoAlimento {
    Alimento alimento;
    struct NoAlimento *prox;
} NoAlimento;

typedef struct NoCategoria {
    Categoria idCategoria;
    char nome[50];
    NoAlimento *listaAlimentos;
    struct NoCategoria *prox;
} NoCategoria;

// -------------------- Funções auxiliares --------------------

// Retorna o nome textual da categoria
const char* nomeCategoria(Categoria c) {
    switch (c) {
        case CEREAIS: return "Cereais e derivados";
        case VERDURAS: return "Verduras, hortaliças e derivados";
        case FRUTAS: return "Frutas e derivados";
        case GORDURAS: return "Gorduras e óleos";
        case PESCADOS: return "Pescados e frutos do mar";
        case CARNES: return "Carnes e derivados";
        case LACTEOS: return "Leite e derivados";
        case BEBIDAS: return "Bebidas (alcoólicas e não alcoólicas)";
        case OVOS: return "Ovos e derivados";
        case ACUCARES: return "Produtos açucarados";
        case MISCELANEAS: return "Miscelâneas";
        case INDUSTRIALIZADOS: return "Outros alimentos industrializados";
        case PREPARADOS: return "Alimentos preparados";
        case LEGUMINOSAS: return "Leguminosas e derivados";
        case NOZES: return "Nozes e sementes";
        default: return "Categoria desconhecida";
    }
}

// Insere categoria na lista em ordem alfabética
NoCategoria* inserirCategoriaOrdenada(NoCategoria **inicio, Categoria cat) {
    NoCategoria *novo = (NoCategoria*) malloc(sizeof(NoCategoria));
    novo->idCategoria = cat;
    strcpy(novo->nome, nomeCategoria(cat));
    novo->listaAlimentos = NULL;
    novo->prox = NULL;

    if (*inicio == NULL || strcmp(novo->nome, (*inicio)->nome) < 0) {
        novo->prox = *inicio;
        *inicio = novo;
        return novo;
    }

    NoCategoria *atual = *inicio;
    while (atual->prox && strcmp(novo->nome, atual->prox->nome) > 0)
        atual = atual->prox;

    novo->prox = atual->prox;
    atual->prox = novo;
    return novo;
}

// Busca uma categoria existente na lista
NoCategoria* buscarCategoria(NoCategoria *inicio, Categoria cat) {
    while (inicio) {
        if (inicio->idCategoria == cat)
            return inicio;
        inicio = inicio->prox;
    }
    return NULL;
}

// Lê dados do binário e cria a lista de categorias
NoCategoria* lerBinario(const char *nomeArquivo, int *totalLidos) {
    FILE *f = fopen(nomeArquivo, "rb");
    if (!f) {
        printf("Erro ao abrir o arquivo binário '%s'.\n", nomeArquivo);
        exit(1);
    }

    NoCategoria *listaCategorias = NULL;
    *totalLidos = 0;

    Alimento temp;
    while (fread(&temp, sizeof(Alimento), 1, f) == 1) {
        (*totalLidos)++;

        // Busca ou insere categoria
        NoCategoria *cat = buscarCategoria(listaCategorias, (Categoria)temp.categoria);
        if (!cat)
            cat = inserirCategoriaOrdenada(&listaCategorias, (Categoria)temp.categoria);

        // Cria nó de alimento
        NoAlimento *novoAlim = (NoAlimento*) malloc(sizeof(NoAlimento));
        novoAlim->alimento = temp;
        novoAlim->prox = cat->listaAlimentos;
        cat->listaAlimentos = novoAlim;
    }

    fclose(f);
    return listaCategorias;
}

// Lista todas as categorias
void listarCategorias(NoCategoria *inicio) {
    printf("\n--- Lista de Categorias ---\n");
    if (!inicio) {
        printf("(Nenhuma categoria carregada)\n");
        return;
    }

    int i = 1;
    while (inicio) {
        printf("%2d. %s\n", i++, inicio->nome);
        inicio = inicio->prox;
    }
}

// Libera toda memória alocada
void liberarMemoria(NoCategoria *inicio) {
    while (inicio) {
        NoCategoria *catTemp = inicio;
        inicio = inicio->prox;

        NoAlimento *alim = catTemp->listaAlimentos;
        while (alim) {
            NoAlimento *aTemp = alim;
            alim = alim->prox;
            free(aTemp);
        }
        free(catTemp);
    }
}

// -------------------- Menu principal --------------------

void exibirMenu() {
    printf("\n===== MENU PRINCIPAL =====\n");
    printf("1 - Listar categorias\n");
    printf("9 - Sair\n");
    printf("===========================\n");
    printf("Escolha uma opção: ");
}

// -------------------- Função main --------------------

int main() {
    NoCategoria *categorias = NULL;
    int total = 0;

    categorias = lerBinario("dados.bin", &total);
    printf("Arquivo carregado com sucesso! %d alimentos lidos.\n", total);

    int opcao;
    do {
        exibirMenu();
        if (scanf("%d", &opcao) != 1) {
            printf("Entrada inválida.\n");
            break;
        }

        switch (opcao) {
            case 1:
                listarCategorias(categorias);
                break;

            case 9:
                printf("Encerrando o programa...\n");
                break;

            default:
                printf("Opção inválida!\n");
                break;
        }
    } while (opcao != 9);

    liberarMemoria(categorias);
    return 0;
}
