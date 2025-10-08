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
BIN_VIGENERE  := $(BIN_DIR)/vigenere
BIN_CRIPTO_VIG := $(BIN_DIR)/criptoAnalisisVigenere

# Fuentes
SRC_AFIN      := $(SRC_DIR)/afin.c $(SRC_DIR)/euclides.c
SRC_AFIN_MOD  := $(SRC_DIR)/afin_modificado.c $(SRC_DIR)/euclides.c
SRC_EUC       := $(SRC_DIR)/euclides.c
SRC_VIGENERE  := $(SRC_DIR)/vigenere.c
SRC_CRIPTO_VIG := $(SRC_DIR)/criptoAnalisisVigenere.c

# Objetos
OBJ_AFIN      := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_AFIN))
OBJ_AFIN_MOD  := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_AFIN_MOD))
OBJ_EUC       := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_EUC))
OBJ_VIGENERE  := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_VIGENERE))
OBJ_CRIPTO_VIG := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_CRIPTO_VIG))

# ===============================

# ===============================
#   REGLAS PRINCIPALES
# ===============================

# Por defecto compila todo
all: $(BIN_AFIN) $(BIN_AFIN_MOD) #$(BIN_EUC) $(BIN_VIGENERE) $(BIN_CRIPTO_VIG)

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

# Ejecutable VIGENERE
$(BIN_VIGENERE): $(OBJ_VIGENERE)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJ_VIGENERE) $(LDFLAGS) -o $@
	@echo "[OK] Generado ejecutable $@"

# Ejecutable CRIPTOANÁLISIS VIGENERE
$(BIN_CRIPTO_VIG): $(OBJ_CRIPTO_VIG)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJ_CRIPTO_VIG) $(LDFLAGS) -o $@
	@echo "[OK] Generado ejecutable $@"


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
	$(BIN_AFIN_MOD) -C -m 26 -a 7 -b 13 -i $(FILES_DIR)/input.txt -o $(FILES_DIR)/output_mod.enc
	@echo "[DONE] Archivo cifrado en $(FILES_DIR)/output_mod.enc"

# Descifrar con afin modificado (bloques)
decrypt_mod:
	@mkdir -p $(FILES_DIR)
	$(BIN_AFIN_MOD) -D -m 26 -a 7 -b 13 -i $(FILES_DIR)/output_mod.enc -o $(FILES_DIR)/output_mod_dec.txt
	@echo "[DONE] Archivo descifrado en $(FILES_DIR)/output_mod_dec.txt"

# CRIFRADO VIGENERE
encrypt_vigenere:
	@mkdir -p $(FILES_DIR)
	$(BIN_VIGENERE) -C -k CLAVE -i $(FILES_DIR)/quijote_mitad.txt -o $(FILES_DIR)/output_vig.enc
	@echo "[DONE] Archivo cifrado en $(FILES_DIR)/output_vig.enc"

# DESCRIFRADO VIGENERE
decrypt_vigenere:
	@mkdir -p $(FILES_DIR)
	$(BIN_VIGENERE) -D -k CLAVE -i $(FILES_DIR)/output_vig.enc -o $(FILES_DIR)/output_vig_dec.txt
	@echo "[DONE] Archivo descifrado en $(FILES_DIR)/output_vig_dec.txt"

# CRIPTOANÁLISIS VIGENERE (test de Kasiski)
analisis_vigenere_kasiski:
	@mkdir -p $(FILES_DIR)
	$(BIN_CRIPTO_VIG) -kasiski -i $(FILES_DIR)/output_vig.enc
	@echo "[DONE] Análisis de texto cifrado completado"

# CRIPTOANÁLISIS VIGENERE (índice de coincidencia)
analisis_vigenere_ic:
	@mkdir -p $(FILES_DIR)
	$(BIN_CRIPTO_VIG) -ic 5 $(FILES_DIR)/output_vig.enc
	@echo "[DONE] Análisis de texto cifrado completado"

# ===============================
#   VALGRIND TESTS
# ===============================

VALGRIND := valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes

valgrind_encrypt_afin: $(BIN_AFIN)
	@echo "[VALGRIND] Comprobando fugas en AFIN..."
	$(VALGRIND) $(BIN_AFIN) -C -m 26 -a 5 -b 8 -i $(FILES_DIR)/quijote.txt -o $(FILES_DIR)/output.enc

valgrind_encrypt_afin_mod: $(BIN_AFIN_MOD)
	@echo "[VALGRIND] Comprobando fugas en AFIN MODIFICADO..."
	$(VALGRIND) $(BIN_AFIN_MOD) -C -m 26 -a 7 -b 13 -i $(FILES_DIR)/quijote.txt -o $(FILES_DIR)/output_mod.enc

valgrind_decrypt_afin:
	@echo "[VALGRIND] Comprobando fugas en AFIN..."
	$(VALGRIND) $(BIN_AFIN) -D -m 26 -a 5 -b 8 -i $(FILES_DIR)/output.enc -o $(FILES_DIR)/output_dec.txt
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