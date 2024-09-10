#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h> // para usar Threads
#include <semaphore.h> // para usar Semaforos
#include <stdbool.h>

// Se definen las variables globales
int cantInvitados = 4;
int cantMozos = 2;
int cantPlatos = 0;
int cantInvitadosSentados = 0;
bool preguntaFueRespondida = false; // Inicia en falso

// Se definen los semáforos
sem_t todosEstanSentados;
sem_t mozosServir;
sem_t puedenComer;
sem_t puedenResponder;
sem_t alguienRespondio;
sem_t puedenLevantarse;
sem_t invitadosSentados;

// Función para simular el tiempo
void simularTiempo(){
    int numero = 0;
    for (int i = 0; i < 99999999; i++){
        numero = numero + i;
    }
}

// Función para sentarse
void sentarse(char sentado) {
    simularTiempo();
    if (sentado == 'i') {
        printf("Se sentó un invitado\n");
    } else if (sentado == 'm') {
        printf("Se sentó Manucho\n");
    }
}

// Función para servir comida
void servirComida() {
    printf("Un mozo sirvió un plato\n");
    simularTiempo();
}

// Función para enojarse
void enojarse() {
    printf("Manucho se enojó por la respuesta que recibió a su pregunta mundialista\n"); 
    simularTiempo();
}

// Función para levantarse
void levantarse(char levantado) {
    simularTiempo();
    if (levantado == 'i') {
        printf("Se levantó un invitado\n");
    } else if (levantado == 'm') {
        printf("Se levantó Manucho\n");
    }
}

// Función para lanzar la pregunta mundialista
void lanzar_pregunta_mundialista() {
    printf("Manucho realiza la pregunta mundialista\n");
    simularTiempo();
}

// Función para lanzar la respuesta mundialista
void lanzar_respuesta_mundialista() {
    printf("Un invitado responde la pregunta de Manucho\n");
    simularTiempo();
}

// Funciones de los hilos
void* accionesMozo() { 
    sem_wait(&mozosServir); //esperan a que Manucho se siente
    printf("El mozo está listo para servir.\n");

    //sirven mientras la cantidad de platos sea != a n+1
    while (cantPlatos != cantInvitados + 1) {
        cantPlatos++;
        printf("Sirviendo plato %d de %d\n", cantPlatos, cantInvitados + 1); // Print para ver el progreso
        servirComida();
        sem_post(&puedenComer); //habilitan a comer a los comensales
        printf("Se habilitó a un comensal para comer.\n"); // Debug
    }
    printf("Todos los platos han sido servidos.\n");
    pthread_exit(NULL);
}

void* accionesInvitado() {
    //los invitados se sientan hasta que cantInvitadosSentados==cantInvitados
    sem_wait(&invitadosSentados);
    sentarse('i');
    
    cantInvitadosSentados++;
    printf("Cantidad de invitados sentados: %d de %d\n", cantInvitadosSentados, cantInvitados); // Debug
    if (cantInvitadosSentados == cantInvitados) {
        printf("Todos los invitados están sentados. Manucho puede sentarse.\n");
        sem_post(&todosEstanSentados); //habilitan a manucho a sentarse
    }
    sem_post(&invitadosSentados); //dejan de sentarse
    
    sem_wait(&puedenComer);
    printf("Un invitado está comiendo...\n"); // Debug

    //comen ...    

    sem_wait(&puedenResponder); //esperan a que manucho habilite la respuesta
    printf("Un invitado está esperando para responder.\n"); // Debug
    if (!preguntaFueRespondida) {
        preguntaFueRespondida = true;
        sem_post(&alguienRespondio);
        printf("Un invitado respondió la pregunta mundialista.\n"); // Debug
        lanzar_respuesta_mundialista();
        sem_post(&alguienRespondio); 
    }
    //si la pregunta ya se respondio, esperan a que manucho habilite que se levanten
    sem_wait(&puedenLevantarse);
    levantarse('i');
    pthread_exit(NULL);
}

void* accionesManucho() { 
    //manucho espera a que todos los invitados esten sentados
    sem_wait(&todosEstanSentados); 
    sentarse('m');
    printf("Manucho se ha sentado.\n"); // Debug
    
    //manucho se sienta y habilita a los mozos a servir la comida
    sem_post(&mozosServir);
    sem_wait(&puedenComer);
    printf("Manucho está comiendo.\n"); // Debug
    
    //come ... 

    lanzar_pregunta_mundialista();
    printf("Manucho hizo la pregunta mundialista.\n"); // Debug
    sem_post(&puedenResponder); //habilita a los invitados a realizar la respuesta mundialista
    sem_wait(&alguienRespondio); //manucho espera a que alguien responda

    printf("Manucho está esperando una respuesta.\n"); // Debug
    enojarse();
    levantarse('m');  
    printf("Manucho se levanto.\n"); // Debug
    sem_post(&puedenLevantarse); //luego de que se levanta, habilita al resto a hacer lo mismo
    pthread_exit(NULL);
}

int main() {
    // Inicialización de los semáforos
    sem_init(&todosEstanSentados, 0, 0);
    sem_init(&mozosServir, 0, 0);
    sem_init(&puedenComer, 0, 0);
    sem_init(&puedenResponder, 0, 0);
    sem_init(&alguienRespondio, 0, 0);
    sem_init(&puedenLevantarse, 0, 0);
    sem_init(&invitadosSentados, 0, 1);

    // Se crean los hilos
    pthread_t manucho;
    pthread_t invitado[cantInvitados];
    pthread_t mozo[cantMozos];
    
    pthread_create(&manucho, NULL, accionesManucho, NULL);
    for (int i = 0; i < cantInvitados; i++) {
        pthread_create(&invitado[i], NULL, accionesInvitado, NULL);
    }
    for (int i = 0; i < cantMozos; i++) {
        pthread_create(&mozo[i], NULL, accionesMozo, NULL);
    }

    // Espera a que todos los hilos terminen
    for (int i = 0; i < cantMozos; i++) {
        pthread_join(mozo[i], NULL);
    }
    for (int i = 0; i < cantInvitados; i++) {
        pthread_join(invitado[i], NULL);
    }
    pthread_join(manucho, NULL);
    
    // Se destruyen los semáforos
    sem_destroy(&todosEstanSentados);
    sem_destroy(&mozosServir);
    sem_destroy(&puedenComer);
    sem_destroy(&puedenResponder);
    sem_destroy(&alguienRespondio);
    sem_destroy(&puedenLevantarse);
    sem_destroy(&invitadosSentados);
    
    return 0;
}
