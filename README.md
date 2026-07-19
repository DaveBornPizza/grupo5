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

### Etapa 2 — Implementação prática *(esta entrega)*

Implementação da solução sobre o **xv6-riscv**: escalonador *Round Robin* substituído por
*Lottery Scheduling*, syscall `settickets(int)` e programa de teste `testeloteria`. O
escalonador e o teste foram implementados pelos colegas; na **integração** foram corrigidos os
pontos que impediam o código de compilar/funcionar (o campo `tickets` não estava declarado, a
constante `DEFAULT_TICKETS` não existia e a syscall `settickets` não estava registrada).

| Arquivo | Alteração | Origem |
|---------|-----------|--------|
| `kernel/proc.c` | PRNG `randtolottery()`, escalonador por sorteio, herança de bilhetes no `fork`. | colegas |
| `user/testeloteria.c` | Teste de proporcionalidade (10/20/30 bilhetes). | colegas |
| `kernel/proc.h` | Declaração do campo `int tickets` em `struct proc`. | integração |
| `kernel/param.h` | `#define DEFAULT_TICKETS 1`. | integração |
| `kernel/proc.c` | Função `settickets()` + remoção de comentário indevido. | integração |
| `kernel/defs.h`, `kernel/syscall.h`, `kernel/syscall.c`, `kernel/sysproc.c` | Registro da syscall `settickets` (nº 22). | integração |
| `user/user.h`, `user/usys.pl` | Interface da syscall para userspace. | integração |
| `user/testerobustez.c`, `Makefile` | Teste de robustez dos casos de borda. | integração |

O detalhamento completo, a metodologia e os resultados estão em
[`report/RELATORIO.md`](report/RELATORIO.md).

---

## Estrutura do repositório

```
grupo5/
├── README.md            # este arquivo (roteiro de execução)
├── slides/              # slides da apresentação (Etapa 1)
├── video/               # link do vídeo da apresentação (Etapa 1)
├── code/                # xv6-riscv com o Lottery Scheduling (Etapa 2)
├── tests/               # testeloteria.c e testerobustez.c (Etapa 2)
└── report/              # relatório técnico (Etapa 2)
```

---

## Pré-requisitos

Para compilar e rodar o xv6 são necessários o **compilador cruzado RISC-V** e o **QEMU**.

**Linux (Debian/Ubuntu):**

```bash
sudo apt-get update
sudo apt-get install -y git build-essential gcc-riscv64-unknown-elf \
                        binutils-riscv64-unknown-elf qemu-system-misc
```

**macOS (Homebrew):**

```bash
brew tap riscv-software-src/riscv
brew install riscv-gnu-toolchain qemu
# dependências do gcc (caso o compilador reclame de bibliotecas ausentes):
brew install gmp mpfr libmpc isl
```

Verifique a instalação:

```bash
qemu-system-riscv64 --version
riscv64-unknown-elf-gcc --version
```

---

## Como compilar e executar (passo a passo)

Todos os comandos abaixo são executados a partir da pasta **`code/`**.

```bash
cd code
```

**1. (Opcional) Limpar artefatos de compilações anteriores:**

```bash
make clean
```

**2. Compilar o xv6 e iniciar o QEMU com UMA CPU:**

```bash
make qemu CPUS=1
```

> ⚠️ **Use `CPUS=1`.** A proporcionalidade do *Lottery Scheduling* só é observável em uma
> única CPU. Com várias CPUs os processos rodam em paralelo e as contagens deixam de refletir
> a razão de bilhetes.

Aguarde a mensagem de boot e o prompt do shell do xv6:

```console
xv6 kernel is booting
init: starting sh
$
```

**3. Rodar os testes** (dentro do shell do xv6):

```console
$ testeloteria            # proporcionalidade: 3 filhos com 10, 20 e 30 bilhetes (600 ticks)
$ testeloteria 200        # mesma coisa, janela menor (mais rápido)
$ testerobustez           # casos de borda: settickets(0) e negativos rejeitados
```

Saída de exemplo (real, `testeloteria 200`):

```console
$ testeloteria 200
bilhetes=30 pid=6 iteracoes=490550
bilhetes=20 pid=5 iteracoes=333036
bilhetes=10 pid=4 iteracoes=198311
```

O número de **iterações** de cada processo fica na razão dos seus bilhetes
(≈ 48% / 33% / 19% para 30 / 20 / 10 bilhetes; ideal 50% / 33% / 17%), comprovando o
compartilhamento proporcional. Cada execução leva a janela de ticks configurada
(alguns segundos); aguarde as três linhas de resultado.

**4. (Opcional) Validar que o kernel continua íntegro:**

```console
$ usertests -q
...
ALL TESTS PASSED
```

**5. Sair do QEMU:** pressione `Ctrl-A` e, em seguida, `x`.

---

## Casos de borda tratados

- **`settickets(0)` ou valores negativos** → a syscall retorna `-1` e o processo mantém os
  bilhetes que já tinha; não é possível zerar os próprios bilhetes.
- **Processo com o mínimo de bilhetes (1)** → continua recebendo uma fatia pequena, porém
  não nula, da CPU: **não há inanição (*starvation*)**.
- **Nenhum processo pronto** → o total de bilhetes é 0, nenhum sorteio ocorre, sem divisão
  por zero.

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
