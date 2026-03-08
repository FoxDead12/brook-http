#include "core/beanstalkd.h"

bsc* bean_client = NULL;

int
brook_beanstalkd_connect () {
  char errstr[BSC_ERRSTR_LEN];

  // ... create client of beanstalkd ...
  bean_client = bsc_new("127.0.0.1", "11301", "default", brook_benstalkd_connection_error, 1024, 1024, 256, errstr);
  if ( !bean_client ) {
    printf("Can't create beanstalkd client: %s\n", errstr);
    return BROOK_ERROR;
  }

  // ... connect client ...
  if ( !bsc_connect(bean_client, errstr) ) {
    printf("Can't connect connect to beantslakd: %s\n", errstr);
    return BROOK_ERROR;
  }

  return BROOK_OK;
}

int
brook_benstalkd_create_job ( brook_connection_t* con ) {

  // ... set tube to send job ...
  bsc_use(bean_client, brook_benstalkd_on_use, con, "default");

  // ... put job to beanstalkd
  bsc_put(bean_client, brook_benstalkd_on_put, con, con->job.priority, con->job.delay, con->job.ttr, con->job.data.len, con->job.data.data, false);

  // ... beanstalkd add event of write ...
  _fds[POOL_INDEX_BEANSTALKD].events |= POLLOUT;

  return BROOK_OK;
}

int
brook_benstalkd_write () {
  bsc_write(bean_client);
  if (AQ_NODES_FREE(bean_client->outq) == bean_client->outq->size && bean_client->outq_offset == 0) {
    _fds[POOL_INDEX_BEANSTALKD].events &= ~POLLOUT;
  }
  return BROOK_OK;
}

void
brook_benstalkd_connection_error ( bsc *client, bsc_error_t error ) {
  printf("beanstalkd connection error or protocol: %d\n", error);
}

void
brook_benstalkd_on_use ( bsc *client, struct bsc_use_info *info ) {
  printf(">>> Resposta lida do socket: USING tube %s\n", info->response.tube);
}

void
brook_benstalkd_on_put ( bsc *client, struct bsc_put_info *info ) {

  brook_connection_t* con = (brook_connection_t*) info->user_data;

  if (info->response.code == BSC_PUT_RES_INSERTED) {
    printf(">>> Resposta lida do socket: JOB INSERTED ID: %llu\n", info->response.id);
    con->job.id = info->response.id;
  } else {
    printf(">>> Resposta lida do socket: Erro código %d\n", info->response.code);
  }
}

//
// // Flag para sabermos quando o trabalho terminou
// static int job_done = 0;
//
// void on_put_done(bsc *client, struct bsc_put_info *info) {
//     if (info->response.code == BSC_PUT_RES_INSERTED) {
//         printf(">>> Resposta lida do socket: JOB INSERTED ID: %llu\n", info->response.id);
//     } else {
//         printf(">>> Resposta lida do socket: Erro código %d\n", info->response.code);
//     }
//     job_done = 1; // Sinaliza para parar o loop
// }
//
// void on_use_done(bsc *client, struct bsc_use_info *info) {
//     printf(">>> Resposta lida do socket: USING tube %s\n", info->response.tube);
// }
//
// void my_error_handler(bsc *client, bsc_error_t error) {
//     fprintf(stderr, "Erro de conexão ou protocolo: %d\n", error);
//     job_done = 1;
// }
//
// int
// brook_beanstalkd_connect () {
//   char errstr[BSC_ERRSTR_LEN];
//
//   // 1. Criar o cliente
//   bsc *client = bsc_new("127.0.0.1", "11301", "default", my_error_handler, 1024, 1024, 256, errstr);
//   if (!client) {
//       printf("Erro ao iniciar: %s\n", errstr);
//       return 1;
//   }
//
//   // 2. Conectar (Isso faz o connect() do socket)
//   if (!bsc_connect(client, errstr)) {
//       printf("Falha na conexão: %s\n", errstr);
//       return 1;
//   }
//
//   // 3. Agendar comandos (Eles entram na fila outq mas NÃO saem ainda)
//   printf("Agendando comandos...\n");
//   bsc_use(client, on_use_done, NULL, "meu_tubo_especifico");
//
//   char *msg = "Ola beanstalkd!";
//   bsc_put(client, on_put_done, NULL, 1024, 0, 60, strlen(msg), msg, false);
//
//   // 4. O LOOP CERTO: Processar o socket enquanto o job não terminar
//   // Em vez de 'for i < 10', usamos um loop que realmente atende o socket
//   printf("Iniciando processamento de leitura/escrita no socket...\n");
//
//   while (!job_done) {
//     // Tenta escrever dados pendentes no socket
//     bsc_write(client);
//
//     // Tenta ler respostas do socket
//     bsc_read(client);
//
//     // Pequena pausa para não fritar a CPU, já que é non-blocking
//     usleep(10000);
//   }
//
//   printf("Finalizado.\n");
//   bsc_free(client);
//   return BROOK_OK;
// }
//
// void
// brook_beanstalkd_on_error ( struct _bsc *b, bsc_error_t error_code ) {
//   const char *err_msg = "UNKNOWN";
//   // Mapeamento baseado no enum que enviaste
//   switch (error_code) {
//     case BSC_ERROR_NONE:         err_msg = "None (Success)"; break;
//     case BSC_ERROR_INTERNAL:     err_msg = "Internal Error"; break;
//     case BSC_ERROR_SOCKET:       err_msg = "Socket Error (Connection/IO)"; break;
//     case BSC_ERROR_MEMORY:       err_msg = "Memory Allocation Error"; break;
//     case BSC_ERROR_QUEUE_FULL:   err_msg = "Command Queue Full"; break;
//     default:                     err_msg = "Undocumented Error"; break;
//   }
//   fprintf(stderr, "[Beanstalk Event] Error Code: %d (%s)\n", error_code, err_msg);
// }
