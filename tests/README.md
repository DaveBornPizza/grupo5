# Testes — Lottery Scheduling

Esta pasta contém os programas usados para validar experimentalmente o
escalonador *Lottery Scheduling* implementado no xv6.

## `testeloteria.c` — teste de proporcionalidade

Programa de usuário que cria 3 processos filhos com **10, 20 e 30 bilhetes** e os
faz competir pela CPU. Usa uma **barreira com dois pipes**: todos os filhos são
criados e sinalizam "pronto"; só quando todos estão prontos o pai libera a
largada, de modo que a medição comece ~ao mesmo tempo para todos. Cada filho
executa uma carga CPU-bound idêntica durante a mesma janela de tempo e reporta
quantas *iterações* completou.

Se o escalonamento for proporcional, o número de iterações de cada processo deve
ficar na razão dos seus bilhetes (10 : 20 : 30).

> É o mesmo arquivo integrado à árvore de compilação em
> [`code/user/testeloteria.c`](../code/user/testeloteria.c), registrado em
> `UPROGS` no `Makefile`.

Uso (dentro do shell do xv6):

```console
$ testeloteria            # janela padrão de 600 ticks
$ testeloteria 200        # janela de 200 ticks (mais rápido)
```

Saída de exemplo (real, `testeloteria 200`):

```console
bilhetes=30 pid=6 iteracoes=490550
bilhetes=20 pid=5 iteracoes=333036
bilhetes=10 pid=4 iteracoes=198311
```

Proporção observada ≈ 48% / 33% / 19%, próxima do ideal 50% / 33% / 17%.

## `testerobustez.c` — revisão de qualidade (integração)

Teste adicionado na etapa de integração para exercitar os **casos de borda**
("furos") da syscall `settickets`:

- `settickets(0)` → deve ser **rejeitado** (retorno `-1`);
- `settickets(-5)` → deve ser **rejeitado**;
- `settickets(10)` → deve ser **aceito** (retorno `0`);
- um processo cujo pedido de 0 bilhetes foi rejeitado **continua executando**
  (mantém os bilhetes herdados) — comprovando que **não há starvation**.

Uso:

```console
$ testerobustez
```

Saída de exemplo (real):

```console
== Teste de robustez do settickets ==
settickets(0)   -> -1  REJEITADO (ok)
settickets(-5)  -> -1  REJEITADO (ok)
settickets(10)  -> 0  ACEITO (ok)
filho pid=4 executou apos settickets(0) -> sem starvation
== fim ==
```

> **Importante:** rode o xv6 com **uma única CPU** (`make qemu CPUS=1`). A
> proporcionalidade só é observável em uma CPU; com várias, os filhos rodam em
> paralelo e as contagens deixam de refletir a razão de bilhetes.
