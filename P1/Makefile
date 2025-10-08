# ===============================
#   CONFIGURACIÓN GENERAL
# ===============================

CC       := gcc
CFLAGS   := -Wall -Wextra -I./lib
LDFLAGS  := -lgmp

# Directorios
SRC_DIR   := ./src
OBJ_DIR   := ./obj
BIN_DIR   := ./bin
FILES_DIR := ./files

# Ejecutables
BIN_AFIN      := $(BIN_DIR)/afin
BIN_AFIN_MOD  := $(BIN_DIR)/afin_mod
BIN_EUC       := $(BIN_DIR)/euclides

# Fuentes
SRC_AFIN      := $(SRC_DIR)/afin.c $(SRC_DIR)/euclides.c
SRC_AFIN_MOD  := $(SRC_DIR)/afin_modificado.c $(SRC_DIR)/euclides.c
SRC_EUC       := $(SRC_DIR)/euclides.c

# Objetos
OBJ_AFIN      := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_AFIN))
OBJ_AFIN_MOD  := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_AFIN_MOD))
OBJ_EUC       := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_EUC))

# ===============================
#   REGLAS PRINCIPALES
# ===============================

# Por defecto compila todo
all: $(BIN_AFIN) $(BIN_AFIN_MOD) #$(BIN_EUC)

# Ejecutable AFIN clásico
$(BIN_AFIN): $(OBJ_AFIN)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJ_AFIN) $(LDFLAGS) -o $@
	@echo "[OK] Generado ejecutable $@"

# Ejecutable AFIN modificado (bloques + permutación)
$(BIN_AFIN_MOD): $(OBJ_AFIN_MOD)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJ_AFIN_MOD) $(LDFLAGS) -o $@
	@echo "[OK] Generado ejecutable $@"

# Ejecutable EUCLIDES (opcional)
#$(BIN_EUC): $(OBJ_EUC)
#	@mkdir -p $(BIN_DIR)
#	$(CC) $(OBJ_EUC) $(LDFLAGS) -o $@
#	@echo "[OK] Generado ejecutable $@"

# ===============================
#   COMPILACIÓN INTERMEDIA
# ===============================

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "[OK] Compilado $<"


# ===============================
#   LIMPIEZA
# ===============================

clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(BIN_DIR)/*
	rm -rf $(FILES_DIR)/*.enc $(FILES_DIR)/*_dec.txt $(FILES_DIR)/*.dec
	@echo "[CLEAN] Archivos intermedios y salidas eliminados"

rebuild: clean all

# ===============================
#   REGLAS DE USO RÁPIDO
# ===============================

# Cifrar texto plano con afin clásico
encrypt_afin:
	@mkdir -p $(FILES_DIR)
	$(BIN_AFIN) -C -m 26 -a 5 -b 8 -i $(FILES_DIR)/quijote.txt -o $(FILES_DIR)/output.enc
	@echo "[DONE] Archivo cifrado en $(FILES_DIR)/output.enc"

# Descifrar con afin clásico
decrypt_afin:
	@mkdir -p $(FILES_DIR)
	$(BIN_AFIN) -D -m 26 -a 5 -b 8 -i $(FILES_DIR)/output.enc -o $(FILES_DIR)/output_dec.txt
	@echo "[DONE] Archivo descifrado en $(FILES_DIR)/output_dec.txt"

# Cifrar con afin modificado (bloques)
encrypt_afin_mod:
	@mkdir -p $(FILES_DIR)
	$(BIN_AFIN_MOD) -C -m 26 -a 36986419 7 -b 2776385085840833906571070249467114581 -i $(FILES_DIR)/quijote.txt -o $(FILES_DIR)/output_mod.enc
	@echo "[DONE] Archivo cifrado en $(FILES_DIR)/output_mod.enc"

# Descifrar con afin modificado (bloques)
decrypt_afin_mod:
	@mkdir -p $(FILES_DIR)
	$(BIN_AFIN_MOD) -D -m 26 -a 36986419 7 -b 2776385085840833906571070249467114581 -i $(FILES_DIR)/output_mod.enc -o $(FILES_DIR)/output_mod_dec.txt
	@echo "[DONE] Archivo descifrado en $(FILES_DIR)/output_mod_dec.txt"

# ===============================
#   VALGRIND TESTS
# ===============================

VALGRIND := valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes

valgrind_encrypt_afin: $(BIN_AFIN)
	@echo "[VALGRIND] Comprobando fugas en AFIN..."
	$(VALGRIND) $(BIN_AFIN) -C -m 26 -a 5 -b 8 -i $(FILES_DIR)/quijote.txt -o $(FILES_DIR)/output.enc

valgrind_encrypt_afin_mod: $(BIN_AFIN_MOD)
	@echo "[VALGRIND] Comprobando fugas en AFIN MODIFICADO..."
	$(VALGRIND) $(BIN_AFIN_MOD) -C -m 26 -a 36986419 7 -b 2776385085840833906571070249467114581 -i $(FILES_DIR)/quijote.txt -o $(FILES_DIR)/output_mod.enc

valgrind_decrypt_afin:
	@echo "[VALGRIND] Comprobando fugas en AFIN..."
	$(VALGRIND) $(BIN_AFIN) -D -m 26 -a 5 -b 8 -i $(FILES_DIR)/output.enc -o $(FILES_DIR)/output_dec.txt
	@echo "[DONE] Archivo descifrado en $(FILES_DIR)/output_dec.txt"

valgrind_decrypt_afin_mod:
	@echo "[VALGRIND] Comprobando fugas en AFIN..."
	$(VALGRIND) $(BIN_AFIN_MOD) -D -m 26 -a 36986419 7 -b 2776385085840833906571070249467114581 -i $(FILES_DIR)/output.enc -o $(FILES_DIR)/output_dec.txt
	@echo "[DONE] Archivo descifrado en $(FILES_DIR)/output_dec.txt"

# valgrind_euclides: $(BIN_EUC)
#	@echo "[VALGRIND] Comprobando fugas en EUCLIDES..."
#	$(VALGRIND) $(BIN_EUC)
#
# ===============================
#   TEST COMPLETO AUTOMÁTICO
# ===============================

test: rebuild
	@mkdir -p $(FILES_DIR)
	@echo "[TEST] Cifrando..."
	$(BIN_AFIN) -C -m 26 -a 5 -b 8 -i $(FILES_DIR)/input.txt -o $(FILES_DIR)/output.enc
	@echo "[TEST] Descifrando..."
	$(BIN_AFIN) -D -m 26 -a 5 -b 8 -i $(FILES_DIR)/output.enc -o $(FILES_DIR)/output_dec.txt
	@echo "[TEST] Comprobando integridad..."
	@diff -q $(FILES_DIR)/input.txt $(FILES_DIR)/output_dec.txt && echo "Archivos idénticos" || echo "Diferencias detectadas"