#include "data_structures.h"
#include "file_format.h" // Tipos e estruturas
#include "file_io.h"     // Necessário para chamadas de I/O em alguns casos (ex: P1.c)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Protótipos das funções auxiliares de impressão que estão no data_structure.c */
void imprimirCabecalhoTabela(void);
void imprimirRodapeTabela(int total);
void imprimirLinhaAlimento(const Alimento *a);

// --- Alocação de Memória (Requisito 4) ---

/* Aloca e inicializa um novo nó de Alimento */
NoAlimento* criarNoAlimento(const Alimento *dados) {
    NoAlimento *novo_no = (NoAlimento*)malloc(sizeof(NoAlimento));
    if (novo_no == NULL) {
        printf("Erro de alocação de memória para NoAlimento.\n");
        exit(EXIT_FAILURE);
    }
    novo_no->dados = *dados; // Copia os dados
    novo_no->proximo = NULL;
    return novo_no;
}

/* Aloca e inicializa um novo nó de Categoria */
NoCategoria* criarNoCategoria(Categoria id) {
    // calloc inicializa a memória com zero, ideal para ponteiros
    NoCategoria *novo_no = (NoCategoria*)calloc(1, sizeof(NoCategoria));
    if (novo_no == NULL) {
        printf("Erro de alocação de memória para NoCategoria.\n");
        exit(EXIT_FAILURE);
    }
    novo_no->id = id;
    // Pega o nome descritivo da categoria
    strncpy(novo_no->nome, obterNomeCategoria(id), sizeof(novo_no->nome) - 1);
    // Campos já zerados pelo calloc:
    // novo_no->alimentos = NULL;
    // novo_no->arvore_energia = NULL;
    // novo_no->arvore_proteina = NULL;
    // novo_no->proximo = NULL;
    return novo_no;
}

// --- Desalocação de Memória (Requisito 5) ---

/* Libera recursivamente a memória de uma Árvore Binária */
void liberarArvore(NoArvore *raiz) {
    if (raiz != NULL) {
        liberarArvore(raiz->esquerda);
        liberarArvore(raiz->direita);
        free(raiz);
    }
}

/* Libera a memória de uma Lista Ligada de Alimentos */
void liberarListaAlimentos(NoAlimento *lista) {
    NoAlimento *atual = lista;
    while (atual != NULL) {
        NoAlimento *temp = atual;
        atual = atual->proximo;
        free(temp);
    }
}

/* Libera a memória de toda a estrutura (Listas e Árvores) */
void liberarListaCategorias(NoCategoria *lista) {
    NoCategoria *atual = lista;
    while (atual != NULL) {
        NoCategoria *temp = atual;
        atual = atual->proximo;
        
        liberarListaAlimentos(temp->alimentos);
        liberarArvore(temp->arvore_energia);
        liberarArvore(temp->arvore_proteina);
        free(temp);
    }
}

// --- Funções de Inserção Ordenada (Requisitos 6 e 7) ---

/* Busca uma categoria na lista ou a cria (e insere ordenadamente alfabeticamente) se não existir */
NoCategoria* encontrarCategoriaOuCriar(NoCategoria **lista_categorias, Categoria id) {
    // 1. Caso CAT_INVALIDA: ignora
    if (id == CAT_INVALIDA) return NULL;

    NoCategoria *novo_no = NULL;
    NoCategoria *atual = *lista_categorias;
    NoCategoria *anterior = NULL;
    const char *nome_cat = obterNomeCategoria(id);
    
    // Loop: Busca a categoria na lista
    while (atual != NULL) {
        int cmp = strcmp(nome_cat, atual->nome);
        // Categoria encontrada
        if (atual->id == id) {
            return atual;
        }
        // O novo nó deve ser inserido antes do atual (ordem alfabética, Requisito 7)
        else if (cmp < 0) {
            break;
        }
        anterior = atual;
        atual = atual->proximo;
    }

    // Categoria não encontrada, cria e insere na posição correta
    novo_no = criarNoCategoria(id);
    
    // Inserção no início da lista (inclui lista vazia)
    if (anterior == NULL) {
        novo_no->proximo = *lista_categorias;
        *lista_categorias = novo_no;
    } 
    // Inserção no meio ou no fim
    else {
        novo_no->proximo = anterior->proximo;
        anterior->proximo = novo_no;
    }
    
    return novo_no;
}

/* Insere um alimento na lista ligada da categoria (ordenado alfabeticamente) */
int inserirAlimentoNaLista(NoCategoria *categoria, const Alimento *dados) {
    NoAlimento *novo_alimento = criarNoAlimento(dados);
    NoAlimento **ponteiro = &(categoria->alimentos); // Ponteiro para o ponteiro do nó atual
    
    // Loop: Percorre a lista de alimentos, parando na posição correta ou no fim
    while (*ponteiro != NULL) {
        // Se o novo alimento deve vir antes do nó atual (ordem alfabética, Requisito 6)
        if (strcmp(dados->nome, (*ponteiro)->dados.nome) < 0) {
            break;
        }
        ponteiro = &(*ponteiro)->proximo; // Avança o ponteiro
    }
    
    novo_alimento->proximo = *ponteiro;
    *ponteiro = novo_alimento;
    return 1;
}

/* Função principal de inserção: encontra/cria categoria e insere alimento */
int inserirAlimento(NoCategoria **lista_categorias, const Alimento *dados) {
    // 1. Encontra a categoria ou a cria (inserção ordenada de categoria)
    NoCategoria *categoria = encontrarCategoriaOuCriar(lista_categorias, dados->categoria);
    
    // 2. Insere o alimento na lista interna (inserção ordenada de alimento)
    if (categoria != NULL) {
        return inserirAlimentoNaLista(categoria, dados);
    }
    return 0; // Falha na inserção
}

// --- Funções de Árvore Binária (Requisito C.b.1 e C.b.2) ---

/* Insere um nó na Árvore Binária de Indexação (ordenada decrescente por chave) */
NoArvore* inserirNaArvore(NoArvore *raiz, float chave, NoAlimento *ptr_alimento, int is_proteina) {
    if (raiz == NULL) {
        NoArvore *novo_no = (NoArvore*)malloc(sizeof(NoArvore));
        if (novo_no == NULL) {
            printf("Erro de alocação de memória para NoArvore.\n");
            exit(EXIT_FAILURE);
        }
        novo_no->chave = chave;
        novo_no->ptr_alimento = ptr_alimento;
        novo_no->esquerda = NULL;
        novo_no->direita = NULL;
        return novo_no;
    }

    /* Convenção de ordenação decrescente:
       - Valores MAIORES vão para a ESQUERDA.
       - Valores MENORES vão para a DIREITA.
       - Percurso Decrescente (do maior para o menor): E-R-D (Esquerda, Raiz, Direita)
    */
    
    // Se a nova chave é MAIOR que a chave do nó atual
    if (chave > raiz->chave) {
        raiz->esquerda = inserirNaArvore(raiz->esquerda, chave, ptr_alimento, is_proteina);
    }
    // Se a nova chave é MENOR que a chave do nó atual
    else if (chave < raiz->chave) {
        raiz->direita = inserirNaArvore(raiz->direita, chave, ptr_alimento, is_proteina);
    }
    // Chaves são iguais (critério de desempate: ID do alimento)
    else {
        int codigo_novo = ptr_alimento->dados.codigo;
        int codigo_raiz = raiz->ptr_alimento->dados.codigo;
        
        // Critério de desempate: Mantém ordem crescente de ID. ID menor vai para a ESQUERDA.
        if (codigo_novo < codigo_raiz) {
            raiz->esquerda = inserirNaArvore(raiz->esquerda, chave, ptr_alimento, is_proteina);
        } 
        // ID maior ou igual vai para a DIREITA
        else {
            raiz->direita = inserirNaArvore(raiz->direita, chave, ptr_alimento, is_proteina);
        }
    }

    return raiz;
}

/* Constrói as duas árvores de indexação para uma categoria (Requisito C.b) */
void construirArvores(NoCategoria *categoria) {
    // 1. Libera árvores existentes (se for reconstrução)
    liberarArvore(categoria->arvore_energia);
    liberarArvore(categoria->arvore_proteina);
    categoria->arvore_energia = NULL;
    categoria->arvore_proteina = NULL;

    NoAlimento *atual = categoria->alimentos;
    // Loop: Percorre a lista de alimentos e insere nas árvores
    while (atual != NULL) {
        // Insere na árvore de Energia (chave = calorias)
        categoria->arvore_energia = inserirNaArvore(
            categoria->arvore_energia, 
            atual->dados.calorias, 
            atual, 
            0
        );
        
        // Insere na árvore de Proteína (chave = proteinas)
        categoria->arvore_proteina = inserirNaArvore(
            categoria->arvore_proteina, 
            atual->dados.proteinas, 
            atual, 
            1
        );
        atual = atual->proximo;
    }
}

/* Reconstrói as árvores de indexação para uma categoria (usado após remoção) */
void reconstruirArvores(NoCategoria *categoria) {
    construirArvores(categoria);
}

/* Constrói todas as árvores de todas as categorias (usado após leitura do BIN) */
void construirTodasAsArvores(NoCategoria *lista_categorias) {
    NoCategoria *atual = lista_categorias;
    while (atual != NULL) {
        construirArvores(atual);
        atual = atual->proximo;
    }
}

// --- Funções de Remoção (Requisito C.c.7 e C.c.8) ---

/* Remove um alimento específico (Requisito C.c.8) */
int removerAlimento(NoCategoria **lista_categorias, int codigo_alimento) {
    NoCategoria *cat_atual = *lista_categorias;
    
    // Loop: Percorre todas as categorias
    while (cat_atual != NULL) {
        NoAlimento *temp = cat_atual->alimentos;
        NoAlimento *anterior = NULL;
        
        // Busca o alimento
        while (temp != NULL && temp->dados.codigo != codigo_alimento) {
            anterior = temp;
            temp = temp->proximo;
        }
        
        if (temp != NULL) { // Se encontrou
            if (anterior == NULL) {
                // É o primeiro elemento
                cat_atual->alimentos = temp->proximo;
            } else {
                // Elemento no meio/fim
                anterior->proximo = temp->proximo;
            }
            free(temp);
            
            // Reconstroi as árvores (Requisito 8)
            reconstruirArvores(cat_atual); 
            return 1; // Sucesso na remoção
        }
        
        cat_atual = cat_atual->proximo;
    }
    
    return 0; // Alimento não encontrado
}

/* Remove uma categoria (Requisito C.c.7) */
int removerCategoria(NoCategoria **lista_categorias, Categoria id) {
    NoCategoria *atual = *lista_categorias;
    NoCategoria *anterior = NULL;

    // Loop: Busca a categoria pelo ID
    while (atual != NULL && atual->id != id) {
        anterior = atual;
        atual = atual->proximo;
    }

    // Categoria não encontrada
    if (atual == NULL) {
        return 0; 
    }

    // Remoção do nó
    if (anterior == NULL) {
        // Primeiro nó
        *lista_categorias = atual->proximo;
    } else {
        // Nó no meio ou fim
        anterior->proximo = atual->proximo;
    }

    // Libera todos os recursos da categoria removida
    liberarListaAlimentos(atual->alimentos);
    liberarArvore(atual->arvore_energia);
    liberarArvore(atual->arvore_proteina);
    free(atual);

    return 1; // Sucesso
}

// --- Funções de Busca e Listagem ---

/* Retorna o ponteiro para uma categoria dado o seu ID */
NoCategoria* buscarCategoriaPorId(NoCategoria *lista_categorias, Categoria id) {
    NoCategoria *atual = lista_categorias;
    while (atual != NULL) {
        if (atual->id == id) {
            return atual;
        }
        atual = atual->proximo;
    }
    return NULL;
}

/* Lista todas as categorias (Requisito C.c.1) */
void listarCategorias(NoCategoria *lista_categorias) {
    printf("\nLista de Categorias (Ordem Alfabética):\n");
    printf("+------+---------------------------------------------------+\n");
    printf("|  ID  | Nome da Categoria                                 |\n");
    printf("+------+---------------------------------------------------+\n");
    
    NoCategoria *atual = lista_categorias;
    int contador = 0;
    while (atual != NULL) {
        printf("| %4d | %-49.49s |\n", atual->id, atual->nome);
        atual = atual->proximo;
        contador++;
    }
    
    printf("+------+---------------------------------------------------+\n");
    printf("Total de %d categorias ativas.\n", contador);
}

/* Função auxiliar para imprimir a linha de alimento */
void imprimirLinhaAlimento(const Alimento *a) {
    printf("| %4d | %-35.35s | %7.2f | %8.2f |\n",
           a->codigo, a->nome, a->calorias, a->proteinas);
}

/* Imprime o cabeçalho da tabela de alimentos */
void imprimirCabecalhoTabela(void) {
    printf("\n+------+-------------------------------------+---------+----------+\n");
    printf("| Cód. | Nome do Alimento                    | Energia | Proteína |\n");
    printf("+------+-------------------------------------+---------+----------+\n");
}

/* Imprime o rodapé da tabela de alimentos */
void imprimirRodapeTabela(int total) {
    printf("+------+-------------------------------------+---------+----------+\n");
    printf("Total: %d alimento(s).\n", total);
}

/* Lista todos os alimentos de uma categoria na ordem da lista (alfabética) (Requisito C.c.2) */
void listarAlimentosPorLista(NoCategoria *categoria) {
    printf("\n--- Alimentos da Categoria: %s (Ordem Alfabética) ---\n", categoria->nome);
    imprimirCabecalhoTabela();
    
    NoAlimento *atual = categoria->alimentos;
    int contador = 0;
    
    while (atual != NULL) {
        imprimirLinhaAlimento(&(atual->dados));
        atual = atual->proximo;
        contador++;
    }
    imprimirRodapeTabela(contador);
}

/* Percorre a árvore em ordem decrescente (E-R-D) (Requisitos C.c.3 e C.c.4) */
void listarDecrescente(NoArvore *raiz) {
    if (raiz != NULL) {
        listarDecrescente(raiz->esquerda); // Visita a subárvore da esquerda (maiores valores)
        imprimirLinhaAlimento(&(raiz->ptr_alimento->dados)); // Visita o nó atual
        listarDecrescente(raiz->direita); // Visita a subárvore da direita (menores valores)
    }
}

/* Função auxiliar para a busca por intervalo que mantém o contador */
void buscarPorIntervalo_recursivo(NoArvore *raiz, float min, float max, int *contador_ptr) {
    if (raiz == NULL) return;

    // Passagem pela esquerda (maiores valores)
    // Se a chave no nó atual é maior ou igual ao mínimo, vale a pena buscar à esquerda
    if (raiz->chave >= min) {
        buscarPorIntervalo_recursivo(raiz->esquerda, min, max, contador_ptr);
    }
    
    // Visita a raiz
    if (raiz->chave >= min && raiz->chave <= max) {
        imprimirLinhaAlimento(&(raiz->ptr_alimento->dados));
        (*contador_ptr)++;
    }
    
    // Passagem pela direita (menores valores)
    // Se a chave no nó atual é menor ou igual ao máximo, vale a pena buscar à direita
    if (raiz->chave <= max) {
        buscarPorIntervalo_recursivo(raiz->direita, min, max, contador_ptr);
    }
}

/* Busca e lista alimentos em um intervalo (Requisitos C.c.5 e C.c.6) */
void buscarPorIntervalo(NoArvore *raiz, float min, float max, const char* nome_criterio) {
    printf("\n--- Busca por %s no Intervalo [%.2f, %.2f] ---\n", nome_criterio, min, max);
    imprimirCabecalhoTabela();
    
    int contador = 0;
    // Chama a função recursiva com o endereço do contador
    buscarPorIntervalo_recursivo(raiz, min, max, &contador);
    
    imprimirRodapeTabela(contador);
}

Aqui está o conteúdo do arquivo `file_io.c`.

```c
/* Conteúdo do arquivo file_io.c */

#include "file_io.h"
#include "file_format.h"
#include "formatter.c" // Inclui as implementações de string utilities
#include "data_structure.h" // Necessário para a função de inserção e construção

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#define TAMANHO_LINHA 512

/* Função auxiliar para ignorar o cabeçalho do CSV */
void ignorarPrimeiraLinha(FILE *arquivo_csv) {
    char linha[TAMANHO_LINHA];
    if (fgets(linha, sizeof(linha), arquivo_csv) == NULL) {
        printf("Aviso: O arquivo CSV está vazio ou o cabeçalho não foi lido.\n");
    }
}

/* Converte CSV para BIN (Função P1) */
int lerCSVpraBIN(const char *arquivoCSV, const char *arquivoBIN) {
    Alimento alimento;
    int contador = 0;
    FILE *arquivo_csv = NULL, *arquivo_binario = NULL;
    char linha[TAMANHO_LINHA];
    const char *delimitador = ";"; // Delimitador padrão do arquivo fornecido

    // 1. Abertura do CSV
    arquivo_csv = fopen(arquivoCSV, "r");
    if (arquivo_csv == NULL) {
        return -1;
    }

    ignorarPrimeiraLinha(arquivo_csv);

    // 2. Criação do BIN
    arquivo_binario = fopen(arquivoBIN, "wb");
    if (arquivo_binario == NULL) {
        fclose(arquivo_csv);
        return -1;
    }

    // Loop: Leitura linha por linha
    while (fgets(linha, sizeof(linha), arquivo_csv) != NULL) {
        removerQuebraLinha(linha);
        
        // Usa uma cópia da linha, pois strtok modifica a string original
        char linha_copia[TAMANHO_LINHA];
        strncpy(linha_copia, linha, TAMANHO_LINHA);
        linha_copia[TAMANHO_LINHA - 1] = '\0';
        
        char *token;
        int campo_atual = 0;
        int campos_lidos = 0; 
        
        // Zera a estrutura para garantir limpeza
        memset(&alimento, 0, sizeof(Alimento));
        alimento.categoria = CAT_INVALIDA; // Padrão
        alimento.codigo = 0; // Padrão

        token = strtok(linha, delimitador);
        // Loop: Tokenização da linha
        while (token) {
            removerQuebraLinha(token);
            removerAspas(token);
            
            // Tratamento dos campos: (Assumindo a ordem: ID;Nome;Calorias;Proteinas;Categoria)
            if (campo_atual == 0) { // ID
                substituirVirgula(token);
                alimento.codigo = atoi(token);
                if (alimento.codigo > 0) campos_lidos++;
            } else if (campo_atual == 1) { // Nome
                strncpy(alimento.nome, token, sizeof(alimento.nome) - 1);
                alimento.nome[sizeof(alimento.nome) - 1] = '\0';
                campos_lidos++;
            } else if (campo_atual == 2) { // Calorias (Energia)
                substituirVirgula(token);
                alimento.calorias = atof(token);
                campos_lidos++;
            } else if (campo_atual == 3) { // Proteína
                substituirVirgula(token);
                alimento.proteinas = atof(token);
                campos_lidos++;
            } else if (campo_atual == 4) { // Categoria
                alimento.categoria = nomeParaCategoria(token);
                if (alimento.categoria != CAT_INVALIDA) {
                    campos_lidos++;
                }
            }
            
            token = strtok(NULL, delimitador);
            campo_atual++;
        }

        // Condicional: Se leu 5 campos essenciais e a categoria é válida
        if (campos_lidos >= 5 && alimento.categoria != CAT_INVALIDA && alimento.codigo > 0) {
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
    
    if (contador == 10) {
        // Volta ao início e conta o total real
        fseek(arquivo, 0, SEEK_END);
        long tamanho = ftell(arquivo);
        contador = tamanho / sizeof(Alimento);
        printf("\nTotal de registros salvos: %ld (Apenas 10 foram mostrados acima).\n", tamanho / sizeof(Alimento));
    } else {
        printf("Total de registros lidos: %d\n", contador);
    }
    
    fclose(arquivo);
}

/* Função P2: Lê BIN para memória (Lista Ligada de Categorias) (Requisito C.a) */
int lerBINparaMemoria(const char *arquivoBIN, NoCategoria **lista_categorias) {
    FILE *arquivo = fopen(arquivoBIN, "rb");
    if (arquivo == NULL) {
        return 0; // 0 alimentos lidos
    }

    Alimento alimento_lido;
    int contador = 0;
    
    // Loop: lê cada registro do arquivo binário
    while (fread(&alimento_lido, sizeof(Alimento), 1, arquivo) == 1) {
        // Insere o alimento na estrutura de memória (Lista Ligada de Categorias e de Alimentos)
        if (inserirAlimento(lista_categorias, &alimento_lido)) {
            contador++;
        }
    }

    fclose(arquivo);
    return contador;
}

/* Função P2: Escreve memória (Lista Ligada) para BIN (Requisito C.c.9) */
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