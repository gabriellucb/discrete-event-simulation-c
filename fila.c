// TAD Fila Genérica
// Implementação baseada nos Tps antigos

#include <stdio.h>
#include <stdlib.h>
#include "fila.h"

// Cria uma fila vazia.
// Retorna ponteiro para a fila criada ou NULL se erro.
struct fila_t *fila_cria () {
    struct fila_t *f = malloc(sizeof(struct fila_t));
    if (!f) return NULL;
    f->prim = NULL;
    f->fim = NULL;
    f->num = 0;
    return f;
}

// Libera todas as estruturas de dados da fila, inclusive os itens.
struct fila_t *fila_destroi (struct fila_t *f) {
    if (!f) return NULL;
    struct fila_nodo_t *p = f->prim;
    while (p) {
        struct fila_nodo_t *t = p->prox;
        free(p->item);
        free(p);
        p = t;
    }
    free(f);
    return NULL;
}

// Insere o item na fila
// Retorna número de itens na fila após a operação ou -1 se erro.
int fila_insere (struct fila_t *f, void *item) {
    if (!f) return -1;
    struct fila_nodo_t *novo = malloc(sizeof(struct fila_nodo_t));
    if (!novo) return -1;
    novo->item = item;
    novo->prox = NULL;
    if (f->fim)
        f->fim->prox = novo;
    else
        f->prim = novo;
    f->fim = novo;
    f->num++;
    return f->num;
}

// Retira o primeiro item da fila e o devolve
void *fila_retira (struct fila_t *f) {
    if (!f || fila_tamanho(f) == 0) return NULL;
    struct fila_nodo_t *p = f->prim;
    void *item = p->item;
    f->prim = p->prox;
    if (!f->prim)
        f->fim = NULL;
    free(p);
    f->num--;
    return item;
}

// Informa o número de itens na fila.
int fila_tamanho (struct fila_t *f) {
    if (!f) return -1;
    return f->num;
}

// Imprime o conteúdo da fila
void fila_imprime (struct fila_t *f) {
    if (!f) return;
    struct fila_nodo_t *p = f->prim;
    printf("Fila: ");
    while (p) {
        printf("%p ", p->item);
        p = p->prox;
    }
    printf("\n");
}
