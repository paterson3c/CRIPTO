# Compilador y flags
CC     := gcc
CFLAGS := -Wall -Wextra -I./lib
LDFLAGS:= -lgmp

# Directorios
SRC_DIR := ./src
OBJ_DIR := ./obj
BIN     := ./main

# Archivos
SRC := $(SRC_DIR)/euclides.c
OBJ := $(OBJ_DIR)/euclides.o

# Regla por defecto
all: $(BIN)

# Compilar ejecutable
$(BIN): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

# Compilar objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar objetos y binario
clean:
	rm -rf $(OBJ_DIR) $(BIN)

# Forzar recompilación
rebuild: clean all
