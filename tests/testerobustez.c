// testerobustez.c — Revisão de qualidade (integração).
//
// Valida os casos de borda ("furos") da syscall settickets do Lottery
// Scheduling: o que acontece se um processo pedir 0 bilhetes? Valores
// negativos? Um valor acima do limite (MAX_TICKETS)? O processo sofre
// starvation ou o kernel quebra?
//
// Uso: testerobustez

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

int
main(void)
{
  printf("== Teste de robustez do settickets ==\n");

  // Caso 1: 0 bilhetes deve ser REJEITADO (retorno -1).
  int r0 = settickets(0);
  printf("settickets(0)        -> %d  %s\n", r0,
         r0 < 0 ? "REJEITADO (ok)" : "ACEITO (FALHA!)");

  // Caso 2: valor negativo deve ser REJEITADO.
  int rn = settickets(-5);
  printf("settickets(-5)       -> %d  %s\n", rn,
         rn < 0 ? "REJEITADO (ok)" : "ACEITO (FALHA!)");

  // Caso 3: acima de MAX_TICKETS deve ser REJEITADO.
  int rmax = settickets(MAX_TICKETS + 1);
  printf("settickets(MAX+1)    -> %d  %s\n", rmax,
         rmax < 0 ? "REJEITADO (ok)" : "ACEITO (FALHA!)");

  // Caso 4: valor válido deve ser ACEITO (retorno 0).
  int rv = settickets(10);
  printf("settickets(10)       -> %d  %s\n", rv,
         rv == 0 ? "ACEITO (ok)" : "FALHA!");

  // Caso 5: mesmo após um pedido inválido, o processo continua executável
  // (mantém os bilhetes que já tinha) — ou seja, NÃO há starvation nem crash.
  int pid = fork();
  if (pid < 0) {
    printf("fork falhou\n");
    exit(1);
  }
  if (pid == 0) {
    settickets(0); // rejeitado; o filho mantém os bilhetes herdados (>= 1)
    volatile long acc = 0;
    for (long i = 0; i < 30000000; i++)
      acc += i;
    printf("filho pid=%d executou apos settickets(0) -> sem starvation\n",
           getpid());
    exit(0);
  }
  wait(0);

  printf("== fim ==\n");
  exit(0);
}
