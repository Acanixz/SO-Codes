//Implemente um sistema em que há três threads que tem a responsabilidade de ler o valor de um sensor e some ao valor de uma variável global e em uma variável local. 
// Você deve simular a contagem com operação de incremento simples (+=1 ou ++). 
// Print a variável local a cada 1 segundo e incremente a variável a cada 1 segundo. 
// O programa deve chegar ao fim quando a soma da variável global chegar a 100. 

// Ao fim, você consegue observar qual condição:
//  1. Todas as threads tem o mesmo valor na variável interna?
//  2. O print da variável global segue um incremento linear?

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h> 
#include<unistd.h>

#define COUNT_READ 12
int var_global = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *th_sensor1(void *param){
    int var_local = 0;
    while (1){
        var_local++;
        pthread_mutex_lock(&lock);
        if (var_global<COUNT_READ){
            var_global++;
        } else {
            pthread_mutex_unlock(&lock);
            pthread_exit(0);
        }
        usleep(1000*1000);
        printf("Var local th1: %d\n", var_local);
    }
}

void *th_sensor2(void *param){
    int var_local = 0;
    while (1){
        var_local++;
        pthread_mutex_lock(&lock);
        if (var_global<COUNT_READ){
            var_global++;
        } else {
            pthread_mutex_unlock(&lock);
            pthread_exit(0);
        }
        usleep(1000*1000);
        printf("Var local th2: %d\n", var_local);
    }
}

void *th_sensor3(void *param){
    int var_local = 0;
    while (1){
        var_local++;
        pthread_mutex_lock(&lock);
        if (var_global<COUNT_READ){
            var_global++;
        } else {
            pthread_mutex_unlock(&lock);
            pthread_exit(0);
        }
        usleep(1000*1000);
        printf("Var local th3: %d\n", var_local);
    }
}


int main(){ 
    pthread_t th_s1, th_s2, th_s3;
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    pthread_create(&th_s1, &attr, th_sensor1, NULL);
    pthread_create(&th_s2, &attr, th_sensor2, NULL);
    pthread_create(&th_s3, &attr, th_sensor3, NULL);

    pthread_join(th_s1, NULL);
    pthread_join(th_s2, NULL);
    pthread_join(th_s3, NULL);

    printf("Var global: %d\n", var_global);
}