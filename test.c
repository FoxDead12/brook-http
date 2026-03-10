// /usr/bin/gcc -g test.c -I/opt/homebrew/include -L/opt/homebrew/lib -lcjson -lbeanstalkclient -o test
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <beanstalkclient.h>

static char *host = "127.0.0.1";
static char *port = "11301";
static int keep_running = 1;

// --- Callbacks ---
void delete_cb(bsc *client, struct bsc_delete_info *info);

void reserve_cb(bsc *client, struct bsc_reserve_info *info) {
    if (info->response.code == BSC_RESERVE_RES_RESERVED) {
        printf("\n---------------------------------------\n");
        printf("RECEBIDO ID: %llu\n", info->response.id);
        printf("DADOS: %.*s\n", info->response.bytes, info->response.data);

        // Simular processamento aqui...

        // Confirmar conclusão ao Beanstalkd
        bsc_delete(client, delete_cb, NULL, info->response.id);
    } else {
        // Se der erro (ex: conexão perdida), tentamos o reserve de novo após um tempo
        printf("[AVISO] Resposta inesperada no reserve: %d. Tentando de novo...\n", info->response.code);
        bsc_reserve(client, reserve_cb, NULL, BSC_RESERVE_NO_TIMEOUT);
    }
}

void delete_cb(bsc *client, struct bsc_delete_info *info) {
    if (info->response.code == BSC_DELETE_RES_DELETED) {
        printf("[OK] Job %llu processado e removido.\n", info->request.id);
    } else {
        printf("[ERRO] Falha ao apagar job %llu (Código: %d)\n", info->request.id, info->response.code);
    }

    // IMPORTANT: Pedir o próximo job imediatamente após terminar este
    printf("\nAguardando próximo job...\n");
    bsc_reserve(client, reserve_cb, NULL, BSC_RESERVE_NO_TIMEOUT);
}

// --- Event Loop Principal ---

int main() {
    char errstr[BSC_ERRSTR_LEN];
    bsc *client;
    fd_set readset, writeset;

    client = bsc_new(host, port, "default", NULL, 4096, 4096, 1024, errstr);
    if (!client) {
        fprintf(stderr, "Erro ao conectar: %s\n", errstr);
        return 1;
    }

    // O Beanstalkd por padrão ouve o "default".
    // Se o teu produtor envia para "baba", tens de dar WATCH.
    bsc_watch(client, NULL, NULL, "third-job");
    // bsc_ignore(client, NULL, NULL, "third-job");

    // Iniciar a primeira reserva
    printf("Worker iniciado. Ouvindo tube: default\n");
    bsc_reserve(client, reserve_cb, NULL, BSC_RESERVE_NO_TIMEOUT);

    // O loop infinito do consumidor
    while (keep_running) {
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_SET(client->fd, &readset);

        // Se houver comandos para enviar (como o delete), ativa o bit de escrita
        if (!AQ_EMPTY(client->outq)) {
            FD_SET(client->fd, &writeset);
        }

        if (select(client->fd + 1, &readset, &writeset, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(client->fd, &readset)) {
          bsc_read(client);
        }

        if (FD_ISSET(client->fd, &writeset)) {
            bsc_write(client);
        }
    }

    bsc_free(client);
    return 0;
}
