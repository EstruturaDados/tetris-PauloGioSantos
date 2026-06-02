/*
 * ============================================================
 *  TETRIS STACK - NÍVEL NOVATO
 *  ByteBros - Jogos Educacionais de Lógica e Programação
 * ============================================================
 *  Descrição:
 *    Simula uma fila circular de 5 peças futuras do jogo
 *    Tetris Stack. O jogador pode:
 *      - Visualizar a fila atual
 *      - Jogar (remover) a peça da frente (dequeue)
 *      - Inserir uma nova peça no final (enqueue)
 *
 *  Conceitos trabalhados:
 *    - Fila circular com arrays
 *    - Structs para representação de peças
 *    - Funções e modularização
 *    - Entrada e saída via terminal
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ----------------------------------------------------------
 *  CONSTANTES
 * ---------------------------------------------------------- */
#define CAPACIDADE_FILA 5   /* número máximo de peças na fila */
#define NUM_TIPOS       4   /* quantidade de tipos de peça    */

/* ----------------------------------------------------------
 *  STRUCT: Peça
 *    nome  -> tipo da peça: 'I', 'O', 'T' ou 'L'
 *    id    -> identificador único de criação
 * ---------------------------------------------------------- */
typedef struct {
    char nome;
    int  id;
} Peca;

/* ----------------------------------------------------------
 *  STRUCT: Fila Circular
 *    pecas    -> array de peças
 *    frente   -> índice do primeiro elemento
 *    tras     -> índice do próximo slot de inserção
 *    tamanho  -> quantidade atual de elementos
 * ---------------------------------------------------------- */
typedef struct {
    Peca pecas[CAPACIDADE_FILA];
    int  frente;
    int  tras;
    int  tamanho;
} FilaCircular;

/* contador global de IDs; garante unicidade entre todas as peças geradas */
static int contadorId = 0;

/* tipos disponíveis de peça */
static const char tiposPeca[NUM_TIPOS] = { 'I', 'O', 'T', 'L' };

/* ----------------------------------------------------------
 *  gerarPeca
 *    Cria e retorna uma nova peça com tipo aleatório e
 *    id sequencial único.
 * ---------------------------------------------------------- */
Peca gerarPeca(void) {
    Peca novaPeca;
    novaPeca.nome = tiposPeca[rand() % NUM_TIPOS];
    novaPeca.id   = contadorId++;
    return novaPeca;
}

/* ----------------------------------------------------------
 *  inicializarFila
 *    Prepara a fila vazia e a preenche com CAPACIDADE_FILA
 *    peças geradas automaticamente.
 * ---------------------------------------------------------- */
void inicializarFila(FilaCircular *fila) {
    fila->frente  = 0;
    fila->tras    = 0;
    fila->tamanho = 0;

    /* preenche a fila com peças iniciais */
    for (int i = 0; i < CAPACIDADE_FILA; i++) {
        fila->pecas[fila->tras] = gerarPeca();
        fila->tras              = (fila->tras + 1) % CAPACIDADE_FILA;
        fila->tamanho++;
    }
}

/* ----------------------------------------------------------
 *  filaCheia
 *    Retorna 1 se a fila estiver cheia, 0 caso contrário.
 * ---------------------------------------------------------- */
int filaCheia(const FilaCircular *fila) {
    return fila->tamanho == CAPACIDADE_FILA;
}

/* ----------------------------------------------------------
 *  filaVazia
 *    Retorna 1 se a fila estiver vazia, 0 caso contrário.
 * ---------------------------------------------------------- */
int filaVazia(const FilaCircular *fila) {
    return fila->tamanho == 0;
}

/* ----------------------------------------------------------
 *  enqueue
 *    Insere uma nova peça gerada automaticamente no final
 *    da fila circular.
 *    Retorna 1 em caso de sucesso, 0 se a fila estiver cheia.
 * ---------------------------------------------------------- */
int enqueue(FilaCircular *fila) {
    if (filaCheia(fila)) {
        printf("  [AVISO] Fila cheia! Nao e possivel inserir nova peca.\n");
        return 0;
    }

    Peca novaPeca        = gerarPeca();
    fila->pecas[fila->tras] = novaPeca;
    fila->tras           = (fila->tras + 1) % CAPACIDADE_FILA;
    fila->tamanho++;

    printf("  Peca [%c %d] inserida no final da fila.\n",
           novaPeca.nome, novaPeca.id);
    return 1;
}

/* ----------------------------------------------------------
 *  dequeue
 *    Remove e retorna a peça da frente da fila circular.
 *    Retorna 1 em caso de sucesso, 0 se a fila estiver vazia.
 * ---------------------------------------------------------- */
int dequeue(FilaCircular *fila, Peca *pecaRemovida) {
    if (filaVazia(fila)) {
        printf("  [AVISO] Fila vazia! Nao ha pecas para jogar.\n");
        return 0;
    }

    *pecaRemovida  = fila->pecas[fila->frente];
    fila->frente   = (fila->frente + 1) % CAPACIDADE_FILA;
    fila->tamanho--;

    printf("  Peca [%c %d] jogada (removida da frente).\n",
           pecaRemovida->nome, pecaRemovida->id);
    return 1;
}

/* ----------------------------------------------------------
 *  exibirFila
 *    Mostra no terminal todas as peças da fila, da frente
 *    para o final, no formato: [tipo id]
 * ---------------------------------------------------------- */
void exibirFila(const FilaCircular *fila) {
    printf("\n  === Fila de Pecas (%d/%d) ===\n  ",
           fila->tamanho, CAPACIDADE_FILA);

    if (filaVazia(fila)) {
        printf("(fila vazia)\n");
        return;
    }

    for (int i = 0; i < fila->tamanho; i++) {
        int indice = (fila->frente + i) % CAPACIDADE_FILA;
        printf("[%c %d] ", fila->pecas[indice].nome, fila->pecas[indice].id);
    }
    printf("\n");
}

/* ----------------------------------------------------------
 *  exibirMenu
 *    Exibe as opções disponíveis ao jogador.
 * ---------------------------------------------------------- */
void exibirMenu(void) {
    printf("\n  +-----------------------------+\n");
    printf("  |       TETRIS STACK          |\n");
    printf("  |       Nivel: Novato         |\n");
    printf("  +-----------------------------+\n");
    printf("  | 1 - Jogar peca  (dequeue)   |\n");
    printf("  | 2 - Inserir peca (enqueue)  |\n");
    printf("  | 0 - Sair                    |\n");
    printf("  +-----------------------------+\n");
    printf("  Escolha: ");
}

/* ----------------------------------------------------------
 *  main
 *    Ponto de entrada: inicializa o jogo e gerencia o loop
 *    principal de interação com o jogador.
 * ---------------------------------------------------------- */
int main(void) {
    srand((unsigned int)time(NULL)); /* semente para números aleatórios */

    FilaCircular fila;
    inicializarFila(&fila);

    printf("\n  Bem-vindo ao TETRIS STACK - Nivel Novato!\n");
    printf("  Fila inicializada com %d pecas.\n", CAPACIDADE_FILA);
    exibirFila(&fila);

    int opcao;
    Peca pecaJogada;

    do {
        exibirMenu();
        if (scanf("%d", &opcao) != 1) {
            /* descarta entrada inválida */
            while (getchar() != '\n');
            opcao = -1;
        }

        printf("\n");

        switch (opcao) {
            case 1: /* jogar peça: remove da frente */
                dequeue(&fila, &pecaJogada);
                break;

            case 2: /* inserir nova peça: adiciona no final */
                enqueue(&fila);
                break;

            case 0: /* encerrar o programa */
                printf("  Encerrando o jogo. Ate logo!\n");
                break;

            default:
                printf("  [ERRO] Opcao invalida. Tente novamente.\n");
                break;
        }

        if (opcao != 0) {
            exibirFila(&fila);
        }

    } while (opcao != 0);

    return 0;
}
