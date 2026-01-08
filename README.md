# ⚡ Discrete Event Simulation Engine (The Boys Theme)

> Um motor de simulação de eventos discretos (DES) desenvolvido em C, focado no gerenciamento manual de memória e orquestração de estruturas de dados genéricas.

## 📖 Sobre o Projeto
Este projeto implementa um sistema de **Simulação de Eventos Discretos**. Diferente de jogos baseados em *frames* contínuos, este motor avança o tempo baseado em uma "Lista de Eventos Futuros" (LEF), técnica amplamente utilizada em sistemas operacionais e logística.

O sistema simula um ecossistema de entidades ("Heróis" e "Bases") que interagem de forma estocástica (aleatória), exigindo controle rigoroso de alocação de memória e manipulação de ponteiros.

## 🛠 Tecnologias & Conceitos Aplicados
* **Linguagem C (C99):** Foco em manipulação de baixo nível e ponteiros.
* **Estruturas de Dados:**
    * **Priority Queue (Fila de Prioridade):** Implementada via lista encadeada ordenada para gerenciar a *Lista de Eventos Futuros (LEF)*, garantindo que os eventos sejam processados na ordem cronológica correta.
    * **Generic Queues (FIFO):** Uso de ponteiros `void*` para criar filas genéricas que podem armazenar qualquer tipo de dado, demonstrando abstração e reutilização de código.
    * **Sets (Conjuntos):** Manipulação de vetores para operações rápidas de união, interseção e pertinência de habilidades.
* **Gerenciamento de Memória:** Estratégia manual de alocação/desalocação (`malloc`/`free`) implementada em todas as estruturas para evitar *memory leaks*.
* **Modularização:** Código desacoplado utilizando arquivos de cabeçalho (`.h`) para definição de interfaces (TADs).

## 🚀 Como Executar
O projeto utiliza um `makefile` para compilação automatizada.

### Pré-requisitos
* GCC Compiler
* Make

### Compilando e Rodando
```bash
# Compilar o projeto e gerar o executável
make

# Executar a simulação
./theboys

# Limpar arquivos temporários e objetos
make clean
