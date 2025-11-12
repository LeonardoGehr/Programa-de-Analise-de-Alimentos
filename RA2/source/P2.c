#include "data_structure.h"
#include "file_format.h"
#include "file_reading.h" // Inclui as funções de leitura/escrita BIN/Memória
#include "formatter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define ARQUIVO_BIN "alimentos.bin"

// --- Protótipos da Interface de Menu ---
void exibirMenu(void);
void adicionarNovoAlimento(NoCategoria **lista_categorias);
void processarOpcao(int opcao, NoCategoria **lista_categorias);
int lerInteiroPositivo(const char *mensagem);
float lerFloatPositivo(const char *mensagem);

int main() {
    // Configura o locale para permitir acentuação e formatação correta em português
    setlocale(LC_ALL, "pt_BR.utf8");
    
    NoCategoria *lista_categorias = NULL; // Ponteiro para o início da lista de categorias
    int opcao;
    int total_lido = 0;

    printf("=== Processo P2: Manipulação e Consultas de Dados Nutricionais ===\n");

    // 1. Leitura do arquivo BIN para a memória (Requisito C.a)
    printf("Carregando dados do arquivo binário (%s) para a memória...\n", ARQUIVO_BIN);
    total_lido = lerBINparaMemoria(ARQUIVO_BIN, &lista_categorias);
    
    if (total_lido > 0) {
        printf("Sucesso! %d alimentos carregados e organizados nas listas.\n", total_lido);
        
        // 2. Construção das árvores de indexação (Requisito C.b)
        printf("Construindo árvores de indexação (Energia e Proteína) para cada categoria...\n");
        construirTodasAsArvores(lista_categorias);
        printf("Árvores construídas com sucesso.\n");

        // 3. Loop do Menu Interativo
        do {
            exibirMenu();
            printf("Escolha uma opção: ");
            if (scanf("%d", &opcao) != 1) {
                // Limpa o buffer em caso de entrada não numérica
                while (getchar() != '\n');
                opcao = -1; // Garante que caia no default e repita
            }
            processarOpcao(opcao, &lista_categorias);
        } while (opcao != 0);

        // 4. Saída: Salvar e liberar memória (Requisito C.c.9)
        printf("\n--- Salvando Dados ---\n");
        int total_salvo = escreverMemoriaParaBIN(ARQUIVO_BIN, lista_categorias);
        if (total_salvo > 0) {
             printf("Sucesso! %d alimentos salvos de volta no arquivo BIN.\n", total_salvo);
        } else if (total_salvo == 0) {
             printf("Aviso: Nenhum dado foi salvo. A lista de alimentos estava vazia.\n");
        } else {
             printf("ERRO: Falha ao salvar os dados de volta no arquivo BIN.\n");
        }
        
        printf("\nLiberando memória alocada...\n");
        liberarListaCategorias(lista_categorias);
        printf("Estruturas liberadas. Programa finalizado.\n");

    } else {
        printf("ERRO: Falha ao carregar dados. O arquivo BIN pode estar vazio ou inexistente. (Total lido: %d)\n", total_lido);
    }

    return 0;
}

// --- Implementação das Funções de Menu e Utilitárias ---

void exibirMenu(void) {
    printf("\n======================================================\n");
    printf("                  MENU DE OPERAÇÕES P2\n");
    printf("======================================================\n");
    printf("  [C.c] Consultas e Listagens:\n");
    printf("  1. Listar Categorias (Req. 1)\n");
    printf("  2. Listar Alimentos de uma Categoria (Ordem Alfabética, Req. 2)\n");
    printf("  3. Listar Alimentos de uma Categoria (Ordem Decrescente de Energia, Req. 3)\n");
    printf("  4. Listar Alimentos de uma Categoria (Ordem Decrescente de Proteína, Req. 4)\n");
    printf("  5. Buscar Alimentos por Intervalo de Energia (Req. 5)\n");
    printf("  6. Buscar Alimentos por Intervalo de Proteína (Req. 6)\n");
    printf("------------------------------------------------------\n");
    printf("  [C.c] Inserção e Remoção:\n");
    printf("  7. Remover uma Categoria e todos seus Alimentos (Req. 7)\n");
    printf("  8. Remover um Alimento específico (Req. 8)\n");
    printf("  9. Adicionar um Novo Alimento\n");
    printf("------------------------------------------------------\n");
    printf("  0. Sair e Salvar Alterações (Req. 9)\n");
    printf("======================================================\n");
}

/* Função para ler inteiros não negativos com validação */
int lerInteiroPositivo(const char *mensagem) {
    int valor;
    while (1) {
        printf("%s", mensagem);
        if (scanf("%d", &valor) == 1) {
            if (valor >= 0) {
                // Limpa o buffer de entrada
                while (getchar() != '\n');
                return valor;
            } else {
                printf("Valor deve ser não negativo. Tente novamente.\n");
            }
        } else {
            printf("Entrada inválida. Digite um número inteiro.\n");
            // Limpa o buffer de entrada
            while (getchar() != '\n');
        }
    }
}

/* Função para ler floats não negativos com validação */
float lerFloatPositivo(const char *mensagem) {
    float valor;
    while (1) {
        printf("%s", mensagem);
        if (scanf("%f", &valor) == 1) {
            if (valor >= 0.0f) {
                // Limpa o buffer de entrada
                while (getchar() != '\n');
                return valor;
            } else {
                printf("Valor deve ser não negativo. Tente novamente.\n");
            }
        } else {
            printf("Entrada inválida. Digite um número decimal (float).\n");
            // Limpa o buffer de entrada
            while (getchar() != '\n');
        }
    }
}

/* Adiciona um novo alimento à estrutura de dados (Menu 9) */
void adicionarNovoAlimento(NoCategoria **lista_categorias) {
    Alimento novo_alimento;
    memset(&novo_alimento, 0, sizeof(Alimento));
    char nome_cat_str[TAM_NOME_CATEGORIA];
    
    printf("\n--- Inserir Novo Alimento ---\n");
    
    // 1. Código
    novo_alimento.codigo = lerInteiroPositivo("Digite o Código do Alimento (ID): ");
    
    // 2. Nome
    printf("Digite o Nome do Alimento: ");
    if (fgets(novo_alimento.nome, sizeof(novo_alimento.nome), stdin) != NULL) {
        removerQuebraLinha(novo_alimento.nome);
    } else {
        printf("Erro na leitura do nome.\n");
        return;
    }
    
    // 3. Energia e Proteína
    novo_alimento.calorias = lerFloatPositivo("Digite o valor de Energia (Calorias): ");
    novo_alimento.proteinas = lerFloatPositivo("Digite o valor de Proteína: ");

    // 4. Categoria
    printf("Digite o Nome da Categoria (ex: Cereais, Pescados): ");
    if (fgets(nome_cat_str, sizeof(nome_cat_str), stdin) != NULL) {
        removerQuebraLinha(nome_cat_str);
    } else {
        printf("Erro na leitura da categoria.\n");
        return;
    }
    
    novo_alimento.categoria = nomeParaCategoria(nome_cat_str);
    if (novo_alimento.categoria == CAT_INVALIDA) {
        printf("Categoria digitada não é válida. Inserção cancelada.\n");
        return;
    }
    
    // 5. Inserção na Lista Ligada de Categorias e Alimentos
    if (inserirAlimento(lista_categorias, &novo_alimento)) {
        printf("\nAlimento '%s' inserido com sucesso na lista de '%s'.\n", 
               novo_alimento.nome, obterNomeCategoria(novo_alimento.categoria));
        
        // 6. Atualiza as Árvores da Categoria Afetada
        NoCategoria *cat_afetada = buscarCategoriaPorId(*lista_categorias, novo_alimento.categoria);
        if (cat_afetada) {
            // Reconstrução simples das árvores da categoria afetada após a inserção
            construirArvores(cat_afetada);
            printf("Árvores de indexação da categoria atualizadas.\n");
        }
        
    } else {
        printf("Falha na inserção do alimento. (Pode ser código duplicado ou erro de alocação)\n");
    }
}


/* Processa a opção escolhida pelo usuário */
void processarOpcao(int opcao, NoCategoria **lista_categorias) {
    Categoria cat_id;
    NoCategoria *categoria;
    int codigo_alimento;
    float min_val, max_val;

    switch (opcao) {
        case 1: // Listar Categorias (Req. 1)
            listarCategorias(*lista_categorias);
            break;

        case 2: // Listar Alimentos (Ordem Alfabética, Req. 2)
        case 3: // Listar Alimentos (Ordem Decrescente Energia, Req. 3)
        case 4: { // Listar Alimentos (Ordem Decrescente Proteína, Req. 4)
            cat_id = (Categoria)lerInteiroPositivo("Digite o ID da Categoria a listar: ");
            categoria = buscarCategoriaPorId(*lista_categorias, cat_id);
            
            if (categoria == NULL) {
                printf("Erro: Categoria com ID %d não encontrada.\n", cat_id);
                break;
            }

            if (opcao == 2) {
                listarAlimentosPorLista(categoria);
            } else if (opcao == 3) {
                printf("\n--- Alimentos da Categoria: %s (Decrescente de Energia) ---\n", categoria->nome);
                imprimirCabecalhoTabela();
                listarDecrescente(categoria->arvore_energia);
                imprimirRodapeTabela(-1); 
            } else if (opcao == 4) {
                printf("\n--- Alimentos da Categoria: %s (Decrescente de Proteína) ---\n", categoria->nome);
                imprimirCabecalhoTabela();
                listarDecrescente(categoria->arvore_proteina);
                imprimirRodapeTabela(-1);
            }
            break;
        }

        case 5: // Buscar por Intervalo de Energia (Req. 5)
        case 6: { // Buscar por Intervalo de Proteína (Req. 6)
            cat_id = (Categoria)lerInteiroPositivo("Digite o ID da Categoria para buscar: ");
            categoria = buscarCategoriaPorId(*lista_categorias, cat_id);
            
            if (categoria == NULL) {
                printf("Erro: Categoria com ID %d não encontrada.\n", cat_id);
                break;
            }
            
            min_val = lerFloatPositivo("Digite o valor MÍNIMO do intervalo: ");
            max_val = lerFloatPositivo("Digite o valor MÁXIMO do intervalo: ");
            
            if (min_val > max_val) {
                printf("Erro: O valor mínimo não pode ser maior que o valor máximo.\n");
                break;
            }

            if (opcao == 5) {
                buscarPorIntervalo(categoria->arvore_energia, min_val, max_val, "Energia (Calorias)");
            } else {
                buscarPorIntervalo(categoria->arvore_proteina, min_val, max_val, "Proteína");
            }
            break;
        }

        case 7: { // Remover Categoria (Req. 7)
            cat_id = (Categoria)lerInteiroPositivo("Digite o ID da Categoria a ser removida: ");
            if (removerCategoria(lista_categorias, cat_id)) {
                printf("Sucesso: Categoria ID %d e todos os seus alimentos foram removidos.\n", cat_id);
            } else {
                printf("Erro: Categoria com ID %d não encontrada.\n", cat_id);
            }
            break;
        }

        case 8: { // Remover Alimento (Req. 8)
            codigo_alimento = lerInteiroPositivo("Digite o Código do Alimento a ser removido: ");
            if (removerAlimento(lista_categorias, codigo_alimento)) {
                printf("Sucesso: Alimento com código %d foi removido e árvores reconstruídas.\n", codigo_alimento);
            } else {
                printf("Erro: Alimento com código %d não encontrado em nenhuma categoria.\n", codigo_alimento);
            }
            break;
        }

        case 9: // Adicionar Novo Alimento
            adicionarNovoAlimento(lista_categorias);
            break;

        case 0:
            printf("Opção Sair selecionada. Preparando-se para salvar dados e finalizar.\n");
            break;
            
        default:
            printf("Opção inválida. Tente novamente.\n");
    }
}