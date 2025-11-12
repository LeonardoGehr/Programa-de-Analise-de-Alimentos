#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/data_structures.h"
#include "../include/file_format.h"

#define PATH_DADOS "source/dados.bin"

static int category_counts[NUM_CATEGORIAS + 1]; // 1..15

/* Converte string para minúsculas (em buffer destino) */
static void to_lower_copy(char *dst, const char *src, size_t dst_size) {
    if (dst_size == 0) return;
    size_t i;
    for (i = 0; i + 1 < dst_size && src[i] != '\0'; i++) {
        dst[i] = (char) tolower((unsigned char) src[i]);
    }
    dst[i] = '\0';
}

/* Função de mapeamento de nome textual para Categoria.
   Usa heurísticas por substring (baseada no código que você já tinha). */
static Categoria mapNomeParaCategoria(const char *nome) {
    char tmp[256];
    to_lower_copy(tmp, nome ? nome : "", sizeof(tmp));

    if (strstr(tmp, "cereais")) return CEREAIS;
    if (strstr(tmp, "verdura") || strstr(tmp, "hortali") || strstr(tmp, "hortal")) return VERDURAS;
    if (strstr(tmp, "fruta") || strstr(tmp, "maca") || strstr(tmp, "banana") || strstr(tmp, "laranja") ) return FRUTAS;
    if (strstr(tmp, "gordura") || strstr(tmp, "óle") || strstr(tmp, "oleo") || strstr(tmp, "óleo")) return GORDURAS;
    if (strstr(tmp, "pescad") || strstr(tmp, "atum") || strstr(tmp, "salm") || strstr(tmp, "sard")) return PESCADOS;
    if (strstr(tmp, "carne") || strstr(tmp, "bovina") || strstr(tmp, "suína") || strstr(tmp, "frango")) return CARNES;
    if (strstr(tmp, "leite") || strstr(tmp, "queijo") || strstr(tmp, "iogurte") || strstr(tmp, "lacteo") || strstr(tmp, "laticínio")) return LACTEOS;
    if (strstr(tmp, "suco") || strstr(tmp, "cerveja") || strstr(tmp, "vinho") || strstr(tmp, "água") || strstr(tmp, "refrigerante") || strstr(tmp, "bebid")) return BEBIDAS;
    if (strstr(tmp, "ovo") || strstr(tmp, "ovos")) return OVOS;
    if (strstr(tmp, "açúcar") || strstr(tmp, "acucar") || strstr(tmp, "doce") || strstr(tmp, "açucarado")) return ACUCARES;
    if (strstr(tmp, "miscel") || strstr(tmp, "miscelâ") || strstr(tmp, "miscela")) return MISCELANEAS;
    if (strstr(tmp, "industrial") || strstr(tmp, "processado") || strstr(tmp, "industrializado")) return INDUSTRIALIZADOS;
    if (strstr(tmp, "preparad") || strstr(tmp, "cozinh") || strstr(tmp, "preparado")) return PREPARADOS;
    if (strstr(tmp, "feijão") || strstr(tmp, "feijao") || strstr(tmp, "lentilha") || strstr(tmp, "soja") || strstr(tmp, "grão") || strstr(tmp, "legumin")) return LEGUMINOSAS;
    if (strstr(tmp, "noz") || strstr(tmp, "castanha") || strstr(tmp, "sement")) return SEMENTES;

    /* Se não reconhecer, devolve CEREAIS como no seu código anterior (padrão) */
    return CEREAIS;
}

/* Nome legível (usa função do seu header, caso queira trocar; mantive redundância segura) */
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
        case SEMENTES: return "Nozes e sementes";
        default: return "Categoria desconhecida";
    }
}

/* Lista todas as categorias com a quantidade correspondente */
void listarCategoriasComContagem(void) {
    printf("\n--- Lista de Categorias (quantidade) ---\n");
    for (int i = CEREAIS; i <= SEMENTES; i++) {
        printf("%2d. %-40s : %3d\n", i, nomeCategoria(i), category_counts[i]);
    }
}

/* Lê o binário e preenche os contadores de categoria. Retorna total lido ou -1 em erro. */
int lerBinarioContandoCategorias(const char *caminho) {
    FILE *fp = fopen(caminho, "rb");
    if (!fp) {
        printf("❌ Erro: não foi possível abrir o arquivo '%s'.\n", caminho);
        return -1;
    }

    /* inicializa contadores */
    for (int i = 0; i <= NUM_CATEGORIAS; i++) category_counts[i] = 0;

    Alimento temp;
    int total = 0;

    while (fread(&temp, sizeof(Alimento), 1, fp) == 1) {
        total++;
        /* identificar categoria pelo nome do alimento */
        Categoria cat = mapNomeParaCategoria(temp.nome);
        if (cat >= CEREAIS && cat <= SEMENTES) {
            category_counts[cat]++;
        } else {
            /* se por algum motivo inválido, ignore (não conta) */
        }
    }

    fclose(fp);
    return total;
}

/* ---------------------- main ---------------------- */

int main(void) {
    printf("===========================================\n");
    printf("   SISTEMA DE ALIMENTOS - P2\n");
    printf("===========================================\n\n");

    const char *caminho = PATH_DADOS;
    int total = lerBinarioContandoCategorias(caminho);
    if (total < 0) return 1;

    printf("✅ %d alimentos foram carregados do binário com sucesso!\n", total);

    int opcao;
    do {
        printf("\n--- MENU PRINCIPAL ---\n");
        printf("1. Listar categorias (com quantidade)\n");
        printf("9. Sair\n");
        printf("Escolha uma opção: ");
        if (scanf("%d", &opcao) != 1) {
            /* limpa stdin e repete */
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
            printf("Entrada inválida. Digite um número.\n");
            continue;
        }

        switch (opcao) {
            case 1:
                listarCategoriasComContagem();
                break;
            case 9:
                printf("Encerrando o programa...\n");
                break;
            default:
                printf("Opção inválida!\n");
                break;
        }

    } while (opcao != 9);

    return 0;
}
