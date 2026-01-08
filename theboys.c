// programa principal do projeto "The Boys - 2024/2"
// Autor: Gabriel Lucas Barbosa

// Includes

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "conjunto.h"
#include "fila.h"
#include "fprio.h"

// Defines e Constantes

#define T_FIM_DO_MUNDO 525600
#define N_TAMANHO_MUNDO 20000
#define N_HABILIDADES 10
#define N_HEROIS (N_HABILIDADES * 5)
#define N_BASES (N_HEROIS / 5)
#define N_MISSOES (T_FIM_DO_MUNDO / 100)
#define N_COMPOSTOS_V (N_HABILIDADES * 3)

typedef enum {
    EV_CHEGA, EV_ESPERA, EV_DESISTE, EV_AVISA, EV_ENTRA, EV_SAI,
    EV_VIAJA, EV_MORRE, EV_MISSAO, EV_FIM
} evento_tipo_t;

//Structs

typedef struct { int x, y; } coord_t;
typedef struct { int id; struct cjto_t *habilidades; int paciencia; int velocidade; int experiencia; int base_id; int vivo; } heroi_t;

typedef struct {
    int id;
    int lotacao;
    coord_t local;
    struct cjto_t *presentes;
    struct fila_t *espera;
    int missoes_cumpridas;
    int fila_max;
    struct cjto_t *habilidades_equipe;
} base_t;

typedef struct { int id; coord_t local; struct cjto_t *habilidades; int tentativas; int cumprida; } missao_t;
typedef struct { evento_tipo_t tipo; int heroi_id; int base_id; int missao_id; } evento_t;
typedef struct { int relogio; heroi_t *herois; base_t *bases; missao_t *missoes; struct fprio_t *lef; int compostos_v; long eventos_tratados; } mundo_t;
typedef struct { int id; long long dist_sq; } base_dist_t;

// Protótipos

mundo_t* mundo_cria();
void agenda_eventos_iniciais(mundo_t *mundo);
void mundo_destroi(mundo_t *mundo);
void evento_chega(mundo_t *m, evento_t *ev);
void evento_espera(mundo_t *m, evento_t *ev);
void evento_desiste(mundo_t *m, evento_t *ev);
void evento_avisa(mundo_t *m, evento_t *ev);
void evento_entra(mundo_t *m, evento_t *ev);
void evento_sai(mundo_t *m, evento_t *ev);
void evento_viaja(mundo_t *m, evento_t *ev);
void evento_morre(mundo_t *m, evento_t *ev);
void evento_missao(mundo_t *m, evento_t *ev);
void evento_fim(mundo_t *m);
void agenda_evento(mundo_t *mundo, int tempo, evento_tipo_t tipo, int h_id, int b_id, int m_id);
long long dist_sq(coord_t p1, coord_t p2);
int compara_bases_dist(const void *a, const void *b);
void imprime_habilidades(struct cjto_t *c);
void imprime_fila(struct fila_t *f);
int busca_base_apta(mundo_t* m, missao_t* missao, base_dist_t* dists);
int usa_composto_v(mundo_t* m, missao_t* missao, base_dist_t* dists);

// Programa Principal

int main() {
    srand(time(NULL));
    // iniciar o mundo
    mundo_t *mundo = mundo_cria();
    agenda_eventos_iniciais(mundo);

    // executar o laço de simulação
    while (1) {
        int tipo, tempo;
        evento_t *ev = fprio_retira(mundo->lef, &tipo, &tempo);
        if (!ev) break;
        mundo->relogio = tempo;
        mundo->eventos_tratados++;
        if (ev->heroi_id != -1 && !mundo->herois[ev->heroi_id].vivo) {
            free(ev);
            continue;
        }
        switch (ev->tipo) {
            case EV_CHEGA:   evento_chega(mundo, ev);   break;
            case EV_ESPERA:  evento_espera(mundo, ev);  break;
            case EV_DESISTE: evento_desiste(mundo, ev); break;
            case EV_AVISA:   evento_avisa(mundo, ev);   break;
            case EV_ENTRA:   evento_entra(mundo, ev);   break;
            case EV_SAI:     evento_sai(mundo, ev);     break;
            case EV_VIAJA:   evento_viaja(mundo, ev);   break;
            case EV_MORRE:   evento_morre(mundo, ev);   break;
            case EV_MISSAO:  evento_missao(mundo, ev);  break;
            case EV_FIM:     evento_fim(mundo); free(ev); goto fim_simulacao;
        }
        free(ev);
    }
fim_simulacao:
    // destroi o mundo
    mundo_destroi(mundo);
    return 0;
}

// Implementação das funções

mundo_t* mundo_cria() {
    mundo_t *mundo = malloc(sizeof(mundo_t));
    mundo->relogio = 0;
    mundo->compostos_v = N_COMPOSTOS_V;
    mundo->eventos_tratados = 0;
    mundo->lef = fprio_cria();

    // Iniciializa os heróis um por um
    mundo->herois = malloc(sizeof(heroi_t) * N_HEROIS);
    for (int i = 0; i < N_HEROIS; i++) {
        mundo->herois[i].id = i;
        mundo->herois[i].experiencia = 0;
        mundo->herois[i].paciencia = rand() % 101;
        mundo->herois[i].velocidade = 50 + (rand() % 4951);
        mundo->herois[i].habilidades = cjto_aleat(1 + (rand() % 3), N_HABILIDADES);
        mundo->herois[i].base_id = -1;
        mundo->herois[i].vivo = 1;
    }

    // Inicializa as bases
    mundo->bases = malloc(sizeof(base_t) * N_BASES);
    for (int i = 0; i < N_BASES; i++) {
        mundo->bases[i] = (base_t){ .id = i, .local.x = rand() % N_TAMANHO_MUNDO, .local.y = rand() % N_TAMANHO_MUNDO,
                                    .lotacao = 3 + (rand() % 8), .presentes = cjto_cria(N_HEROIS), .espera = fila_cria(),
                                    .missoes_cumpridas = 0, .fila_max = 0, .habilidades_equipe = cjto_cria(N_HABILIDADES) };
    }

    //Missões
    mundo->missoes = malloc(sizeof(missao_t) * N_MISSOES);
    for (int i = 0; i < N_MISSOES; i++) {
        mundo->missoes[i] = (missao_t){ .id = i, .local.x = rand() % N_TAMANHO_MUNDO, .local.y = rand() % N_TAMANHO_MUNDO,
                                        .habilidades = cjto_aleat(6 + (rand() % 5), N_HABILIDADES), .tentativas = 0, .cumprida = 0 };
    }

    return mundo;
}

void agenda_eventos_iniciais(mundo_t *mundo) {
    for (int i = 0; i < N_HEROIS; i++) {
        agenda_evento(mundo, rand() % 4321, EV_CHEGA, i, rand() % N_BASES, -1);
    }
    for (int i = 0; i < N_MISSOES; i++) {
        agenda_evento(mundo, rand() % (T_FIM_DO_MUNDO + 1), EV_MISSAO, -1, -1, i);
    }
    agenda_evento(mundo, T_FIM_DO_MUNDO, EV_FIM, -1, -1, -1);
}

void mundo_destroi(mundo_t *mundo) {
    if (!mundo) return;
    for (int i = 0; i < N_HEROIS; i++) cjto_destroi(mundo->herois[i].habilidades);
    free(mundo->herois);
    for (int i = 0; i < N_BASES; i++) {
        cjto_destroi(mundo->bases[i].presentes);
        cjto_destroi(mundo->bases[i].habilidades_equipe); 
        while (fila_tamanho(mundo->bases[i].espera) > 0) free(fila_retira(mundo->bases[i].espera));
        fila_destroi(mundo->bases[i].espera);
    }
    free(mundo->bases);
    for (int i = 0; i < N_MISSOES; i++) cjto_destroi(mundo->missoes[i].habilidades);
    free(mundo->missoes);
    while (fprio_tamanho(mundo->lef) > 0) {
        int tipo, tempo;
        free(fprio_retira(mundo->lef, &tipo, &tempo));
    }
    fprio_destroi(mundo->lef);
    free(mundo);
}

void evento_chega(mundo_t *m, evento_t *ev) {
    heroi_t *heroi = &m->herois[ev->heroi_id];
    base_t *base = &m->bases[ev->base_id];
    heroi->base_id = base->id;
    int tam_fila = fila_tamanho(base->espera);
    int presentes = cjto_card(base->presentes);
    int espera = (heroi->paciencia > (10 * tam_fila));
    if (presentes < base->lotacao && tam_fila == 0) 
	espera = 1;

    printf("%6d: CHEGA HEROI %2d BASE %d (%2d/%2d) %s\n", m->relogio, heroi->id, base->id, presentes, base->lotacao, espera ? "ESPERA" : "DESISTE");
    agenda_evento(m, m->relogio, espera ? EV_ESPERA : EV_DESISTE, heroi->id, base->id, -1);
}

void evento_espera(mundo_t *m, evento_t *ev) {
    base_t *base = &m->bases[ev->base_id];
    printf("%6d: ESPERA HEROI %2d BASE %d (%2d)\n", m->relogio, ev->heroi_id, base->id, fila_tamanho(base->espera));
    int *id_ptr = malloc(sizeof(int));
    *id_ptr = ev->heroi_id;
    fila_insere(base->espera, id_ptr);
    if (fila_tamanho(base->espera) > base->fila_max) base->fila_max = fila_tamanho(base->espera);
    	agenda_evento(m, m->relogio, EV_AVISA, -1, base->id, -1);
}

void evento_desiste(mundo_t *m, evento_t *ev) {
    printf("%6d: DESIST HEROI %2d BASE %d\n", m->relogio, ev->heroi_id, ev->base_id);
    agenda_evento(m, m->relogio, EV_VIAJA, ev->heroi_id, rand() % N_BASES, -1);
}

void evento_avisa(mundo_t *m, evento_t *ev) {
    base_t *base = &m->bases[ev->base_id];

    printf("%6d: AVISA PORTEIRO BASE %d (%2d/%2d) FILA ", m->relogio, base->id, cjto_card(base->presentes), base->lotacao);
    imprime_fila(base->espera);
    printf("\n");

    while (cjto_card(base->presentes) < base->lotacao && fila_tamanho(base->espera) > 0) {
        int *id_ptr = fila_retira(base->espera);
        int heroi_id = *id_ptr;
        free(id_ptr);
    
	if (m->herois[heroi_id].vivo) {
            printf("%6d: AVISA PORTEIRO BASE %d ADMITE %2d\n", m->relogio, base->id, heroi_id);
            cjto_insere(base->presentes, heroi_id);
            agenda_evento(m, m->relogio, EV_ENTRA, heroi_id, base->id, -1);
        }
    }
}

void evento_entra(mundo_t *m, evento_t *ev) {
    heroi_t *heroi = &m->herois[ev->heroi_id];
    base_t *base = &m->bases[ev->base_id];
    int tempo_saida = m->relogio + 15 + heroi->paciencia + (1 + rand() % 20);
    printf("%6d: ENTRA HEROI %2d BASE %d (%2d/%2d) SAI %d\n", m->relogio, heroi->id, base->id, cjto_card(base->presentes), base->lotacao, tempo_saida);
    

    // Adiciona habilidades do novo herói (Atualiza o cache)
    struct cjto_t* uniao_temp = cjto_uniao(base->habilidades_equipe, heroi->habilidades);
    cjto_destroi(base->habilidades_equipe);
    base->habilidades_equipe = uniao_temp;

    agenda_evento(m, tempo_saida, EV_SAI, heroi->id, base->id, -1);
}

void evento_sai(mundo_t *m, evento_t *ev) {
    base_t *base = &m->bases[ev->base_id];
    cjto_retira(base->presentes, ev->heroi_id);
    
    // Recalcula do zero com os heróis restantes (meio ruim para bases muito cheias mas acho que é o mais eficiente)
    cjto_destroi(base->habilidades_equipe);
    base->habilidades_equipe = cjto_cria(N_HABILIDADES);
    for (int i = 0; i < N_HEROIS; i++) {
        if (cjto_pertence(base->presentes, i)) {
            struct cjto_t* uniao_temp = cjto_uniao(base->habilidades_equipe, m->herois[i].habilidades);
            cjto_destroi(base->habilidades_equipe);
            base->habilidades_equipe = uniao_temp;
        }
    }

    printf("%6d: SAI   HEROI %2d BASE %d (%2d/%2d)\n", m->relogio, ev->heroi_id, ev->base_id, cjto_card(base->presentes), base->lotacao);
    agenda_evento(m, m->relogio, EV_VIAJA, ev->heroi_id, rand() % N_BASES, -1);
    agenda_evento(m, m->relogio, EV_AVISA, -1, ev->base_id, -1);
}

void evento_viaja(mundo_t *m, evento_t *ev) {
    heroi_t *heroi = &m->herois[ev->heroi_id];
    base_t *origem = &m->bases[heroi->base_id];
    base_t *destino = &m->bases[ev->base_id];
    int dist = (int)sqrt(dist_sq(origem->local, destino->local));
    int duracao = (heroi->velocidade > 0) ? (dist / heroi->velocidade) : dist;
    int chegada = m->relogio + duracao;
    printf("%6d: VIAJA HEROI %2d BASE %d BASE %d DIST %d VEL %d CHEGA %d\n", m->relogio, heroi->id, origem->id, destino->id, dist, heroi->velocidade, chegada);
    heroi->base_id = destino->id;
    agenda_evento(m, chegada, EV_CHEGA, heroi->id, destino->id, -1);
}

void evento_morre(mundo_t *m, evento_t *ev) {
    heroi_t *heroi = &m->herois[ev->heroi_id];
    if (!heroi->vivo) return;
    heroi->vivo = 0;
    // A lógica é a mesma de um evento SAI então dupliquei ele aqui
    /* Talvez é melhor mudar p uma função auxiliar*/
    base_t* base = &m->bases[heroi->base_id];
    cjto_retira(base->presentes, heroi->id);
    cjto_destroi(base->habilidades_equipe);
    base->habilidades_equipe = cjto_cria(N_HABILIDADES);
    for (int i = 0; i < N_HEROIS; i++) {
        if (cjto_pertence(base->presentes, i)) {
            struct cjto_t* uniao_temp = cjto_uniao(base->habilidades_equipe, m->herois[i].habilidades);
            cjto_destroi(base->habilidades_equipe);
            base->habilidades_equipe = uniao_temp;
        }
    }
    printf("%6d: MORRE HEROI %2d MISSAO %d\n", m->relogio, heroi->id, ev->missao_id);
    agenda_evento(m, m->relogio, EV_AVISA, -1, heroi->base_id, -1);
}

int busca_base_apta(mundo_t* m, missao_t* missao, base_dist_t* dists) {
    for (int i = 0; i < N_BASES; i++) {
        base_t *base = &m->bases[dists[i].id];
        if (cjto_card(base->presentes) == 0) continue;

        if (cjto_contem(base->habilidades_equipe, missao->habilidades)) {
            printf("%6d: MISSAO %d CUMPRIDA BASE %d HABS: [", m->relogio, missao->id, base->id);
            imprime_habilidades(base->habilidades_equipe);
            printf("]\n");
            for (int h_id = 0; h_id < N_HEROIS; h_id++)
                if (cjto_pertence(base->presentes, h_id)) m->herois[h_id].experiencia++;
            base->missoes_cumpridas++;
            return 1;
        }
    }
    return 0;
}

int usa_composto_v(mundo_t* m, missao_t* missao, base_dist_t* dists) {
    if (m->compostos_v <= 0 || (m->relogio % 2500 != 0)) return 0;
    base_t *base_proxima = &m->bases[dists[0].id];
    int heroi_a_sacrificar = -1, max_exp = -1;
    for (int h_id = 0; h_id < N_HEROIS; h_id++) {
        if (cjto_pertence(base_proxima->presentes, h_id) && m->herois[h_id].vivo) {
            if (m->herois[h_id].experiencia > max_exp) {
                max_exp = m->herois[h_id].experiencia;
                heroi_a_sacrificar = h_id;
            }
        }
    }
    if (heroi_a_sacrificar != -1) {
        m->compostos_v--;
        printf("%6d: MISSAO %d CUMPRIDA BASE %d (COMPOSTO V)\n", m->relogio, missao->id, base_proxima->id);
        base_proxima->missoes_cumpridas++;
        for (int h_id = 0; h_id < N_HEROIS; h_id++)
            if (cjto_pertence(base_proxima->presentes, h_id) && h_id != heroi_a_sacrificar)
                m->herois[h_id].experiencia++;
        agenda_evento(m, m->relogio, EV_MORRE, heroi_a_sacrificar, base_proxima->id, missao->id);
        return 1;
    }
    return 0;
}

void evento_missao(mundo_t *m, evento_t *ev) {
    missao_t *missao = &m->missoes[ev->missao_id];
    missao->tentativas++;
    printf("%6d: MISSAO %d TENT %d HAB REQ: [", m->relogio, missao->id, missao->tentativas);
    imprime_habilidades(missao->habilidades);
    printf("]\n");
    base_dist_t dists[N_BASES];
    for (int i = 0; i < N_BASES; i++) {
        dists[i].id = i;
        dists[i].dist_sq = dist_sq(missao->local, m->bases[i].local);
    }
    qsort(dists, N_BASES, sizeof(base_dist_t), compara_bases_dist);
    if (busca_base_apta(m, missao, dists) || usa_composto_v(m, missao, dists)) {
        missao->cumprida = 1;
    } else {
        printf("%6d: MISSAO %d IMPOSSIVEL\n", m->relogio, missao->id);
        agenda_evento(m, m->relogio + 1440, EV_MISSAO, -1, -1, missao->id);
    }
}

void evento_fim(mundo_t *m) {
    printf("%6d: FIM\n", m->relogio);
    int mortos = 0;
    for (int i = 0; i < N_HEROIS; i++) {
        heroi_t *h = &m->herois[i];
        printf("HEROI %2d %-5s PAC %3d VEL %4d EXP %4d HABS [",
               h->id, h->vivo ? "VIVO" : "MORTO", h->paciencia, h->velocidade, h->experiencia);
        imprime_habilidades(h->habilidades);
        printf("]\n");
        if (!h->vivo) mortos++;
    }
    for (int i = 0; i < N_BASES; i++) {
        base_t *b = &m->bases[i];
        printf("BASE %2d LOT %2d FILA MAX %2d MISSOES %d\n",
               b->id, b->lotacao, b->fila_max, b->missoes_cumpridas);
    }
    int cumpridas = 0, min_tent = -1, max_tent = 0;
    double soma_tent = 0;
    for(int i = 0; i < N_MISSOES; i++) {
        if(m->missoes[i].cumprida) {
            cumpridas++;
            soma_tent += m->missoes[i].tentativas;
            if(min_tent == -1 || m->missoes[i].tentativas < min_tent) min_tent = m->missoes[i].tentativas;
            if(max_tent < m->missoes[i].tentativas) max_tent = m->missoes[i].tentativas;
        }
    }
    printf("EVENTOS TRATADOS: %ld\n", m->eventos_tratados);
    printf("MISSOES CUMPRIDAS: %d/%d (%.1f%%)\n", cumpridas, N_MISSOES, N_MISSOES > 0 ? 100.0 * cumpridas / N_MISSOES : 0.0);
    if (cumpridas > 0)
        printf("TENTATIVAS/MISSAO: MIN %d, MAX %d, MEDIA %.1f\n", min_tent, max_tent, soma_tent / cumpridas);
    printf("TAXA MORTALIDADE: %.1f%%\n", N_HEROIS > 0 ? 100.0 * mortos / N_HEROIS : 0.0);
}

void agenda_evento(mundo_t *mundo, int tempo, evento_tipo_t tipo, int h_id, int b_id, int m_id) {
    evento_t *ev = malloc(sizeof(evento_t));
    *ev = (evento_t){ .tipo = tipo, .heroi_id = h_id, .base_id = b_id, .missao_id = m_id };
    fprio_insere(mundo->lef, ev, tipo, tempo);
}

long long dist_sq(coord_t p1, coord_t p2) {
    long long dx = p1.x - p2.x;
    long long dy = p1.y - p2.y;
    return dx * dx + dy * dy;
}

int compara_bases_dist(const void *a, const void *b) {
    base_dist_t *ba = (base_dist_t *)a;
    base_dist_t *bb = (base_dist_t *)b;
    if (ba->dist_sq < bb->dist_sq) return -1;
    if (ba->dist_sq > bb->dist_sq) return 1;
    return 0;
}

void imprime_habilidades(struct cjto_t *c) {
    if (!c) return;
    int primeiro = 1;
    for (int i = 0; i < c->cap; i++) {
        if (cjto_pertence(c, i)) {
            if (!primeiro) printf(" ");
            printf("%d", i);
            primeiro = 0;
        }
    }
}

void imprime_fila(struct fila_t *f) {
    printf("[");
    if (f && f->prim) {
        struct fila_nodo_t *nodo = f->prim;
        while(nodo) {
            printf(" %2d", *(int*)nodo->item);
            nodo = nodo->prox;
        }
    }
    printf(" ]");
}
