#include <stdio.h>
#include <stdlib.h>
#include "fprio.h"

// Cria uma fila vazia.
struct fprio_t *fprio_cria() {
    struct fprio_t *f = malloc(sizeof(struct fprio_t));
    if (!f) return NULL;
    f->prim = NULL;
    f->fim = NULL;
    f->num = 0;
    return f;
}

// Libera todas as estruturas de dados da fila.
struct fprio_t *fprio_destroi(struct fprio_t *f) {
    if (!f) return NULL;
    struct fpnodo_t *p = f->prim;
    while (p) {
        struct fpnodo_t *t = p->prox;
        free(p->item);
        free(p);
        p = t;
    }
    free(f);
    return NULL;
}

// Insere o item na fila, mantendo ordenada de forma crescente.
int fprio_insere(struct fprio_t *f, void *item, int tipo, int prio) {
    if (!f) return -1;
    struct fpnodo_t *novo = malloc(sizeof(struct fpnodo_t));
    if (!novo) return -1;
    novo->item = item;
    novo->tipo = tipo;
    novo->prio = prio;
    novo->prox = NULL;

    // a fila está vazia ou o novo item é o de menor prioridade
    if (!f->prim || prio < f->prim->prio) {
        novo->prox = f->prim;
        f->prim = novo;
    } else {
        // percorre a fila para achar a posição correta
        struct fpnodo_t *p = f->prim;
        // A condição garante a FIFO para prioridades iguais
        while (p->prox && p->prox->prio <= prio) {
            p = p->prox;
        }
        novo->prox = p->prox;
        p->prox = novo;
    }

    if (!novo->prox) {
        f->fim = novo;
    }

    f->num++;
    return f->num;
}

// Retira o primeiro item da fila e o devolve.
void *fprio_retira(struct fprio_t *f, int *tipo, int *prio) {
    if (!f || f->num == 0) return NULL;
    struct fpnodo_t *p = f->prim;
    void *item = p->item;
    if (tipo) *tipo = p->tipo;
    if (prio) *prio = p->prio;
    f->prim = p->prox;
    if (!f->prim)
        f->fim = NULL;
    free(p);
    f->num--;
    return item;
}

// Informa o número de itens na fila.
int fprio_tamanho(struct fprio_t *f) {
    if (!f) return -1;
    return f->num;
}

// Imprime o conteúdo da fila
void fprio_imprime(struct fprio_t *f) {
    if (!f) return;
    struct fpnodo_t *p = f->prim;
    int primeiro = 1;
    while(p) {
        if (!primeiro) printf(" ");
        printf("(%d %d)", p->tipo, p->prio);
        p = p->prox;
        primeiro = 0;
    }
}
