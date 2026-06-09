# Lottery Scheduling com `settickets(int)` no xv6

**Disciplina:** GBC045 — Sistemas Operacionais
**Curso:** Bacharelado em Ciência da Computação — FACOM/UFU
**Tema 5:** chamada de sistema `settickets(int)` e algoritmo de escalonamento *Lottery Scheduling* (compartilhamento proporcional)

## Vídeo da apresentação
📹 Disponível em: https://www.youtube.com/watch?v=jG1Lr0_pmUI 

---

## Integrantes

- Daniel - 12411BSI201
- Eduarda Paulino - 12421BCC077
- Gustavo Martins - 12421BCC103
- João Arthur - 12421BCC069
- Pedro Luyd - 12421BCC041

---

## Sobre o projeto

Este projeto estuda o **Lottery Scheduling**, um mecanismo de alocação de recursos por
sorteio aleatório, conforme proposto por Waldspurger & Weihl em 1994. Diferente
do *Round Robin*, escalonador padrão do xv6, que divide o tempo de CPU igualmente de
forma circular, o Lottery Scheduling implementa **compartilhamento proporcional**, onde cada
processo recebe CPU na proporção de sua importância.

Como descrito no artigo original, os direitos sobre o recurso são representados por
**bilhetes** (*tickets*). A cada quantum, o escalonador realiza um sorteio e o processo dono do
bilhete sorteado executa. Assim, um processo que detém uma fração maior dos bilhetes vence o
sorteio com mais frequência. Segundo a formalização apresentada no artigo original, a probabilidade de um
processo com `t` bilhetes vencer um sorteio entre `T` bilhetes totais é `p = t/T`, e a precisão
da proporção observada melhora conforme o número de sorteios cresce.

O mecanismo entre o processo e o escalonador é a chamada de sistema **`settickets(int)`**: é por meio
dela que um processo informa ao kernel quantos bilhetes deseja. Como o próprio artigo argumenta
sem um mecanismo desse tipo todos os processos manteriam a mesma quantidade de bilhetes, e
o Lottery Scheduling se comportaria, na prática, como um Round Robin, perdendo justamente o
controle proporcional que o caracteriza.

---

## Etapas do projeto

O trabalho é dividido em duas etapas complementares.

### Etapa 1 — Fundamentação *(esta entrega)*

Entrega de caráter conceitual, contendo:

- **Vídeo (5–7 min):** apresentação do tema, motivação, funcionamento do mecanismo de bilhetes,
  papel da `settickets(int)`, proposta de implementação no xv6 e aplicações práticas, com a
  participação de todos os integrantes.
- **Slides:** material de apoio visual com os detalhes do projeto (pasta `slides/`).
- **README:** este arquivo.

A proposta de implementação no xv6 apresentada nesta etapa prevê três alterações principais,
além do registro da nova syscall:

1. **Estrutura do processo** : incluir um campo de bilhetes, para que o escalonador conheça a
   quantidade de bilhetes de cada processo.
2. **Escalonador (`proc.c`)** : substituir a seleção circular pela lógica de sorteio: somar os
   bilhetes dos processos prontos, sortear um número dentro desse total e escolher o processo no
   qual a soma acumulada alcança o valor sorteado.
3. **`fork()`** : fazer o processo filho herdar o número de bilhetes do pai, preservando a
   proporcionalidade configurada.

### Etapa 2 — Implementação prática *(a entregar)*

Implementação da solução no xv6, incluindo a criação/registro da syscall, as alterações no
kernel, um programa de usuário para teste e a **análise comparativa experimental com o Round
Robin** original do xv6 (métricas, tabelas e gráficos). Arquivos previstos para modificação:
`proc.c` (escalonador), `syscall.c` (syscall) e `user.h` (interface).

---

## Estrutura do repositório

```
grupoX/
├── README.md     # este arquivo
├── slides/       # slides da apresentação (Etapa 1)
├── video/        # vídeo da apresentação (Etapa 1)
├── code/         # código modificado do xv6 (Etapa 2)
├── tests/        # programa(s) de teste (Etapa 2)
└── report/       # relatório técnico (Etapa 2)
```

---

## Referências

WALDSPURGER, C. A.; WEIHL, W. E. **Lottery Scheduling: Flexible Proportional-Share Resource
Management**. In: *Proceedings of the First Symposium on Operating Systems Design and
Implementation (OSDI '94)*, p. 1–11. Monterey, Califórnia: USENIX Association, nov. 1994.
Disponível em: <https://www.usenix.org/legacy/publications/library/proceedings/osdi/full_papers/waldspurger.pdf>.

**xv6: a simple, Unix-like teaching operating system** (revisão RISC-V). MIT PDOS (Parallel
and Distributed Operating Systems Group). Disponível em:
<https://pdos.csail.mit.edu/6.1810/2024/xv6/book-riscv-rev4.pdf>.

MIT PDOS. **xv6 — documentação (curso 6.1810, 2024)**. Disponível em:
<https://pdos.csail.mit.edu/6.1810/2024/xv6.html>.
