#include "data_structures.h"
#include "file_format.h" // Se Alimento for referenciado

// Array de nomes de categorias para a função obterNomeCategoria
const char* NOME_CATEGORIAS[] = {
    "DESCONHECIDA",
    "CEREAIS", "VERDURAS", "FRUTAS", "GORDURAS", "PESCADOS",
    "CARNES", "LACTEOS", "BEBIDAS", "OVOS", "ACUCARES",
    "MISCELANEAS", "INDUSTRIALIZADOS", "PREPARADOS", "LEGUMINOSAS", "SEMENTES"
};

/**
 * Retorna o nome da categoria.
 * (STUB - Funcionalidade real seria implementada aqui)
 */
const char* obterNomeCategoria(Categoria cat) {
    // MAX_CATEGORIAS é definido em data_structures.h como 15.
    // O array tem 16 posições (0 a 15).
    if (cat >= 0 && cat <= NUM_CATEGORIAS) {
        return NOME_CATEGORIAS[cat];
    }
    return NOME_CATEGORIAS[CATEGORIA_DESCONHECIDA];
}

/**
 * Insere um alimento na estrutura de lista de categorias.
 * Esta é uma versão STUB, mas crucial para a compilação do file_io.c.
 * A implementação real deve lidar com a criação de NoCategoria, 
 * NoAlimento e a inserção nas árvores.
 */
int inserirAlimento(NoCategoria **lista_categorias_head, AlimentoDados *dados) {
    // Nesta versão stub, apenas simulamos o sucesso da inserção.
    // A implementação real virá aqui.
    // printf("STUB: Alimento %s inserido (Cód: %d).\n", dados->descricao, dados->codigo);
    return 1; // Retorna 1 (sucesso) para não bloquear a leitura do binário no file_io.c
}

// Stubs para outras funções de utilidade (se o main.c precisar delas para compilar)
Categoria intParaCategoria(int cat_num) {
    if (cat_num >= 0 && cat_num <= NUM_CATEGORIAS) return (Categoria)cat_num;
    return CATEGORIA_DESCONHECIDA;
}
void limparBufferEntrada(void) {}
void pausar(void) {}
void exibirMenu(void) {}
void reconstruirArvoresCategoria(NoCategoria *no_cat) {}
// Protótipo precisa de ponteiro duplo para o anterior
NoCategoria* buscarCategoriaPorID(NoCategoria *head, Categoria id, NoCategoria **anterior) { return NULL; }
void liberarListaCategorias(NoCategoria *head) {}