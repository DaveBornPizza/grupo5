# Relatório Técnico — Lottery Scheduling no xv6

**Disciplina:** GBC045 — Sistemas Operacionais · FACOM/UFU
**Tema 5:** chamada de sistema `settickets(int)` e algoritmo *Lottery Scheduling*
**Etapa 2:** implementação prática e análise experimental

## Integrantes

- Daniel — 12411BSI201
- Eduarda Paulino — 12421BCC077
- Gustavo Martins — 12421BCC103
- João Arthur — 12421BCC069
- Pedro Luyd — 12421BCC041

---

## 1. Objetivo

Substituir o escalonador *Round Robin* (RR) padrão do xv6-riscv por um
escalonador de **compartilhamento proporcional** baseado em sorteio de bilhetes
(*Lottery Scheduling*, Waldspurger & Weihl, 1994) e disponibilizar a chamada de
sistema `settickets(int)`, que permite a um processo definir quantos bilhetes
possui. Quanto mais bilhetes, maior a probabilidade de o processo vencer o
sorteio a cada quantum e, portanto, maior a fração de CPU recebida.

A probabilidade de um processo com `t` bilhetes vencer um sorteio entre `T`
bilhetes totais é:

$$p = \frac{t}{T}$$

e a proporção observada de CPU converge para esse valor conforme o número de
sorteios cresce.

---

## 2. Implementação

Base: **xv6-riscv** (MIT PDOS, revisão RISC-V). O escalonador e o teste de
proporcionalidade foram implementados pelos colegas; na etapa de **integração**
foram corrigidos os pontos que impediam o código de compilar e a syscall de
funcionar. A tabela distingue as duas contribuições.

| Arquivo | Alteração | Origem |
|---------|-----------|--------|
| [`kernel/proc.c`](../code/kernel/proc.c) | PRNG `randtolottery()`, escalonador por sorteio, herança de bilhetes no `fork`, uso de `tickets`/`DEFAULT_TICKETS`. | colegas |
| [`user/testeloteria.c`](../code/user/testeloteria.c) | Teste de proporcionalidade com barreira por *pipes*. | colegas |
| [`kernel/proc.h`](../code/kernel/proc.h) | Declaração do campo `int tickets` em `struct proc` (faltava). | integração |
| [`kernel/param.h`](../code/kernel/param.h) | `#define DEFAULT_TICKETS 1` (faltava). | integração |
| [`kernel/proc.c`](../code/kernel/proc.c) | Função `settickets()` + remoção de comentário indevido. | integração |
| [`kernel/defs.h`](../code/kernel/defs.h) | Protótipo `int settickets(int)`. | integração |
| [`kernel/syscall.h`](../code/kernel/syscall.h) | `#define SYS_settickets 22`. | integração |
| [`kernel/syscall.c`](../code/kernel/syscall.c) | Registro de `sys_settickets` na tabela de syscalls. | integração |
| [`kernel/sysproc.c`](../code/kernel/sysproc.c) | Handler `sys_settickets`. | integração |
| [`user/user.h`](../code/user/user.h), [`user/usys.pl`](../code/user/usys.pl) | Interface da syscall para userspace. | integração |
| [`user/testerobustez.c`](../code/user/testerobustez.c) + `Makefile` | Teste de robustez dos casos de borda. | integração |

> **Furo encontrado na revisão de qualidade:** o `proc.c` dos colegas já usava
> `p->tickets` e `DEFAULT_TICKETS` e o teste chamava `settickets()`, mas o campo
> nunca foi declarado, a constante nunca foi definida e a syscall nunca foi
> registrada. Como estava, **o kernel não compilava** (`'struct proc' has no
> member named 'tickets'`) e o teste não linkava. A integração fechou esses furos.

### 2.1 Estrutura do processo e valor padrão

Cada `struct proc` passou a ter um campo `tickets`. Em `allocproc`, todo processo
nasce com `DEFAULT_TICKETS = 1`. Assim, nenhum processo fica com zero bilhetes
por omissão (o que o excluiria permanentemente do sorteio).

### 2.2 Herança em `fork`

Em `kfork`, o filho copia o número de bilhetes do pai
(`np->tickets = p->tickets`), preservando a proporção configurada — exatamente
como recomenda o artigo original.

### 2.3 Gerador de números pseudoaleatórios

O kernel do xv6 não possui fonte de aleatoriedade. Os colegas adicionaram um PRNG
*congruente linear* (LCG), o mesmo dos geradores clássicos de `rand()`,
perturbado a cada rodada pelo contador de `ticks` do relógio:

```c
unsigned long next_random = 1;

unsigned int
randtolottery(void) {
    next_random = next_random * 1103515245 + 12345;
    return (unsigned int)(next_random / 65536) % 32768;
}
```

### 2.4 O escalonador por sorteio

O laço do escalonador foi reescrito em **duas passagens**:

1. **Soma dos bilhetes:** percorre a tabela de processos somando os bilhetes de
   todos os que estão `RUNNABLE` (`total_tickets`). Se o total for 0 (ninguém
   pronto), volta ao topo do laço (com interrupções habilitadas).
2. **Sorteio e seleção:** sorteia o bilhete vencedor em `[0, total_tickets-1]` e
   percorre novamente a tabela acumulando bilhetes; o primeiro processo cujo
   acumulado ultrapassa o bilhete vencedor é executado.

```c
int total_tickets = 0;
for(p = proc; p < &proc[NPROC]; p++) {
  acquire(&p->lock);
  if(p->state == RUNNABLE)
    total_tickets += p->tickets;
  release(&p->lock);
}
if(total_tickets == 0)
  continue;

next_random += ticks;
int winning_ticket = randtolottery() % total_tickets;
int ticket_counter = 0;
for(p = proc; p < &proc[NPROC]; p++) {
  acquire(&p->lock);
  if(p->state == RUNNABLE) {
    ticket_counter += p->tickets;
    if(ticket_counter > winning_ticket) {   // dono do bilhete vencedor
      p->state = RUNNING;
      c->proc = p;
      swtch(&c->context, &p->context);
      c->proc = 0;
      release(&p->lock);
      break;
    }
  }
  release(&p->lock);
}
```

Cada `p->lock` é adquirido e liberado individualmente, preservando a disciplina
de travas do xv6. Não há divisão por zero: o sorteio só ocorre quando
`total_tickets > 0`.

### 2.5 A chamada `settickets(int)`

```c
int settickets(int n) {
  struct proc *p = myproc();
  if (n < 1)          // rejeita 0 e valores negativos
    return -1;
  acquire(&p->lock);
  p->tickets = n;
  release(&p->lock);
  return 0;
}
```

A validação `n < 1` é o principal ponto de robustez: impede que um processo se
atribua **0 bilhetes** (o que causaria *starvation* permanente) ou um valor
negativo (que corromperia a soma de bilhetes).

---

## 3. Metodologia experimental

O programa de teste [`testeloteria.c`](../tests/testeloteria.c) dos colegas:

1. Cria **3 filhos** com **10, 20 e 30 bilhetes** (via `settickets`).
2. **Barreira com dois *pipes*:** cada filho sinaliza "pronto" e bloqueia; o pai
   espera todos ficarem prontos e só então libera a largada. Assim a medição
   começa ~ao mesmo tempo para todos, sem a vantagem que o primeiro filho criado
   teria enquanto os demais ainda não existem.
3. Cada filho executa uma carga CPU-bound idêntica durante a mesma janela de
   tempo (padrão 600 ticks; configurável: `testeloteria <ticks>`), contando
   quantas *iterações* completou.
4. Cada filho imprime `bilhetes=.. pid=.. iteracoes=..`.

**Ambiente:** xv6-riscv em QEMU `riscv64`, executado com **uma única CPU**
(`make qemu CPUS=1`). A restrição a uma CPU é necessária porque, com várias CPUs,
os filhos rodam em paralelo e as contagens deixam de refletir a razão de
bilhetes.

---

## 4. Resultados

### 4.1 Proporção 1:2:3 (`testeloteria 200`)

Bilhetes 10, 20 e 30 (total = 60):

| PID | Bilhetes | Iterações | CPU% obtido | CPU% ideal ($t_i/T$) |
|-----|----------|----------|-------------|----------------------|
| 6   | 30       | 490550   | **48%**     | 50% |
| 5   | 20       | 333036   | **33%**     | 33% |
| 4   | 10       | 198311   | **19%**     | 17% |

Total de iterações = 1.021.897. O CPU obtido acompanha de perto o valor ideal,
com desvio de poucos pontos percentuais — compatível com a natureza estatística
do sorteio em uma janela finita. Repetindo o teste, o processo com mais bilhetes
vence consistentemente mais sorteios.

### 4.2 Comparação com o Round Robin

O escalonador original do xv6 percorre a tabela de processos de forma circular e
dá a cada processo `RUNNABLE` um quantum, **ignorando qualquer noção de
prioridade ou peso**. Consequentemente, três processos CPU-bound receberiam
aproximadamente 33% da CPU cada, independentemente de sua importância.

Com o *Lottery Scheduling*, a mesma carga (10/20/30 bilhetes) resulta em
aproximadamente 19/33/48% — ou seja, o mecanismo passa a **respeitar o peso
relativo** de cada processo. Note ainda que, se todos tivessem o mesmo número de
bilhetes, o sorteio daria a cada um ~$1/N$ da CPU, degenerando exatamente no
comportamento igualitário do Round Robin. Essa é a diferença fundamental entre um
escalonador de tempo igualitário (RR) e um de compartilhamento proporcional
(Lottery).

---

## 5. Revisão de qualidade — casos de borda ("furos")

| Caso | Comportamento observado | Conclusão |
|------|-------------------------|-----------|
| `settickets(0)` | Retorna `-1`; o processo **mantém** os bilhetes que já tinha. | Rejeitado com segurança; sem *starvation*. |
| `settickets(-5)` | Mesma validação `n < 1` ⇒ retorna `-1`. | Valores negativos nunca entram na soma de bilhetes. |
| Processo após `settickets(0)` rejeitado | Continua executável (mantém ≥ 1 bilhete). | **Não há inanição:** todo processo `RUNNABLE` tem probabilidade > 0. |
| Nenhum processo `RUNNABLE` | `total_tickets == 0` ⇒ `continue` (sem sorteio). | Sem divisão por zero. |

Evidência (`testerobustez`):

```console
== Teste de robustez do settickets ==
settickets(0)   -> -1  REJEITADO (ok)
settickets(-5)  -> -1  REJEITADO (ok)
settickets(10)  -> 0  ACEITO (ok)
filho pid=4 executou apos settickets(0) -> sem starvation
== fim ==
```

A validação `n < 1` em `settickets` (adicionada na integração) é o principal ponto
de robustez: impede que um processo se atribua **0 bilhetes** (o que causaria
*starvation* permanente) ou um valor negativo (que corromperia a soma de
bilhetes).

### 5.1 Regressão do kernel

Para garantir que a troca do escalonador não quebrou o restante do sistema,
executamos a suíte oficial do xv6:

```console
$ usertests -q
...
ALL TESTS PASSED
```

Todos os testes rápidos passaram. As mensagens `usertrap(): unexpected scause...`
que aparecem durante `kernmem`, `MAXVAplus`, `nowrite`, `sbrkbugs` e `lazy_unmap`
são **esperadas**: esses testes provocam faltas de propósito e cada um termina
com `OK`.

---

## 6. Como reproduzir

Veja o passo a passo completo no [README principal](../README.md). Em resumo:

```bash
cd code
make qemu CPUS=1
# no shell do xv6:
$ testeloteria 200     # teste de proporcionalidade (10/20/30 bilhetes)
$ testerobustez        # casos de borda (0 e negativos rejeitados)
$ usertests -q         # regressão do kernel -> ALL TESTS PASSED
# para sair do QEMU: Ctrl-A e depois X
```

---

## 7. Referências

- WALDSPURGER, C. A.; WEIHL, W. E. *Lottery Scheduling: Flexible Proportional-Share
  Resource Management*. OSDI '94, USENIX, 1994.
- MIT PDOS. *xv6: a simple, Unix-like teaching operating system* (RISC-V), 2024.
