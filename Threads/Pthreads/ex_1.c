// Desenvolver uma aplicação de leia uma entrada do teclado, 
// some com uma constante e imprima na tela o dado resultante da soma.
// Divida as tarefas em threads usando a biblioteca pthread (preferencialmente).

// Para compilar: gcc ex_1.c -o NOME_OUTPUT -lpthread

#include <pthread.h>
#include <stdio.h>

int var = 0;

void *teclado(void *param){
    printf("Forneça um numero: ");
    char num;
    scanf("%d", &var);
}

void *somar(void *param){
    var = var + 10;
}

void *get_soma(void *param){
    printf("O valor atual da soma é: %d\n", var);
}

int main(){
    // Thread_Teclado, ..soma, ...imprime
    pthread_t th_t, th_s, th_i;
    pthread_attr_t attr; // Atributos de configuração, compartilhado entre os threads

    // Inicializa o endereço de atributos com valores padrão
    pthread_attr_init(&attr);

    // Cria cada thread
    // ID, Atributo, Método, (restrições?)

    // Nota: isso intencionalmente falha, pois os 3 threads executam
    // então a soma e print ocorreram antes mesmo de acabar de digitar
    // "Correção": Adicionar os joins após cada create, porém isso torna o código sequencial
    pthread_create(&th_t, &attr, teclado, NULL);
    pthread_join(th_t, NULL);
    pthread_create(&th_s, &attr, somar, NULL);
    pthread_join(th_s, NULL);
    pthread_create(&th_i, &attr, get_soma, NULL);

    // Aguarda os threads, sem esperar retorno
    // pthread_join(&th_t, NULL);
    // pthread_join(&th_s, NULL);
    pthread_join(th_i, NULL);

    printf("Fim do processo");
    return 0;
}

// Por serem separadas em nivel de memória, os métodos devem começar com * e conter void *param (opcionalmente)