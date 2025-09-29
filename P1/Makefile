# Compilador y flags
CC     := gcc
CFLAGS := -Wall -Wextra -I./lib
LDFLAGS:= -lgmp

# Directorios
SRC_DIR := ./src
OBJ_DIR := ./obj
BIN_AFIN := ./afin
BIN_EUC  := ./euclides

# Fuentes
SRC_AFIN := $(SRC_DIR)/afin.c $(SRC_DIR)/euclides.c
SRC_EUC  := $(SRC_DIR)/euclides.c

# Objetos
OBJ_AFIN := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_AFIN))
OBJ_EUC  := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_EUC))

# Regla por defecto: compilar ambos ejecutables
all: $(BIN_AFIN) #$(BIN_EUC)

# Ejecutable afin
$(BIN_AFIN): $(OBJ_AFIN)
	$(CC) $(OBJ_AFIN) $(LDFLAGS) -o $@

# Ejecutable euclides
#$(BIN_EUC): $(OBJ_EUC)
#	$(CC) $(OBJ_EUC) $(LDFLAGS) -o $@

# Compilar cada .c en su .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar
clean:
	rm -rf $(OBJ_DIR) $(BIN_AFIN) $(BIN_EUC)

# Recompilar todo
rebuild: clean all
