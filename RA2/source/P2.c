#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/data_structures.h"
#include "../include/file_format.h"

#define PATH_DADOS "source/dados.bin"

void salvarDadosBinariosAtualizado(const char *caminho, NoCategoria *head);
static int category_counts[NUM_CATEGORIAS + 1]; // 1..15

/* FUNÇÕES AUXILIARES */

int contarTotalAlimentos(NoCategoria *head) {
    int total = 0;
    while (head) {
        total += head->total_alimentos;
        head = head->prox;
    }
    return total;
}

/* Converte string para minúsculas (em buffer destino) */
static void to_lower_copy(char *dst, const char *src, size_t dst_size) {
    if (dst_size == 0) return;
    size_t i;
    for (i = 0; i + 1 < dst_size && src[i] != '\0'; i++) {
        dst[i] = (char) tolower((unsigned char) src[i]);
    }
    dst[i] = '\0';
}

/* Tenta identificar categoria pelo nome do alimento */
static Categoria mapNomeParaCategoria(const char *nome) {
    char tmp[256];
    to_lower_copy(tmp, nome ? nome : "", sizeof(tmp));

    if (strstr(tmp, "cereais")) return CEREAIS;
    if (strstr(tmp, "verdura") || strstr(tmp, "hortali")) return VERDURAS;
    if (strstr(tmp, "fruta") || strstr(tmp, "maca") || strstr(tmp, "banana") || strstr(tmp, "laranja")) return FRUTAS;
    if (strstr(tmp, "gordura") || strstr(tmp, "oleo") || strstr(tmp, "óleo")) return GORDURAS;
    if (strstr(tmp, "pescad") || strstr(tmp, "atum") || strstr(tmp, "salm")) return PESCADOS;
    if (strstr(tmp, "carne") || strstr(tmp, "bovina") || strstr(tmp, "frango")) return CARNES;
    if (strstr(tmp, "leite") || strstr(tmp, "queijo") || strstr(tmp, "lacteo")) return LACTEOS;
    if (strstr(tmp, "suco") || strstr(tmp, "cerveja") || strstr(tmp, "bebid")) return BEBIDAS;
    if (strstr(tmp, "ovo")) return OVOS;
    if (strstr(tmp, "acucar") || strstr(tmp, "doce")) return ACUCARES;
    if (strstr(tmp, "miscel")) return MISCELANEAS;
    if (strstr(tmp, "industrial")) return INDUSTRIALIZADOS;
    if (strstr(tmp, "preparad")) return PREPARADOS;
    if (strstr(tmp, "feijao") || strstr(tmp, "soja")) return LEGUMINOSAS;
    if (strstr(tmp, "noz") || strstr(tmp, "castanha")) return SEMENTES;

    return CEREAIS; // padrão se não achar
}

/* Nome textual da categoria */
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

/* FUNÇÕES PRINCIPAIS */

/* Lê o binário e conta alimentos por categoria */
int lerBinarioContandoCategorias(const char *caminho) {
    FILE *fp = fopen(caminho, "rb");
    if (!fp) {
        printf("❌ Erro: não foi possível abrir o arquivo '%s'.\n", caminho);
        return -1;
    }

    for (int i = 0; i <= NUM_CATEGORIAS; i++)
        category_counts[i] = 0;

    Alimento temp;
    int total = 0;

    while (fread(&temp, sizeof(Alimento), 1, fp) == 1) {
        total++;
        Categoria cat = mapNomeParaCategoria(temp.nome);
        if (cat >= CEREAIS && cat <= SEMENTES)
            category_counts[cat]++;
    }

    fclose(fp);
    return total;
}

/* Lista todas as categorias e suas contagens */
void listarCategoriasComContagem(NoCategoria *head) {
    NoCategoria *categorias[NUM_CATEGORIAS] = {0};
    NoCategoria *ptr = head;
    while (ptr) {
        categorias[ptr->id - 1] = ptr; // assumes ids 1..15
        ptr = ptr->prox;
    }

    printf("\n--- Lista de Categorias (quantidade) ---\n");
    for (int i = 0; i < NUM_CATEGORIAS; i++) {
        if (categorias[i])
            printf("%2d. %-40s : %3d\n", categorias[i]->id, categorias[i]->nome, categorias[i]->total_alimentos);
    }
}

/* FUNÇÕES DE MENU */

/* Opção 2: Listar todos os alimentos de uma categoria */
void listarAlimentosDeCategoria(const char *caminho) {
    int opc;
    printf("\nDigite o numero da categoria (1–15): ");
    if (scanf("%d", &opc) != 1 || opc < 1 || opc > NUM_CATEGORIAS) {
        printf("Categoria inválida.\n");
        while (getchar() != '\n'); // limpa entrada
        return;
    }

    Categoria cat = (Categoria)opc;
    FILE *fp = fopen(caminho, "rb");
    if (!fp) {
        printf("Erro: não foi possível abrir o arquivo '%s'.\n", caminho);
        return;
    }

    printf("\n--- Alimentos da categoria: %s ---\n", nomeCategoria(cat));

    Alimento temp;
    int encontrados = 0;
    while (fread(&temp, sizeof(Alimento), 1, fp) == 1) {
        Categoria atual = mapNomeParaCategoria(temp.nome);
        if (atual == cat) {
            printf("Código: %d | Nome: %-30s | Calorias: %6.2f | Proteínas: %6.2f\n",
                   temp.codigo, temp.nome, temp.calorias, temp.proteinas);
            encontrados++;
        }
    }

    if (encontrados == 0)
        printf("(Nenhum alimento encontrado nesta categoria)\n");

    fclose(fp);
}

/* Opção 3: Listar por energia (decrescente) */
/* Helpers: inserir alimento ordenado por nome, inserir na árvore por energia,
   percorrer árvore em ordem decrescente, liberar estruturas. */

static NoAlimento* criarNoAlimento(const AlimentoDados *dados) {
    NoAlimento *n = (NoAlimento*) malloc(sizeof(NoAlimento));
    if (!n) return NULL;
    n->dados = *dados;
    n->prox = NULL;
    n->no_arvore_energia = NULL;
    n->no_arvore_proteina = NULL;
    return n;
}

/* Insere na lista ligada em ordem alfabética pela descricao */
static void inserirAlimentoOrdenadoPorNome(NoAlimento **head, NoAlimento *novo) {
    if (head == NULL || novo == NULL) return;

    if (*head == NULL || strcasecmp(novo->dados.descricao, (*head)->dados.descricao) < 0) {
        novo->prox = *head;
        *head = novo;
        return;
    }

    NoAlimento *atual = *head;
    while (atual->prox != NULL && strcasecmp(atual->prox->dados.descricao, novo->dados.descricao) < 0) {
        atual = atual->prox;
    }
    novo->prox = atual->prox;
    atual->prox = novo;
}

NoArvore* inserirNoArvoreProteina(NoArvore *root, NoAlimento *ptr_alim) {
    if (!root) {
        NoArvore *novo = malloc(sizeof(NoArvore));
        novo->ptr_alimento = ptr_alim;
        novo->chave = ptr_alim->dados.proteina;
        novo->esq = novo->dir = NULL;
        return novo;
    }

    if (ptr_alim->dados.proteina < root->chave)
        root->esq = inserirNoArvoreProteina(root->esq, ptr_alim);
    else
        root->dir = inserirNoArvoreProteina(root->dir, ptr_alim);

    return root;
}

NoArvore* inserirNoArvoreEnergia(NoArvore *root, NoAlimento *ptr_alim) {
    if (!root) {
        NoArvore *novo = (NoArvore*) malloc(sizeof(NoArvore));
        novo->ptr_alimento = ptr_alim;
        novo->chave = ptr_alim->dados.energia;
        novo->esq = novo->dir = NULL;
        return novo;
    }

    if (ptr_alim->dados.energia < root->chave)
        root->esq = inserirNoArvoreEnergia(root->esq, ptr_alim);
    else
        root->dir = inserirNoArvoreEnergia(root->dir, ptr_alim);

    return root;
}

void liberarListaCategorias(NoCategoria *head) {
    NoCategoria *cat_atual = head;
    while (cat_atual) {
        NoCategoria *prox_cat = cat_atual->prox;

        // Libera lista de alimentos
        NoAlimento *alim_atual = cat_atual->lista_alimentos_head;
        while (alim_atual) {
            NoAlimento *prox_alim = alim_atual->prox;
            free(alim_atual);
            alim_atual = prox_alim;
        }

        // Libera árvores
        // Função auxiliar para liberar árvores
        void liberarArvore(NoArvore *root) {
            if (!root) return;
            liberarArvore(root->esq);
            liberarArvore(root->dir);
            free(root);
        }
        liberarArvore(cat_atual->arvore_energia_root);
        liberarArvore(cat_atual->arvore_proteina_root);

        free(cat_atual);
        cat_atual = prox_cat;
    }
}



/* Constrói a lista de categorias, lista de alimentos e árvores */

NoCategoria* construirEstruturasCategorias(const char *caminho) {
    FILE *fp = fopen(caminho, "rb");
    if (!fp) {
        printf("❌ Erro: não foi possível abrir o arquivo '%s'.\n", caminho);
        return NULL;
    }

    NoCategoria *lista_categorias_head = NULL;

    Alimento temp;
    while (fread(&temp, sizeof(Alimento), 1, fp) == 1) {
        /* identifica categoria */
        Categoria cat = mapNomeParaCategoria(temp.nome);

        /* procura se categoria já existe na lista */
        NoCategoria *cat_no = lista_categorias_head;
        NoCategoria *anterior = NULL;
        while (cat_no && cat_no->id != cat) {
            anterior = cat_no;
            cat_no = cat_no->prox;
        }

        if (!cat_no) {
            /* cria novo nó de categoria */
            cat_no = (NoCategoria*) malloc(sizeof(NoCategoria));
            if (!cat_no) {
                printf("Erro de memória ao criar categoria.\n");
                fclose(fp);
                return lista_categorias_head;
            }
            cat_no->id = cat;
            strncpy(cat_no->nome, nomeCategoria(cat), MAX_DESC-1);
            cat_no->nome[MAX_DESC-1] = '\0';
            cat_no->lista_alimentos_head = NULL;
            cat_no->total_alimentos = 0;
            cat_no->arvore_energia_root = NULL;
            cat_no->arvore_proteina_root = NULL;
            cat_no->prox = NULL;

            /* adiciona ao final da lista */
            if (!anterior) {
                lista_categorias_head = cat_no;
            } else {
                anterior->prox = cat_no;
            }
        }

        /* cria nó de alimento */
        AlimentoDados dados;
        dados.codigo = temp.codigo;
        strncpy(dados.descricao, temp.nome, MAX_DESC-1);
        dados.descricao[MAX_DESC-1] = '\0';
        dados.energia = temp.calorias;
        dados.proteina = temp.proteinas;
        dados.categoria = cat;

        NoAlimento *novo_no = criarNoAlimento(&dados);
        if (!novo_no) {
            printf("Erro de memória ao criar nó de alimento.\n");
            fclose(fp);
            return lista_categorias_head;
        }

        /* insere na lista de alimentos (ordem alfabética) */
        inserirAlimentoOrdenadoPorNome(&cat_no->lista_alimentos_head, novo_no);
        cat_no->total_alimentos++;

        /* insere nas árvores de indexação */
        cat_no->arvore_energia_root = inserirNoArvoreEnergia(cat_no->arvore_energia_root, novo_no);

        /* árvore de proteína: mesma lógica que energia, só que chave = proteina */
        if (!novo_no->no_arvore_proteina) {
            NoArvore *raiz_prot = (NoArvore*) malloc(sizeof(NoArvore));
            if (!raiz_prot) continue;
            raiz_prot->chave = novo_no->dados.proteina;
            raiz_prot->ptr_alimento = novo_no;
            raiz_prot->esq = raiz_prot->dir = NULL;
            novo_no->no_arvore_proteina = raiz_prot;
        }
        cat_no->arvore_proteina_root = inserirNoArvoreProteina(cat_no->arvore_proteina_root, novo_no);
    }

    fclose(fp);
    return lista_categorias_head;
}

/* Percorre em ordem decrescente: dir, node, esq */
static void percorrerArvoreEnergiaDesc(NoArvore *root, int *contador) {
    if (root == NULL) return;
    percorrerArvoreEnergiaDesc(root->dir, contador);
    /* imprime o alimento apontado */
    NoAlimento *a = root->ptr_alimento;
    if (a) {
        printf("Código: %4d | %-45s | Energia: %7.2f kcal | Proteína: %6.2f g\n",
               a->dados.codigo, a->dados.descricao, a->dados.energia, a->dados.proteina);
        (*contador)++;
    }
    percorrerArvoreEnergiaDesc(root->esq, contador);
}

/* Libera árvore */
static void liberarArvore(NoArvore *root) {
    if (!root) return;
    liberarArvore(root->esq);
    liberarArvore(root->dir);
    free(root);
}

/* Libera lista ligada de alimentos (libera nós; não toca em ptrs na árvore pq apontam para esses nós) */
static void liberarListaAlimentos(NoAlimento *head) {
    NoAlimento *at = head;
    while (at) {
        NoAlimento *tmp = at;
        at = at->prox;
        free(tmp);
    }
}

/* opção 3 — recebe o caminho do binário */
void listarAlimentosPorEnergia(const char *caminho) {
    if (!caminho) return;

    int opc;
    printf("\nDigite o número da categoria (1–15): ");
    if (scanf("%d", &opc) != 1 || opc < 1 || opc > NUM_CATEGORIAS) {
        printf("Categoria inválida.\n");
        while (getchar() != '\n'); // limpa entrada
        return;
    }
    while (getchar() != '\n'); // limpa resto da linha

    Categoria cat_escolhida = (Categoria)opc;

    FILE *fp = fopen(caminho, "rb");
    if (!fp) {
        printf("Erro: não foi possível abrir o arquivo '%s'.\n", caminho);
        return;
    }

    /* Construir lista ligada (ordenada por nome) e árvore indexando por energia */
    NoAlimento *lista_head = NULL;
    NoArvore *arvore_root = NULL;

    Alimento temp;
    while (fread(&temp, sizeof(Alimento), 1, fp) == 1) {
        /* determina categoria do registro (usa sua heurística existente mapNomeParaCategoria) */
        Categoria cat = mapNomeParaCategoria(temp.nome);
        if (cat != cat_escolhida) continue;

        /* converte Alimento -> AlimentoDados */
        AlimentoDados dados;
        dados.codigo = temp.codigo;
        /* copiar nome (respeitando MAX_DESC) */
        strncpy(dados.descricao, temp.nome, MAX_DESC - 1);
        dados.descricao[MAX_DESC - 1] = '\0';
        dados.energia = temp.calorias;
        dados.proteina = temp.proteinas;
        dados.categoria = cat_escolhida;

        /* cria nó e insere na lista por nome */
        NoAlimento *novo = criarNoAlimento(&dados);
        if (!novo) {
            printf("Erro de memória ao criar nó de alimento.\n");
            fclose(fp);
            liberarListaAlimentos(lista_head);
            liberarArvore(arvore_root);
            return;
        }
        inserirAlimentoOrdenadoPorNome(&lista_head, novo);

        /* também insere na árvore (aponta para o mesmo nó da lista) */
        arvore_root = inserirNoArvoreEnergia(arvore_root, novo);
    }

    fclose(fp);

    /* Imprime resultados usando a árvore para garantir ordem decrescente por energia */
    printf("\n--- Alimentos da categoria: %s (EM ORDEM DECRESCENTE POR ENERGIA) ---\n", nomeCategoria(cat_escolhida));
    if (arvore_root == NULL) {
        printf("(Nenhum alimento encontrado nesta categoria)\n");
    } else {
        /* cabeçalho simplificado */
        printf("Código | Descrição                                           | Energia  | Proteína\n");
        printf("-------+-----------------------------------------------------+----------+---------\n");
        int contador = 0;
        percorrerArvoreEnergiaDesc(arvore_root, &contador);
        printf("-------+-----------------------------------------------------+----------+---------\n");
        printf("Total: %d alimento(s)\n", contador);
    }

    /* libera memória */
    liberarArvore(arvore_root);
    liberarListaAlimentos(lista_head);
}

/* Converte inteiro 1 ~ 15 para Categoria */
Categoria intParaCategoria(int id) {
    if (id >= CEREAIS && id <= SEMENTES)
        return (Categoria)id;
    return CEREAIS; // padrão
}

NoCategoria* buscarCategoriaPorID(NoCategoria *head, Categoria c, NoCategoria **prev) {
    NoCategoria *anterior = NULL;
    NoCategoria *atual = head;
    while (atual) {
        if (atual->id == c) {
            if (prev) *prev = anterior;
            return atual;
        }
        anterior = atual;
        atual = atual->prox;
    }
    if (prev) *prev = NULL;
    return NULL;
}



/* Opção 4: Listar por proteína */
void listarAlimentosPorProteina(NoCategoria *lista_categorias_head) {
    if (!lista_categorias_head) {
        printf("Nao ha categorias carregadas.\n");
        return;
    }

    int cat_id;
    printf("Digite o numero da categoria desejada (1-%d): ", NUM_CATEGORIAS);
    if (scanf("%d", &cat_id) != 1) {
        int c; while ((c = getchar()) != '\n' && c != EOF) { }
        printf("Entrada invalida.\n");
        return;
    }

    /* Limpa buffer */
    int ch; while ((ch = getchar()) != '\n' && ch != EOF) { }

    NoCategoria *cat = buscarCategoriaPorID(lista_categorias_head, intParaCategoria(cat_id), NULL);
    if (!cat) {
        printf("Categoria nao encontrada.\n");
        return;
    }

    if (!cat->arvore_proteina_root) {
        printf("Nenhum alimento cadastrado nesta categoria.\n");
        return;
    }

    printf("\n--- Alimentos de %s por PROTEINA (decrescente) ---\n", cat->nome);

    /* Função interna recursiva para percorrer árvore em decrescente */
    void percorrerArvoreDecrescente(NoArvore *no) {
        if (!no) return;
        percorrerArvoreDecrescente(no->dir); /* primeiro direita = maior */
        printf("Codigo: %d | Nome: %-30s | Proteina: %.2fg | Energia: %.2f Kcal\n",
               no->ptr_alimento->dados.codigo,
               no->ptr_alimento->dados.descricao,
               no->ptr_alimento->dados.proteina,
               no->ptr_alimento->dados.energia);
        percorrerArvoreDecrescente(no->esq);
    }

    percorrerArvoreDecrescente(cat->arvore_proteina_root);
    printf("\nTotal de alimentos exibidos: %d\n", cat->total_alimentos);
}

/* Opção 5: Listar alimentos por intervalo de energia */
void listarAlimentosPorIntervaloEnergia(NoCategoria *lista_categorias_head) {
    if (!lista_categorias_head) {
        printf("Não há categorias carregadas.\n");
        return;
    }

    int cat_id;
    printf("Digite o número da categoria desejada (1-%d): ", NUM_CATEGORIAS);
    if (scanf("%d", &cat_id) != 1) {
        int c; while ((c = getchar()) != '\n' && c != EOF) {}
        printf("Entrada inválida.\n");
        return;
    }

    float min, max;
    printf("Digite o valor mínimo de energia (Kcal): ");
    if (scanf("%f", &min) != 1) { while (getchar() != '\n'); printf("Entrada inválida.\n"); return; }
    printf("Digite o valor máximo de energia (Kcal): ");
    if (scanf("%f", &max) != 1) { while (getchar() != '\n'); printf("Entrada inválida.\n"); return; }
    if (min > max) { printf("O mínimo não pode ser maior que o máximo.\n"); return; }

    /* Limpa buffer */
    int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}

    NoCategoria *cat = buscarCategoriaPorID(lista_categorias_head, intParaCategoria(cat_id), NULL);
    if (!cat) {
        printf("Categoria não encontrada.\n");
        return;
    }

    if (!cat->arvore_energia_root) {
        printf("Nenhum alimento cadastrado nesta categoria.\n");
        return;
    }

    printf("\n--- Alimentos de %s com energia entre %.2f e %.2f Kcal ---\n", cat->nome, min, max);
    printf("Código | Descrição                                           | Energia  | Proteína\n");
    printf("-------+-----------------------------------------------------+----------+---------\n");

    /* Função recursiva interna */
    void percorrerArvoreIntervalo(NoArvore *no) {
        if (!no) return;

        if (no->chave > min) // há chance de subárvore esquerda ter elementos no intervalo
            percorrerArvoreIntervalo(no->esq);

        if (no->chave >= min && no->chave <= max) {
            NoAlimento *a = no->ptr_alimento;
            if (a) {
                printf("%4d   | %-45s | %7.2f | %6.2f\n",
                       a->dados.codigo, a->dados.descricao, a->dados.energia, a->dados.proteina);
            }
        }

        if (no->chave < max) // há chance de subárvore direita ter elementos no intervalo
            percorrerArvoreIntervalo(no->dir);
    }

    percorrerArvoreIntervalo(cat->arvore_energia_root);
}

/* Opção 6: Listar por intervalo de proteína */
void listarAlimentosPorIntervaloProteina(NoCategoria *lista_categorias_head) {
    if (!lista_categorias_head) {
        printf("Não há categorias carregadas.\n");
        return;
    }

    int cat_id;
    printf("\nDigite o numero da categoria desejada (1-%d): ", NUM_CATEGORIAS);
    if (scanf("%d", &cat_id) != 1 || cat_id < 1 || cat_id > NUM_CATEGORIAS) {
        int c; while ((c = getchar()) != '\n' && c != EOF) {}
        printf("Entrada inválida.\n");
        return;
    }
    while (getchar() != '\n'); // limpa buffer

    NoCategoria *cat = buscarCategoriaPorID(lista_categorias_head, intParaCategoria(cat_id), NULL);
    if (!cat) {
        printf("Categoria não encontrada.\n");
        return;
    }

    if (!cat->arvore_proteina_root) {
        printf("Nenhum alimento cadastrado nesta categoria.\n");
        return;
    }

    float min_prot, max_prot;
    printf("Digite o valor minimo de proteina (g): ");
    if (scanf("%f", &min_prot) != 1) { while(getchar() != '\n'); printf("Entrada inválida.\n"); return; }
    printf("Digite o valor maximo de proteina (g): ");
    if (scanf("%f", &max_prot) != 1) { while(getchar() != '\n'); printf("Entrada inválida.\n"); return; }
    while(getchar() != '\n');

    if (min_prot > max_prot) {
        float tmp = min_prot;
        min_prot = max_prot;
        max_prot = tmp;
    }

    int contador = 0;
    printf("\n--- Alimentos de %s com proteína entre %.2fg e %.2fg ---\n", cat->nome, min_prot, max_prot);
    printf("Código | Descrição                                           | Proteína | Energia\n");
    printf("-------+-----------------------------------------------------+----------+--------\n");

    /* Função interna recursiva para percorrer árvore de proteína em ordem decrescente e filtrar */
    void percorrerArvoreProteinaIntervalo(NoArvore *no) {
        if (!no) return;

        /* percorre direita primeiro = decrescente */
        percorrerArvoreProteinaIntervalo(no->dir);

        if (no->chave >= min_prot && no->chave <= max_prot) {
            NoAlimento *a = no->ptr_alimento;
            if (a) {
                printf("%6d | %-45s | %8.2f | %6.2f\n",
                       a->dados.codigo, a->dados.descricao,
                       a->dados.proteina, a->dados.energia);
                contador++;
            }
        }

        /* percorre esquerda */
        percorrerArvoreProteinaIntervalo(no->esq);
    }

    percorrerArvoreProteinaIntervalo(cat->arvore_proteina_root);

    if (contador == 0)
        printf("(Nenhum alimento encontrado neste intervalo)\n");

    printf("-------+-----------------------------------------------------+----------+--------\n");
    printf("Total de alimentos exibidos: %d\n", contador);
}

/* BACKUP INICIAL */
void criarBackupInicial(const char *caminho_original) {
    const char *backup = "dados_original.bin";
    FILE *orig = fopen(caminho_original, "rb");
    if (!orig) {
        printf("❌ Erro: não foi possível abrir '%s' para backup.\n", caminho_original);
        exit(1);
    }

    FILE *bkp = fopen(backup, "wb");
    if (!bkp) {
        printf("❌ Erro: não foi possível criar backup '%s'.\n", backup);
        fclose(orig);
        exit(1);
    }

    char buffer[1024];
    size_t lidos;
    while ((lidos = fread(buffer, 1, sizeof(buffer), orig)) > 0) {
        fwrite(buffer, 1, lidos, bkp);
    }

    fclose(orig);
    fclose(bkp);
    printf("✅ Backup inicial criado como '%s'.\n", backup);
}

/* OPÇÃO 7: REMOVER CATEGORIA */
NoCategoria* removerCategoria(NoCategoria *head, int *alteracao_dados) {
    if (!head) {
        printf("Nenhuma categoria carregada.\n");
        return head;
    }

    NoCategoria *ptr = head;
    int n = 0;
    printf("\n--- Lista de Categorias ---\n");
    while (ptr) {
        n++;
        printf("%2d. %s\n", n, ptr->nome);
        ptr = ptr->prox;
    }

    int escolha;
    printf("\nDigite o número da categoria a remover (1-%d): ", n);
    if (scanf("%d", &escolha) != 1 || escolha < 1 || escolha > n) {
        while (getchar() != '\n');
        printf("Entrada inválida.\n");
        return head;
    }
    while (getchar() != '\n');

    NoCategoria *anterior = NULL;
    NoCategoria *atual = head;
    int idx = 1;
    while (atual && idx < escolha) {
        anterior = atual;
        atual = atual->prox;
        idx++;
    }

    if (!atual) return head;

    /* libera memória da categoria */
    NoAlimento *a = atual->lista_alimentos_head;
    while (a) {
        NoAlimento *tmp = a;
        a = a->prox;
        free(tmp);
    }
    liberarArvore(atual->arvore_energia_root);
    liberarArvore(atual->arvore_proteina_root);

    if (anterior == NULL) head = atual->prox;
    else anterior->prox = atual->prox;

    printf("Categoria '%s' removida com sucesso!\n", atual->nome);
    free(atual);

    if (alteracao_dados) *alteracao_dados = 1;
    return head;
}

/* OPÇÃO 8: REMOVER ALIMENTO ESPECÍFICO */
void removerAlimentoEspecifico(NoCategoria *head, int *alteracao_dados) {
    if (!head) {
        printf("Nenhuma categoria carregada.\n");
        return;
    }

    /* Lista categorias */
    NoCategoria *cat = head;
    int idx_cat = 1;
    printf("\n--- Categorias ---\n");
    while (cat) {
        printf("%2d. %s\n", idx_cat, cat->nome);
        cat = cat->prox;
        idx_cat++;
    }

    int escolha_cat;
    printf("\nDigite o número da categoria: ");
    if (scanf("%d", &escolha_cat) != 1 || escolha_cat < 1 || escolha_cat >= idx_cat) {
        while (getchar() != '\n');
        printf("Entrada inválida.\n");
        return;
    }
    while (getchar() != '\n');

    cat = head;
    for (int i = 1; i < escolha_cat && cat; i++) cat = cat->prox;
    if (!cat || !cat->lista_alimentos_head) { printf("Categoria vazia.\n"); return; }

    /* Lista alimentos */
    NoAlimento *a = cat->lista_alimentos_head;
    int idx = 1;
    printf("\n--- Alimentos em %s ---\n", cat->nome);
    while (a) {
        printf("%3d. %-40s | Energia: %.2f | Proteína: %.2f\n",
               idx, a->dados.descricao, a->dados.energia, a->dados.proteina);
        a = a->prox;
        idx++;
    }

    int escolha_alim;
    printf("\nDigite o número do alimento a remover: ");
    if (scanf("%d", &escolha_alim) != 1 || escolha_alim < 1 || escolha_alim >= idx) {
        while (getchar() != '\n');
        printf("Entrada inválida.\n");
        return;
    }
    while (getchar() != '\n');

    NoAlimento *atual = cat->lista_alimentos_head;
    NoAlimento *anterior = NULL;
    int contador = 1;
    while (atual && contador < escolha_alim) {
        anterior = atual;
        atual = atual->prox;
        contador++;
    }

    if (!atual) { printf("Alimento não encontrado.\n"); return; }

    if (!anterior) cat->lista_alimentos_head = atual->prox;
    else anterior->prox = atual->prox;

    printf("Alimento '%s' removido da categoria '%s'.\n", atual->dados.descricao, cat->nome);
    free(atual);

    /* Reconstroi árvores */
    liberarArvore(cat->arvore_energia_root);
    liberarArvore(cat->arvore_proteina_root);
    cat->arvore_energia_root = NULL;
    cat->arvore_proteina_root = NULL;
    cat->total_alimentos = 0;

    NoAlimento *tmp = cat->lista_alimentos_head;
    while (tmp) {
        cat->arvore_energia_root = inserirNoArvoreEnergia(cat->arvore_energia_root, tmp);
        cat->arvore_proteina_root = inserirNoArvoreProteina(cat->arvore_proteina_root, tmp);
        cat->total_alimentos++;
        tmp = tmp->prox;
    }

    if (alteracao_dados) *alteracao_dados = 1;
}

/* OPÇÃO 9: ENCERRAR E SALVAR */
void encerrarPrograma(const char *caminho, NoCategoria *head, int alteracao_dados) {
    if (alteracao_dados) {
        salvarDadosBinariosAtualizado(caminho, head);
    } else {
        printf("\nNenhuma alteração detectada. Encerrando.\n");
    }
    liberarListaCategorias(head);
    printf("Programa encerrado.\n");
}






/* ===========================================================
   MAIN
   =========================================================== */

int main(void) {
    int alteracao_dados = 0;
    printf("===========================================\n");
    printf("   SISTEMA DE ALIMENTOS - P2\n");
    printf("===========================================\n\n");

    const char *caminho = PATH_DADOS;
    criarBackupInicial(caminho);
    NoCategoria *lista_categorias_head = construirEstruturasCategorias(caminho);
    if (!lista_categorias_head) {
        printf("Erro ao construir estruturas de categorias.\n");
        return 1;
    }

    int total = 0;
    NoCategoria *ptr = lista_categorias_head;
    while (ptr) {
        total += ptr->total_alimentos;
        ptr = ptr->prox;
    }

    printf("✅ %d alimentos foram carregados do binário com sucesso!\n", total);



    printf("✅ %d alimentos foram carregados do binário com sucesso!\n", total);

    int opcao;
    do {
        printf("\n--- MENU PRINCIPAL ---\n");
        printf("1  - Listar todas as categorias (ordem da lista)\n");
        printf("2  - Listar todos os alimentos de certa categoria (ordem da lista)\n");
        printf("3  - Listar alimentos de certa categoria por ENERGIA (decrescente)\n");
        printf("4  - Listar alimentos de certa categoria por PROTEINA (decrescente)\n");
        printf("5  - Listar alimentos de certa categoria por INTERVALO DE ENERGIA\n");
        printf("6  - Listar alimentos de certa categoria por INTERVALO DE PROTEINA\n");
        printf("7  - Remover uma categoria\n");
        printf("8  - Remover um alimento especifico\n");
        printf("9  - Encerrar (salva se houver alteracoes)\n");
        printf("Escolha uma opcao (1-9): ");

        if (scanf("%d", &opcao) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("Entrada inválida. Digite um número.\n");
            continue;
        }

        switch (opcao) {
            case 1:
                listarCategoriasComContagem(lista_categorias_head);
                break;
            case 2:
                listarAlimentosDeCategoria(caminho);
                break;
            case 3:
                listarAlimentosPorEnergia(caminho);
                break;
            case 4:
                listarAlimentosPorProteina(lista_categorias_head);
                break;
            case 5:
                listarAlimentosPorIntervaloEnergia(lista_categorias_head);
                break;
            case 6:
                listarAlimentosPorIntervaloProteina(lista_categorias_head);
                break;
            case 7:
                lista_categorias_head = removerCategoria(lista_categorias_head, &alteracao_dados);
                break;
            case 8:
                removerAlimentoEspecifico(lista_categorias_head, &alteracao_dados);
                break;
            case 9:
                encerrarPrograma(caminho, lista_categorias_head, alteracao_dados);
                break;
        }
    } while (opcao != 9);

        if (alteracao_dados) {
            salvarDadosBinariosAtualizado("arquivo_alterado.bin", lista_categorias_head);
            printf("Arquivo atualizado salvo como 'arquivo_alterado.bin'.\n");
        }

    liberarListaCategorias(lista_categorias_head);
    return 0;
}

