/*
 * ============================================================
 *  TETRIS STACK - NÍVEL AVENTUREIRO
 *  ByteBros - Jogos Educacionais de Lógica e Programação
 * ============================================================
 *  Descrição:
 *    Expande o Nível Novato adicionando uma pilha linear de
 *    reserva (capacidade máxima: 3 peças). O jogador pode:
 *      - Visualizar fila e pilha
 *      - Jogar a peça da frente (dequeue)
 *      - Reservar a peça da frente na pilha (push)
 *      - Usar a peça do topo da pilha (pop) → insere na frente
 *      - Inserir nova peça automática no final da fila
 *
 *  Novidades em relação ao Nível Novato:
 *    - Introdução da pilha linear de reserva
 *    - A fila permanece sempre cheia com 5 peças
 *    - Menu expandido com 4 opções
 *
 *  Conceitos trabalhados:
 *    - Fila circular com arrays
 *    - Pilha linear com array
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
#define CAPACIDADE_FILA  5   /* número máximo de peças na fila   */
#define CAPACIDADE_PILHA 3   /* número máximo de peças na pilha  */
#define NUM_TIPOS        4   /* quantidade de tipos de peça      */

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

/* ----------------------------------------------------------
 *  STRUCT: Pilha Linear
 *    pecas  -> array de peças
 *    topo   -> índice do elemento no topo (-1 = vazia)
 * ---------------------------------------------------------- */
typedef struct {
    Peca pecas[CAPACIDADE_PILHA];
    int  topo;
} Pilha;

/* contador global de IDs; garante unicidade entre todas as peças geradas */
static int contadorId = 0;

/* tipos disponíveis de peça */
static const char tiposPeca[NUM_TIPOS] = { 'I', 'O', 'T', 'L' };

/* ============================================================
 *  FUNÇÕES AUXILIARES DE GERAÇÃO
 * ============================================================ */

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

/* ============================================================
 *  FUNÇÕES DA FILA CIRCULAR
 * ============================================================ */

/* ----------------------------------------------------------
 *  inicializarFila
 *    Prepara a fila vazia e a preenche com CAPACIDADE_FILA
 *    peças geradas automaticamente.
 * ---------------------------------------------------------- */
void inicializarFila(FilaCircular *fila) {
    fila->frente  = 0;
    fila->tras    = 0;
    fila->tamanho = 0;

    for (int i = 0; i < CAPACIDADE_FILA; i++) {
        fila->pecas[fila->tras] = gerarPeca();
        fila->tras              = (fila->tras + 1) % CAPACIDADE_FILA;
        fila->tamanho++;
    }
}

/* ----------------------------------------------------------
 *  filaCheia / filaVazia
 * ---------------------------------------------------------- */
int filaCheia(const FilaCircular *fila) {
    return fila->tamanho == CAPACIDADE_FILA;
}

int filaVazia(const FilaCircular *fila) {
    return fila->tamanho == 0;
}

/* ----------------------------------------------------------
 *  enqueue
 *    Insere nova peça gerada automaticamente no final da fila.
 *    Retorna 1 em sucesso, 0 se cheia.
 * ---------------------------------------------------------- */
int enqueue(FilaCircular *fila) {
    if (filaCheia(fila)) {
        printf("  [AVISO] Fila cheia! Nao e possivel inserir nova peca.\n");
        return 0;
    }

    Peca novaPeca               = gerarPeca();
    fila->pecas[fila->tras]     = novaPeca;
    fila->tras                  = (fila->tras + 1) % CAPACIDADE_FILA;
    fila->tamanho++;

    printf("  Peca [%c %d] inserida automaticamente no final da fila.\n",
           novaPeca.nome, novaPeca.id);
    return 1;
}

/* ----------------------------------------------------------
 *  enqueueFrente
 *    Insere uma peça específica na FRENTE da fila circular.
 *    Usado ao recuperar peça da pilha de reserva.
 *    Retorna 1 em sucesso, 0 se cheia.
 * ---------------------------------------------------------- */
int enqueueFrente(FilaCircular *fila, Peca peca) {
    if (filaCheia(fila)) {
        printf("  [AVISO] Fila cheia! Nao e possivel inserir peca reservada.\n");
        return 0;
    }

    /* move frente um passo para trás no círculo */
    fila->frente = (fila->frente - 1 + CAPACIDADE_FILA) % CAPACIDADE_FILA;
    fila->pecas[fila->frente] = peca;
    fila->tamanho++;
    return 1;
}

/* ----------------------------------------------------------
 *  dequeue
 *    Remove e devolve a peça da frente.
 *    Retorna 1 em sucesso, 0 se vazia.
 * ---------------------------------------------------------- */
int dequeue(FilaCircular *fila, Peca *pecaRemovida) {
    if (filaVazia(fila)) {
        printf("  [AVISO] Fila vazia! Nao ha pecas para jogar.\n");
        return 0;
    }

    *pecaRemovida  = fila->pecas[fila->frente];
    fila->frente   = (fila->frente + 1) % CAPACIDADE_FILA;
    fila->tamanho--;
    return 1;
}

/* ----------------------------------------------------------
 *  exibirFila
 *    Mostra todas as peças da fila: frente → final.
 * ---------------------------------------------------------- */
void exibirFila(const FilaCircular *fila) {
    printf("  Fila (%d/%d): ", fila->tamanho, CAPACIDADE_FILA);

    if (filaVazia(fila)) {
        printf("(vazia)\n");
        return;
    }

    for (int i = 0; i < fila->tamanho; i++) {
        int idx = (fila->frente + i) % CAPACIDADE_FILA;
        printf("[%c %d] ", fila->pecas[idx].nome, fila->pecas[idx].id);
    }
    printf("\n");
}

/* ============================================================
 *  FUNÇÕES DA PILHA LINEAR
 * ============================================================ */

/* ----------------------------------------------------------
 *  inicializarPilha
 *    Inicializa a pilha como vazia (topo = -1).
 * ---------------------------------------------------------- */
void inicializarPilha(Pilha *pilha) {
    pilha->topo = -1;
}

/* ----------------------------------------------------------
 *  pilhaCheia / pilhaVazia
 * ---------------------------------------------------------- */
int pilhaCheia(const Pilha *pilha) {
    return pilha->topo == CAPACIDADE_PILHA - 1;
}

int pilhaVazia(const Pilha *pilha) {
    return pilha->topo == -1;
}

/* ----------------------------------------------------------
 *  push
 *    Empilha a peça da frente da fila na pilha de reserva.
 *    Após reservar, insere automaticamente nova peça na fila
 *    para mantê-la sempre cheia.
 *    Retorna 1 em sucesso, 0 em falha.
 * ---------------------------------------------------------- */
int push(Pilha *pilha, FilaCircular *fila) {
    if (pilhaCheia(pilha)) {
        printf("  [AVISO] Pilha cheia! Nao e possivel reservar mais pecas.\n");
        return 0;
    }

    Peca pecaDaFrente;
    if (!dequeue(fila, &pecaDaFrente)) {
        return 0;
    }

    pilha->topo++;
    pilha->pecas[pilha->topo] = pecaDaFrente;

    printf("  Peca [%c %d] reservada na pilha.\n",
           pecaDaFrente.nome, pecaDaFrente.id);

    /* mantém a fila sempre cheia após retirada */
    enqueue(fila);
    return 1;
}

/* ----------------------------------------------------------
 *  pop
 *    Retira a peça do topo da pilha e a insere na frente
 *    da fila (para ser a próxima a ser jogada).
 *    Retorna 1 em sucesso, 0 em falha.
 * ---------------------------------------------------------- */
int pop(Pilha *pilha, FilaCircular *fila) {
    if (pilhaVazia(pilha)) {
        printf("  [AVISO] Pilha vazia! Nao ha pecas reservadas.\n");
        return 0;
    }

    if (filaCheia(fila)) {
        printf("  [AVISO] Fila cheia! Remova uma peca antes de usar a reserva.\n");
        return 0;
    }

    Peca pecaReservada = pilha->pecas[pilha->topo];
    pilha->topo--;

    /* insere a peça reservada na frente da fila */
    enqueueFrente(fila, pecaReservada);

    printf("  Peca reservada [%c %d] retornada para a frente da fila.\n",
           pecaReservada.nome, pecaReservada.id);
    return 1;
}

/* ----------------------------------------------------------
 *  exibirPilha
 *    Mostra todas as peças da pilha: topo → base.
 * ---------------------------------------------------------- */
void exibirPilha(const Pilha *pilha) {
    printf("  Pilha de reserva (%d/%d): ",
           pilha->topo + 1, CAPACIDADE_PILHA);

    if (pilhaVazia(pilha)) {
        printf("(vazia)\n");
        return;
    }

    /* exibe do topo para a base */
    for (int i = pilha->topo; i >= 0; i--) {
        printf("[%c %d] ", pilha->pecas[i].nome, pilha->pecas[i].id);
    }
    printf("(base)\n");
}

/* ============================================================
 *  INTERFACE COM O JOGADOR
 * ============================================================ */

/* ----------------------------------------------------------
 *  exibirEstado
 *    Exibe o estado completo: fila e pilha de reserva.
 * ---------------------------------------------------------- */
void exibirEstado(const FilaCircular *fila, const Pilha *pilha) {
    printf("\n  ----------------------------------------\n");
    exibirFila(fila);
    exibirPilha(pilha);
    printf("  ----------------------------------------\n");
}

/* ----------------------------------------------------------
 *  exibirMenu
 *    Exibe as opções disponíveis ao jogador.
 * ---------------------------------------------------------- */
void exibirMenu(void) {
    printf("\n  +-------------------------------+\n");
    printf("  |       TETRIS STACK            |\n");
    printf("  |       Nivel: Aventureiro      |\n");
    printf("  +-------------------------------+\n");
    printf("  | 1 - Jogar peca  (dequeue)     |\n");
    printf("  | 2 - Reservar peca (push)      |\n");
    printf("  | 3 - Usar peca reservada (pop) |\n");
    printf("  | 0 - Sair                      |\n");
    printf("  +-------------------------------+\n");
    printf("  Escolha: ");
}

/* ----------------------------------------------------------
 *  main
 *    Ponto de entrada: inicializa jogo e gerencia o loop
 *    principal de interação com o jogador.
 * ---------------------------------------------------------- */
int main(void) {
    srand((unsigned int)time(NULL));

    FilaCircular fila;
    Pilha        pilha;

    inicializarFila(&fila);
    inicializarPilha(&pilha);

    printf("\n  Bem-vindo ao TETRIS STACK - Nivel Aventureiro!\n");
    printf("  Fila inicializada com %d pecas. Pilha de reserva vazia.\n",
           CAPACIDADE_FILA);
    exibirEstado(&fila, &pilha);

    int  opcao;
    Peca pecaJogada;

    do {
        exibirMenu();
        if (scanf("%d", &opcao) != 1) {
            while (getchar() != '\n');
            opcao = -1;
        }
        printf("\n");

        switch (opcao) {
            case 1: /* jogar peça + reposição automática */
                if (dequeue(&fila, &pecaJogada)) {
                    printf("  Peca [%c %d] jogada!\n",
                           pecaJogada.nome, pecaJogada.id);
                    enqueue(&fila); /* mantém fila cheia */
                }
                break;

            case 2: /* reservar peça da frente na pilha */
                push(&pilha, &fila);
                break;

            case 3: /* usar peça do topo da pilha */
                pop(&pilha, &fila);
                break;

            case 0:
                printf("  Encerrando o jogo. Ate logo!\n");
                break;

            default:
                printf("  [ERRO] Opcao invalida. Tente novamente.\n");
                break;
        }

        if (opcao != 0) {
            exibirEstado(&fila, &pilha);
        }

    } while (opcao != 0);

    return 0;
}
