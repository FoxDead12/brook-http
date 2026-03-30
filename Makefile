# 1. Identificar o Sistema Operativo
UNAME_S := $(shell uname -s)

# 2. Configurações base
TARGET = brook
CC = gcc
SRC_DIR = src
INC_DIR = includes
BUILD_DIR = build

# Flags base (comuns a ambos)
CFLAGS = -I$(INC_DIR) -Wall -Wextra -O2
LDFLAGS = -lcjson -lbeanstalkclient -lhiredis

# 3. Ajustes específicos por OS
ifeq ($(UNAME_S), Darwin)
    # macOS: Adiciona caminhos do Homebrew
    HOMEBREW_PREFIX = /opt/homebrew
    CFLAGS += -I$(HOMEBREW_PREFIX)/include
    LDFLAGS += -L$(HOMEBREW_PREFIX)/lib
else
    # Linux: Assume que as libs estão nos caminhos padrão do sistema
    # (Podes adicionar caminhos extras aqui se necessário)
    CFLAGS += -I/usr/local/include
    LDFLAGS += -L/usr/local/lib
endif

# 4. Localizar ficheiros (Recursivo)
# Find funciona em ambos os sistemas para pastas e subpastas
SRC = $(shell find $(SRC_DIR) -name "*.c")
OBJ = $(SRC:.c=.o)

# 5. Regras de Build
all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(OBJ) -o $(BUILD_DIR)/$(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 6. Instalação (Universal para Unix)
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

install: $(TARGET)
	@echo "A instalar $(TARGET) em $(BINDIR)..."
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(BUILD_DIR)/$(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	find $(SRC_DIR) -name "*.o" -delete

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

.PHONY: all clean install uninstall
