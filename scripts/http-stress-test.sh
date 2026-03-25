#!/bin/bash

URL="http://localhost:6001/third-example"
URL2="http://localhost:6001/first-example"

echo "Disparando testes em paralelo (2000 requisições no total)..."

# --- BLOCO 1: POST para URL2 ---
# (
  echo "[Bloco 1] Iniciando 1000 POSTs para $URL2..."
  seq 1000 | xargs -I % -P 50 curl -s -X POST \
      -H "Content-Type: application/json" \
      -d "{\"id\": %, \"status\": \"testing\"}" \
      -o /dev/null "$URL2"
  echo "[Bloco 1] POSTs concluídos!"
# ) &
#
# # --- BLOCO 2: GET para URL ---
# (
#   echo "[Bloco 2] Iniciando 1000 GETs para $URL..."
#   seq 1000 | xargs -I % -P 50 curl -s -X GET \
#       -H "Content-Type: application/json" \
#       -d "{\"id\": %, \"status\": \"testing\"}" \
#       -o /dev/null "$URL"
#   echo "[Bloco 2] GETs concluídos!"
# ) &

# O comando 'wait' é CRUCIAL aqui.
# Ele faz o script esperar que ambos os processos em background (&) terminem
# antes de mostrar a mensagem final de "Concluído".
wait

echo "------------------------------------------"
echo "Todos os testes paralelos foram concluídos!"
