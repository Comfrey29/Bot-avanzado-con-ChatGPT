/******************************************************************************

Welcome to GDB Online.
  GDB online is an online compiler and debugger tool for C, C++, Python, PHP, Ruby, 
  C#, OCaml, VB, Perl, Swift, Prolog, Javascript, Pascal, COBOL, HTML, CSS, JS
  Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <math.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER "127.0.0.1"

void *run_server(void *arg);
void *run_client(void *arg);
void ia_resposta(int new_socket, char *missatge);
void calculadora(int new_socket);

int main() {
    pthread_t server_thread, client_thread;

    // Iniciar el servidor en un fil
    if (pthread_create(&server_thread, NULL, run_server, NULL) != 0) {
        perror("No s'ha pogut crear el fil del servidor");
        return 1;
    }

    // Esperar una mica perquè el servidor s'iniciï
    sleep(1);

    // Iniciar el client en un altre fil
    if (pthread_create(&client_thread, NULL, run_client, NULL) != 0) {
        perror("No s'ha pogut crear el fil del client");
        return 1;
    }

    // Esperar que els fils acabin
    pthread_join(server_thread, NULL);
    pthread_join(client_thread, NULL);

    return 0;
}

void *run_server(void *arg) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Crear socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Creació de socket fallida");
        return NULL;
    }
    printf("Servidor: Socket creat amb èxit.\n");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Enllaçar
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Enllaç fallit");
        return NULL;
    }
    printf("Servidor: Socket enllaçat al port %d.\n", PORT);

    // Escoltar
    if (listen(server_fd, 3) < 0) {
        perror("Error en escoltar");
        return NULL;
    }
    printf("Servidor: Escoltant al port %d...\n", PORT);

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Error en acceptar connexió");
        return NULL;
    }
    printf("Servidor: Connexió acceptada!\n");

    // Benvinguda al client
    char benvinguda[] = "Benvingut al servidor! Escriu 'Ajuda' per veure les opcions disponibles.";
    send(new_socket, benvinguda, strlen(benvinguda), 0);
    printf("Servidor: Missatge de benvinguda enviat.\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            printf("Servidor: Client desconnectat.\n");
            break;
        }
        printf("Servidor: Missatge rebut: %s\n", buffer);

        ia_resposta(new_socket, buffer);
    }

    close(new_socket);
    close(server_fd);
    printf("Servidor: Tancat.\n");
    return NULL;
}

void ia_resposta(int new_socket, char *missatge) {
    char resposta[BUFFER_SIZE];

    // Comanda "Hola"
    if (strstr(missatge, "Hola")) {
        strcpy(resposta, "Hola! Tenim les següents categories per ajudar-te: Programació, Matemàtiques, Francès. Escriu la categoria per obtenir més informació.");
    }
    // Comanda "Programació"
    else if (strstr(missatge, "Programació")) {
        strcpy(resposta, "La programació és l'art de crear programes mitjançant codi. Alguns llenguatges populars inclouen JavaScript, Python, C, etc.");
    }
    // Comanda "Matemàtiques"
    else if (strstr(missatge, "Matemàtiques")) {
        strcpy(resposta, "Les matemàtiques inclouen branques com l'Àlgebra, Càlcul i Geometria. Puc ajudar-te amb exercicis.");
    }
    // Comanda "Francès"
    else if (strstr(missatge, "Francès")) {
        strcpy(resposta, "El Francès és una llengua romànica. Algunes paraules bàsiques inclouen 'Bonjour' (Bon dia) i 'Merci' (Gràcies).");
    }
    // Comanda "Ajuda"
    else if (strstr(missatge, "Ajuda")) {
        strcpy(resposta, "Comandes disponibles:\n1. Programació\n2. Matemàtiques\n3. Francès\n4. Calculadora\n5. Clear (per esborrar la pantalla)\n6. Ajuda (per veure aquesta llista)");
    }
    // Comanda "Calculadora"
    else if (strstr(missatge, "Calculadora")) {
        calculadora(new_socket);
        return;
    }
    // Comanda "Clear" o "cls" (esborrar pantalla)
    else if (strstr(missatge, "Clear") || strstr(missatge, "cls")) {
        strcpy(resposta, "\033[H\033[J");  // Comanda per esborrar la pantalla
    }
    // Resposta per defecte
    else {
        strcpy(resposta, "No he entès el teu missatge. Escriu 'Ajuda' per veure les opcions disponibles.");
    }

    send(new_socket, resposta, strlen(resposta), 0);
    printf("Servidor: Resposta enviada: %s\n", resposta);
}

void calculadora(int new_socket) {
    char resposta[BUFFER_SIZE];
    double num1, num2, resultat;
    char operacio[BUFFER_SIZE];

    send(new_socket, "Benvingut a la calculadora! Escriviu l'operació en el format: número1 operació número2\nPer exemple: 5 + 3", 100, 0);

    memset(operacio, 0, BUFFER_SIZE);
    int valread = read(new_socket, operacio, BUFFER_SIZE);
    if (valread <= 0) {
        strcpy(resposta, "Error llegint operació.");
        send(new_socket, resposta, strlen(resposta), 0);
        return;
    }

    if (sscanf(operacio, "%lf %s %lf", &num1, operacio, &num2) == 3) {
        if (strcmp(operacio, "+") == 0) {
            resultat = num1 + num2;
        } else if (strcmp(operacio, "-") == 0) {
            resultat = num1 - num2;
        } else if (strcmp(operacio, "*") == 0) {
            resultat = num1 * num2;
        } else if (strcmp(operacio, "/") == 0) {
            if (num2 != 0) {
                resultat = num1 / num2;
            } else {
                strcpy(resposta, "No es pot dividir per zero.");
                send(new_socket, resposta, strlen(resposta), 0);
                return;
            }
        } else {
            strcpy(resposta, "Operació no vàlida.");
            send(new_socket, resposta, strlen(resposta), 0);
            return;
        }

        snprintf(resposta, BUFFER_SIZE, "El resultat de l'operació %lf %s %lf és: %lf", num1, operacio, num2, resultat);
    } else {
        strcpy(resposta, "Operació invàlida. Escriviu en el format: número1 operació número2 (per exemple: 5 + 3)");
    }

    send(new_socket, resposta, strlen(resposta), 0);
    printf("Servidor: Resposta de la calculadora enviada: %s\n", resposta);
}

void *run_client(void *arg) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Client: Error en la creació del socket\n");
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, SERVER, &serv_addr.sin_addr) <= 0) {
        printf("Client: Adreça invàlida\n");
        return NULL;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Client: La connexió ha fallat\n");
        return NULL;
    }
    printf("Client: Connectat al servidor!\n");

    while (1) {
        printf("Tu: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        send(sock, buffer, strlen(buffer), 0);
        printf("Client: Missatge enviat.\n");

        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(sock, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            printf("Client: Servidor desconnectat.\n");
            break;
        }
        printf("Servidor: %s\n", buffer);

        if (strstr(buffer, "Fins aviat!")) {
            printf("Client: Tancant connexió...\n");
            break;
        }
    }

    close(sock);
    printf("Client: Connexió tancada.\n");
    return NULL;
}


