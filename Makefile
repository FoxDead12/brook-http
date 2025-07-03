CC = clang
CFLAGS = -Wall -Wextra -g \
    -I./src \
    -I./src/helpers \
    -I./src/helpers/files \
    -I./src/helpers/json \
    -I./src/helpers/request \
    -I./src/helpers/types \
    -I/usr/local/include

LDFLAGS = -L/usr/local/lib -ljson-c

BIN = http-c-broker
SRC_DIR = src
OBJ_DIR = build

# Fontes
SRCS := main.c $(shell find $(SRC_DIR) -name '*.c')

# Objetos
OBJ_MAIN := $(OBJ_DIR)/main.o
OBJ_SRCS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(shell find $(SRC_DIR) -name '*.c'))
OBJS := $(OBJ_MAIN) $(OBJ_SRCS)

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Regra para main.c na raiz
$(OBJ_DIR)/main.o: main.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para os arquivos dentro de src/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN) $(OBJ_DIR)

.PHONY: all clean
