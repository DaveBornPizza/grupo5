#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_PROCESSOS 3

int
main(int argc, char *argv[])
{
  int duracao = 600;
  int bilhetes[NUM_PROCESSOS] = {10, 20, 30};
  int inicio[2];
  int pronto[2];

  if(argc == 2){
    duracao = atoi(argv[1]);

    if(duracao <= 0){
      printf("Uso: testeloteria [duracao_em_ticks]\n");
      exit(1);
    }
  }

  /*
   * Os pipes servem como uma barreira:
   * todos os filhos são criados primeiro e só depois começam
   * o laço de trabalho.
   */
  if(pipe(inicio) < 0 || pipe(pronto) < 0){
    printf("Erro ao criar pipes\n");
    exit(1);
  }

  for(int i = 0; i < NUM_PROCESSOS; i++){
    int pid = fork();

    if(pid < 0){
      printf("Erro no fork\n");
      exit(1);
    }

    if(pid == 0){
      close(inicio[1]);
      close(pronto[0]);

      if(settickets(bilhetes[i]) < 0){
        printf("Erro ao definir %d bilhetes\n", bilhetes[i]);
        exit(1);
      }

      /*
       * Informa ao pai que este filho está pronto.
       */
      char sinal = 'P';
      write(pronto[1], &sinal, 1);

      /*
       * Aguarda a liberação simultânea pelo pai.
       */
      read(inicio[0], &sinal, 1);

      int tick_inicial = uptime();
      unsigned long iteracoes = 0;
      volatile unsigned long trabalho = 0;

      while(uptime() - tick_inicial < duracao){
        /*
         * Mesma carga para todos os processos.
         */
        for(int j = 0; j < 1000; j++){
          trabalho += j;
        }

        iteracoes++;
      }

      printf(
        "bilhetes=%d pid=%d iteracoes=%lu\n",
        bilhetes[i],
        getpid(),
        iteracoes
      );

      close(inicio[0]);
      close(pronto[1]);
      exit(0);
    }
  }

  close(inicio[0]);
  close(pronto[1]);

  /*
   * Aguarda os três filhos informarem que estão prontos.
   */
  for(int i = 0; i < NUM_PROCESSOS; i++){
    char sinal;
    read(pronto[0], &sinal, 1);
  }

  /*
   * Libera os três filhos.
   */
  for(int i = 0; i < NUM_PROCESSOS; i++){
    char sinal = 'I';
    write(inicio[1], &sinal, 1);
  }

  close(inicio[1]);
  close(pronto[0]);

  for(int i = 0; i < NUM_PROCESSOS; i++){
    wait(0);
  }

  exit(0);
}
