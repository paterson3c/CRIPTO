# Compilador y flags
CC     := gcc
CFLAGS := -Wall -Wextra -I./lib
LDFLAGS:= -lgmp

# Directorios
SRC_DIR := ./src
OBJ_DIR := ./obj
BIN_AFIN := ./afin
BIN_AFIN_MOD := ./afin_mod
BIN_EUC  := ./euclides

# Fuentes
SRC_AFIN     := $(SRC_DIR)/afin.c $(SRC_DIR)/euclides.c
SRC_AFIN_MOD := $(SRC_DIR)/afin_modificado.c $(SRC_DIR)/euclides.c
SRC_EUC      := $(SRC_DIR)/euclides.c

# Objetos
OBJ_AFIN     := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_AFIN))
OBJ_AFIN_MOD := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_AFIN_MOD))
OBJ_EUC      := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_EUC))

# Regla por defecto: compilar todos
all: $(BIN_AFIN) $(BIN_AFIN_MOD) #$(BIN_EUC)

# Ejecutable afin clásico
$(BIN_AFIN): $(OBJ_AFIN)
	$(CC) $(OBJ_AFIN) $(LDFLAGS) -o $@

# Ejecutable afin modificado (bloques + permutación)
$(BIN_AFIN_MOD): $(OBJ_AFIN_MOD)
	$(CC) $(OBJ_AFIN_MOD) $(LDFLAGS) -o $@

# Ejecutable euclides (opcional)
#$(BIN_EUC): $(OBJ_EUC)
#	$(CC) $(OBJ_EUC) $(LDFLAGS) -o $@

# Compilar cada .c en su .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar
clean:
	rm -rf $(OBJ_DIR) $(BIN_AFIN) $(BIN_AFIN_MOD) $(BIN_EUC) *enc *_dec.txt *.dec

# Recompilar todo
rebuild: clean all
