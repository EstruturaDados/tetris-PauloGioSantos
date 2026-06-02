/*
 * ============================================================
 *  TETRIS STACK - NÍVEL MESTRE
 *  ByteBros - Jogos Educacionais de Lógica e Programação
 * ============================================================
 *  Descrição:
 *    Expande o Nível Aventureiro com operações estratégicas
 *    avançadas entre fila circular e pilha de reserva:
 *      1 - Jogar a peça da frente (dequeue + reposição automática)
 *      2 - Reservar peça na pilha (push)
 *      3 - Usar peça reservada (pop → frente da fila)
 *      4 - Trocar topo da pilha com frente da fila
 *      5 - Desfazer última ação (undo via histórico de snapshots)
 *      6 - Inverter a fila circular
 *
 *  Novidades em relação ao Nível Aventureiro:
 *    - Operação de troca direta entre pilha e fila
 *    - Histórico de desfazer (pilha de snapshots do estado completo)
 *    - Inversão completa da ordem da fila
 *
 *  Conceitos trabalhados:
 *    - Fila circular com array (reaproveitamento de espaço)
 *    - Pilha linear de reserva
 *    - Pilha auxiliar de histórico (undo)
 *    - Cópia de estado com memcpy
 *    - Modularização clara de funções
 *    - Entrada/saída via terminal
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ----------------------------------------------------------
 *  CONSTANTES
 * ---------------------------------------------------------- */
#define CAPACIDADE_FILA      5   /* máximo de peças na fila circular      */
#define CAPACIDADE_PILHA     3   /* máximo de peças na pilha de reserva   */
#define CAPACIDADE_HISTORICO 20  /* máximo de snapshots no histórico undo */
#define NUM_TIPOS            4   /* quantidade de tipos de peça           */

/* ----------------------------------------------------------
 *  ENUM: Tipo de Ação — identifica qual operação gerou um
 *        snapshot no histórico de desfazer.
 * ---------------------------------------------------------- */
typedef enum {
    ACAO_JOGAR,    /* dequeue + enqueue de nova peça          */
    ACAO_RESERVAR, /* push: da frente da fila para a pilha    */
    ACAO_USAR,     /* pop: do topo da pilha para a fila       */
    ACAO_TROCAR,   /* swap: topo da pilha <-> frente da fila  */
    ACAO_INVERTER  /* inversão completa da fila               */
} TipoAcao;

/* ----------------------------------------------------------
 *  STRUCT: Peça
 *    nome -> tipo da peça: 'I', 'O', 'T' ou 'L'
 *    id   -> identificador único sequencial
 * ---------------------------------------------------------- */
typedef struct {
    char nome;
    int  id;
} Peca;

/* ----------------------------------------------------------
 *  STRUCT: Fila Circular
 *    pecas   -> array de peças (buffer circular)
 *    frente  -> índice da próxima peça a ser removida
 *    tras    -> índice do próximo slot de inserção
 *    tamanho -> quantidade atual de peças
 * ---------------------------------------------------------- */
typedef struct {
    Peca pecas[CAPACIDADE_FILA];
    int  frente;
    int  tras;
    int  tamanho;
} FilaCircular;

/* ----------------------------------------------------------
 *  STRUCT: Pilha Linear de Reserva
 *    pecas -> array de peças
 *    topo  -> índice do elemento no topo (-1 = vazia)
 * ---------------------------------------------------------- */
typedef struct {
    Peca pecas[CAPACIDADE_PILHA];
    int  topo;
} Pilha;

/* ----------------------------------------------------------
 *  STRUCT: Snapshot
 *    Cópia completa do estado (fila + pilha) salva antes de
 *    cada operação, usada pelo mecanismo de desfazer.
 * ---------------------------------------------------------- */
typedef struct {
    /* estado da fila */
    Peca filaSnapshot[CAPACIDADE_FILA];
    int  filaFrente;
    int  filaTras;
    int  filaTamanho;

    /* estado da pilha de reserva */
    Peca pilhaSnapshot[CAPACIDADE_PILHA];
    int  pilhaTopo;

    TipoAcao acao; /* qual operação gerou este snapshot */
} Snapshot;

/* ----------------------------------------------------------
 *  STRUCT: Histórico de Desfazer
 *    Pilha de snapshots — o último salvo é o mais recente.
 * ---------------------------------------------------------- */
typedef struct {
    Snapshot entradas[CAPACIDADE_HISTORICO];
    int      topo; /* -1 = vazio */
} Historico;

/* ============================================================
 *  VARIÁVEIS GLOBAIS
 * ============================================================ */

/* contador global de IDs; garante unicidade entre todas as peças */
static int contadorId = 0;

/* tipos disponíveis de peça */
static const char tiposPeca[NUM_TIPOS] = { 'I', 'O', 'T', 'L' };

/* nomes legíveis para os tipos de ação (usados no undo) */
static const char *nomeAcao[] = {
    "Jogar peca",
    "Reservar peca",
    "Usar peca reservada",
    "Trocar peca",
    "Inverter fila"
};

/* ============================================================
 *  GERAÇÃO DE PEÇAS
 * ============================================================ */

/*
 * gerarPeca
 *   Cria uma nova peça com tipo aleatório e id único crescente.
 */
Peca gerarPeca(void) {
    Peca p;
    p.nome = tiposPeca[rand() % NUM_TIPOS];
    p.id   = contadorId++;
    return p;
}

/* ============================================================
 *  FILA CIRCULAR
 * ============================================================ */

/*
 * inicializarFila
 *   Zera os índices e preenche a fila com CAPACIDADE_FILA peças.
 */
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

/* filaCheia: retorna 1 se não há mais espaço disponível */
int filaCheia(const FilaCircular *fila) {
    return fila->tamanho == CAPACIDADE_FILA;
}

/* filaVazia: retorna 1 se não há peças na fila */
int filaVazia(const FilaCircular *fila) {
    return fila->tamanho == 0;
}

/*
 * enqueue
 *   Gera uma nova peça automaticamente e a insere no final.
 *   Retorna 1 em sucesso, 0 se a fila estiver cheia.
 */
int enqueue(FilaCircular *fila) {
    if (filaCheia(fila)) {
        printf("  [AVISO] Fila cheia! Nao e possivel inserir nova peca.\n");
        return 0;
    }

    Peca p = gerarPeca();
    fila->pecas[fila->tras] = p;
    fila->tras              = (fila->tras + 1) % CAPACIDADE_FILA;
    fila->tamanho++;

    printf("  Peca [%c %d] inserida automaticamente na fila.\n", p.nome, p.id);
    return 1;
}

/*
 * enqueueFrente
 *   Insere uma peça específica na FRENTE da fila (usando o slot
 *   circular anterior). Usado ao recuperar peça da pilha.
 *   Retorna 1 em sucesso, 0 se a fila estiver cheia.
 */
int enqueueFrente(FilaCircular *fila, Peca peca) {
    if (filaCheia(fila)) {
        printf("  [AVISO] Fila cheia! Nao e possivel inserir na frente.\n");
        return 0;
    }

    fila->frente              = (fila->frente - 1 + CAPACIDADE_FILA) % CAPACIDADE_FILA;
    fila->pecas[fila->frente] = peca;
    fila->tamanho++;
    return 1;
}

/*
 * dequeue
 *   Remove e devolve a peça da frente da fila.
 *   Retorna 1 em sucesso, 0 se a fila estiver vazia.
 */
int dequeue(FilaCircular *fila, Peca *out) {
    if (filaVazia(fila)) {
        printf("  [AVISO] Fila vazia! Nao ha pecas para remover.\n");
        return 0;
    }

    *out          = fila->pecas[fila->frente];
    fila->frente  = (fila->frente + 1) % CAPACIDADE_FILA;
    fila->tamanho--;
    return 1;
}

/*
 * inverterFila
 *   Inverte a ordem das peças na fila circular.
 *   Estratégia:
 *     1. Copia peças em ordem linear para array auxiliar
 *     2. Reescreve a fila com os elementos em ordem inversa,
 *        normalizando os índices (frente = 0)
 */
void inverterFila(FilaCircular *fila) {
    if (fila->tamanho <= 1) {
        printf("  Fila com %d peca(s) — nada a inverter.\n", fila->tamanho);
        return;
    }

    int qtd = fila->tamanho; /* captura antes de qualquer modificação */

    /* copia peças em ordem (frente → final) */
    Peca aux[CAPACIDADE_FILA];
    for (int i = 0; i < qtd; i++) {
        aux[i] = fila->pecas[(fila->frente + i) % CAPACIDADE_FILA];
    }

    /* reescreve a fila em ordem invertida com índices normalizados */
    fila->frente  = 0;
    fila->tras    = 0;
    fila->tamanho = 0;

    for (int i = qtd - 1; i >= 0; i--) {
        fila->pecas[fila->tras] = aux[i];
        fila->tras              = (fila->tras + 1) % CAPACIDADE_FILA;
        fila->tamanho++;
    }

    printf("  Fila invertida com sucesso (%d pecas).\n", qtd);
}

/*
 * exibirFila
 *   Exibe todas as peças da fila no formato: [tipo id]
 *   Ordem: frente → final.
 */
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
 *  PILHA LINEAR DE RESERVA
 * ============================================================ */

/* inicializarPilha: topo = -1 indica pilha vazia */
void inicializarPilha(Pilha *pilha) {
    pilha->topo = -1;
}

/* pilhaCheia: retorna 1 se o array de reserva estiver lotado */
int pilhaCheia(const Pilha *pilha) {
    return pilha->topo == CAPACIDADE_PILHA - 1;
}

/* pilhaVazia: retorna 1 se não há peças reservadas */
int pilhaVazia(const Pilha *pilha) {
    return pilha->topo == -1;
}

/*
 * push
 *   Remove a peça da frente da fila e empilha na pilha de reserva.
 *   Após a reserva, insere nova peça automática para manter a fila cheia.
 *   Retorna 1 em sucesso, 0 em falha.
 */
int push(Pilha *pilha, FilaCircular *fila) {
    if (pilhaCheia(pilha)) {
        printf("  [AVISO] Pilha cheia! Maximo de %d pecas reservadas.\n",
               CAPACIDADE_PILHA);
        return 0;
    }

    Peca p;
    if (!dequeue(fila, &p)) return 0;

    pilha->topo++;
    pilha->pecas[pilha->topo] = p;
    printf("  Peca [%c %d] reservada na pilha.\n", p.nome, p.id);

    enqueue(fila); /* reposição automática para manter a fila cheia */
    return 1;
}

/*
 * pop
 *   Retira a peça do topo da pilha e a insere na FRENTE da fila.
 *   Para isso, a fila precisa ter ao menos um slot livre.
 *   Retorna 1 em sucesso, 0 em falha.
 */
int pop(Pilha *pilha, FilaCircular *fila) {
    if (pilhaVazia(pilha)) {
        printf("  [AVISO] Pilha vazia! Nao ha pecas reservadas.\n");
        return 0;
    }
    if (filaCheia(fila)) {
        printf("  [AVISO] Fila cheia! Remova uma peca antes de usar a reserva.\n");
        return 0;
    }

    Peca p = pilha->pecas[pilha->topo];
    pilha->topo--;

    enqueueFrente(fila, p);
    printf("  Peca reservada [%c %d] retornada para a frente da fila.\n",
           p.nome, p.id);
    return 1;
}

/*
 * exibirPilha
 *   Exibe as peças reservadas do topo para a base.
 */
void exibirPilha(const Pilha *pilha) {
    printf("  Pilha reserva (%d/%d): ", pilha->topo + 1, CAPACIDADE_PILHA);

    if (pilhaVazia(pilha)) {
        printf("(vazia)\n");
        return;
    }

    for (int i = pilha->topo; i >= 0; i--) {
        printf("[%c %d] ", pilha->pecas[i].nome, pilha->pecas[i].id);
    }
    printf("(base)\n");
}

/* ============================================================
 *  OPERAÇÃO AVANÇADA: TROCA
 * ============================================================ */

/*
 * trocarTopoComFrente
 *   Troca diretamente a peça no topo da pilha com a peça na
 *   frente da fila, sem remover nenhuma das estruturas.
 *   Retorna 1 em sucesso, 0 em falha.
 */
int trocarTopoComFrente(FilaCircular *fila, Pilha *pilha) {
    if (pilhaVazia(pilha)) {
        printf("  [AVISO] Pilha vazia! Nao ha peca no topo para trocar.\n");
        return 0;
    }
    if (filaVazia(fila)) {
        printf("  [AVISO] Fila vazia! Nao ha peca na frente para trocar.\n");
        return 0;
    }

    Peca *topoPtr   = &pilha->pecas[pilha->topo];
    Peca *frentePtr = &fila->pecas[fila->frente];

    /* troca via variável temporária */
    Peca temp  = *topoPtr;
    *topoPtr   = *frentePtr;
    *frentePtr = temp;

    printf("  Troca realizada:\n");
    printf("    Novo topo da pilha : [%c %d]\n", topoPtr->nome,   topoPtr->id);
    printf("    Nova frente da fila: [%c %d]\n", frentePtr->nome, frentePtr->id);
    return 1;
}

/* ============================================================
 *  HISTÓRICO DE DESFAZER (UNDO)
 * ============================================================ */

/* inicializarHistorico: topo = -1 indica histórico vazio */
void inicializarHistorico(Historico *hist) {
    hist->topo = -1;
}

int historicoVazio(const Historico *hist) {
    return hist->topo == -1;
}

int historicoCheia(const Historico *hist) {
    return hist->topo == CAPACIDADE_HISTORICO - 1;
}

/*
 * salvarSnapshot
 *   Captura o estado completo da fila e da pilha de reserva
 *   antes de executar uma operação. Se o histórico estiver cheio,
 *   descarta o snapshot mais antigo (FIFO interno).
 */
void salvarSnapshot(Historico *hist,
                    const FilaCircular *fila,
                    const Pilha *pilha,
                    TipoAcao acao) {
    if (historicoCheia(hist)) {
        /* desloca todos os snapshots uma posição para liberar espaço */
        for (int i = 0; i < hist->topo; i++) {
            hist->entradas[i] = hist->entradas[i + 1];
        }
        hist->topo--;
    }

    hist->topo++;
    Snapshot *s = &hist->entradas[hist->topo];

    /* copia estado da fila */
    memcpy(s->filaSnapshot, fila->pecas, sizeof(fila->pecas));
    s->filaFrente  = fila->frente;
    s->filaTras    = fila->tras;
    s->filaTamanho = fila->tamanho;

    /* copia estado da pilha de reserva */
    memcpy(s->pilhaSnapshot, pilha->pecas, sizeof(pilha->pecas));
    s->pilhaTopo = pilha->topo;

    s->acao = acao;
}

/*
 * desfazer
 *   Restaura o estado salvo no snapshot mais recente do histórico,
 *   revertendo a última operação executada.
 *   Retorna 1 em sucesso, 0 se não há nada a desfazer.
 */
int desfazer(Historico *hist, FilaCircular *fila, Pilha *pilha) {
    if (historicoVazio(hist)) {
        printf("  [AVISO] Nenhuma acao para desfazer.\n");
        return 0;
    }

    Snapshot *s = &hist->entradas[hist->topo];

    /* restaura fila */
    memcpy(fila->pecas, s->filaSnapshot, sizeof(fila->pecas));
    fila->frente  = s->filaFrente;
    fila->tras    = s->filaTras;
    fila->tamanho = s->filaTamanho;

    /* restaura pilha de reserva */
    memcpy(pilha->pecas, s->pilhaSnapshot, sizeof(pilha->pecas));
    pilha->topo = s->pilhaTopo;

    printf("  Acao \"%s\" desfeita com sucesso.\n", nomeAcao[s->acao]);

    hist->topo--; /* remove snapshot consumido */
    return 1;
}

/* ============================================================
 *  INTERFACE COM O JOGADOR
 * ============================================================ */

/*
 * exibirEstado
 *   Mostra o estado completo do jogo: fila, pilha e contagem
 *   de ações disponíveis no histórico de desfazer.
 */
void exibirEstado(const FilaCircular *fila, const Pilha *pilha,
                  const Historico *hist) {
    printf("\n  ============================================\n");
    exibirFila(fila);
    exibirPilha(pilha);
    printf("  Historico (undo): %d acao(oes) disponivel(is)\n",
           hist->topo + 1);
    printf("  ============================================\n");
}

/*
 * exibirMenu
 *   Exibe o menu de opções do Nível Mestre.
 */
void exibirMenu(void) {
    printf("\n  +------------------------------------------+\n");
    printf("  |           TETRIS STACK                   |\n");
    printf("  |           Nivel: Mestre                  |\n");
    printf("  +------------------------------------------+\n");
    printf("  | 1 - Jogar peca (dequeue)                 |\n");
    printf("  | 2 - Reservar peca (push)                 |\n");
    printf("  | 3 - Usar peca reservada (pop)            |\n");
    printf("  | 4 - Trocar topo pilha <-> frente fila    |\n");
    printf("  | 5 - Desfazer ultima acao (undo)          |\n");
    printf("  | 6 - Inverter fila                        |\n");
    printf("  | 0 - Sair                                 |\n");
    printf("  +------------------------------------------+\n");
    printf("  Escolha: ");
}

/* ============================================================
 *  PONTO DE ENTRADA
 * ============================================================ */
int main(void) {
    srand((unsigned int)time(NULL));

    FilaCircular fila;
    Pilha        pilha;
    Historico    historico;

    inicializarFila(&fila);
    inicializarPilha(&pilha);
    inicializarHistorico(&historico);

    printf("\n  Bem-vindo ao TETRIS STACK - Nivel Mestre!\n");
    printf("  Fila inicializada com %d pecas. Pilha e historico vazios.\n",
           CAPACIDADE_FILA);
    exibirEstado(&fila, &pilha, &historico);

    int  opcao;
    Peca pecaJogada;
    int  acaoBemSucedida; /* controla se o snapshot deve ser mantido */

    do {
        exibirMenu();
        if (scanf("%d", &opcao) != 1) {
            while (getchar() != '\n');
            opcao = -1;
        }
        printf("\n");

        switch (opcao) {

            /* ---- 1: Jogar peça ---- */
            case 1:
                salvarSnapshot(&historico, &fila, &pilha, ACAO_JOGAR);
                if (dequeue(&fila, &pecaJogada)) {
                    printf("  Peca [%c %d] jogada!\n",
                           pecaJogada.nome, pecaJogada.id);
                    enqueue(&fila); /* reposição automática */
                } else {
                    historico.topo--; /* desfaz snapshot se a ação falhou */
                }
                break;

            /* ---- 2: Reservar peça ---- */
            case 2:
                salvarSnapshot(&historico, &fila, &pilha, ACAO_RESERVAR);
                acaoBemSucedida = push(&pilha, &fila);
                if (!acaoBemSucedida) historico.topo--;
                break;

            /* ---- 3: Usar peça reservada ---- */
            case 3:
                salvarSnapshot(&historico, &fila, &pilha, ACAO_USAR);
                acaoBemSucedida = pop(&pilha, &fila);
                if (!acaoBemSucedida) historico.topo--;
                break;

            /* ---- 4: Trocar topo da pilha com frente da fila ---- */
            case 4:
                salvarSnapshot(&historico, &fila, &pilha, ACAO_TROCAR);
                acaoBemSucedida = trocarTopoComFrente(&fila, &pilha);
                if (!acaoBemSucedida) historico.topo--;
                break;

            /* ---- 5: Desfazer última ação ---- */
            case 5:
                desfazer(&historico, &fila, &pilha);
                break;

            /* ---- 6: Inverter fila ---- */
            case 6:
                salvarSnapshot(&historico, &fila, &pilha, ACAO_INVERTER);
                inverterFila(&fila);
                break;

            /* ---- 0: Sair ---- */
            case 0:
                printf("  Encerrando o jogo. Ate logo!\n");
                break;

            default:
                printf("  [ERRO] Opcao invalida. Tente novamente.\n");
                break;
        }

        if (opcao != 0) {
            exibirEstado(&fila, &pilha, &historico);
        }

    } while (opcao != 0);

    return 0;
}
