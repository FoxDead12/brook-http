#include "core/configure.h"

int
brook_load_configuration ( brook_conf_t* config, const char* path ) {

  FILE *file = fopen(path, "r");
  if (!file) return -1;

  char line[512];
  int section = 0; // 0: global, 1: beanstalkd, 2: redis

  while (fgets(line, sizeof(line), file)) {
    // Detetar mudança de secção pela indentação ou palavra-chave
    if (strstr(line, "beanstalkd:")) { section = 1; continue; }
    if (strstr(line, "redis:"))      { section = 2; continue; }

    // Se a linha não começar com espaço e não for uma das secções, volta ao global
    if (line[0] != ' ' && line[0] != '\t' && strchr(line, ':')) {
      section = 0;
    }

    if (section == 0) { // GLOBAL
      if (strstr(line, "name:"))       clean_value(config->name, line, 32);
      if (strstr(line, "port:"))       { char p[10]; clean_value(p, line, 10); config->port = atoi(p); }
      if (strstr(line, "gatekeeper:")) clean_value(config->gatekeeper, line, 256);
      if (strstr(line, "log:")) clean_value(config->gatekeeper, line, 1024);
    }
    else if (section == 1) { // BEANSTALKD
      if (strstr(line, "host:")) clean_value(config->beanstalkd.host, line, 64);
      if (strstr(line, "port:")) { char p[10]; clean_value(p, line, 10); config->beanstalkd.port = atoi(p); }
    }
    else if (section == 2) { // REDIS
      if (strstr(line, "host:")) clean_value(config->redis.host, line, 64);
      if (strstr(line, "port:")) { char p[10]; clean_value(p, line, 10); config->redis.port = atoi(p); }
    }
  }

  fclose(file);
  return 0;
}

void
clean_value ( char *dest, const char *src, int max_len ) {
  const char *start = strchr(src, ':');
  if (!start) return;
  start++; // pula o ':'

  // Pula espaços e aspas iniciais
  while (*start && (isspace(*start) || *start == '"')) start++;

  strncpy(dest, start, max_len);

  // Remove espaços e aspas finais
  char *end = dest + strlen(dest) - 1;
  while (end > dest && (isspace(*end) || *end == '"' || *end == '\n' || *end == '\r')) {
      *end = '\0';
      end--;
  }
}
