CC       = gcc
CFLAGS   = -std=c99 -fstack-protector-all -Wall -Werror=implicit-function-declaration -g
LDFLAGS  = -lm

PROJECT  = ted
SRC      = src
TST      = tst
UNITY    = tst/unity

# Objetos do programa principal
OBJS = $(SRC)/main.o      \
       $(SRC)/libs.o      \
       $(SRC)/list.o      \
       $(SRC)/quadra.o    \
       $(SRC)/habitante.o \
       $(SRC)/hashfile.o  \
       $(SRC)/paths.o     \
       $(SRC)/system.o

# ─── Executavel principal ───────────────────────────────────────────────────

$(PROJECT): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(SRC)/$(PROJECT) $(LDFLAGS)

$(SRC)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ─── Testes unitarios ────────────────────────────────────────────────────────

UNITY_OBJ = $(UNITY)/unity.o

$(UNITY)/unity.o: $(UNITY)/unity.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cada modulo tem seu proprio binario de teste
t_list: $(UNITY_OBJ) $(SRC)/list.o $(TST)/t_list.o
	$(CC) $(CFLAGS) $^ -o $(TST)/t_list $(LDFLAGS)

t_paths: $(UNITY_OBJ) $(SRC)/paths.o $(TST)/t_paths.o
	$(CC) $(CFLAGS) $^ -o $(TST)/t_paths $(LDFLAGS)

t_quadra: $(UNITY_OBJ) $(SRC)/quadra.o $(TST)/t_quadra.o
	$(CC) $(CFLAGS) $^ -o $(TST)/t_quadra $(LDFLAGS)

t_habitante: $(UNITY_OBJ) $(SRC)/habitante.o $(TST)/t_habitante.o
	$(CC) $(CFLAGS) $^ -o $(TST)/t_habitante $(LDFLAGS)

t_hashfile: $(UNITY_OBJ) $(SRC)/hashfile.o $(TST)/t_hashfile.o
	$(CC) $(CFLAGS) $^ -o $(TST)/t_hashfile $(LDFLAGS)

t_hash_extensivel: $(UNITY_OBJ) $(SRC)/hash_extensivel.o $(TST)/t_hash_extensivel.o
	$(CC) $(CFLAGS) $^ -o $(TST)/t_hash_extensivel $(LDFLAGS)

t_system: $(UNITY_OBJ) $(SRC)/system.o $(SRC)/paths.o $(SRC)/quadra.o \
          $(SRC)/habitante.o $(SRC)/hashfile.o $(SRC)/list.o $(SRC)/libs.o \
          $(TST)/t_system.o
	$(CC) $(CFLAGS) $^ -o $(TST)/t_system $(LDFLAGS)

$(TST)/%.o: $(TST)/%.c
	$(CC) $(CFLAGS) -I$(SRC) -I$(UNITY) -c $< -o $@

# tstall: compila e roda todos os testes unitarios
tstall: t_list t_paths t_quadra t_habitante t_hashfile t_hash_extensivel t_system
	./$(TST)/t_list
	./$(TST)/t_paths
	./$(TST)/t_quadra
	./$(TST)/t_habitante
	./$(TST)/t_hashfile
	./$(TST)/t_hash_extensivel
	./$(TST)/t_system

# ─── Valgrind ────────────────────────────────────────────────────────────────

VALGRIND = valgrind --leak-check=full --show-leak-kinds=all \
           --track-origins=yes --error-exitcode=1 -q

BED  = ../testes-t1
BSD  = ../saida_teste
PASS = 0
FAIL = 0

valgrind: $(PROJECT)
	@mkdir -p $(BSD)
	@pass=0; fail=0; \
	for cidade in c1 c2 c3; do \
	  for qry in $(BED)/$$cidade/*.qry; do \
	    nome=$$(basename $$qry .qry); \
	    if $(VALGRIND) ./$(SRC)/$(PROJECT) \
	         -e $(BED)/ -f $$cidade.geo -o $(BSD)/ \
	         -pm $$cidade.pm -q $$cidade/$$nome.qry 2>&1 | grep -q "ERROR\|Invalid\|uninitialised\|definitely lost\|indirectly lost"; then \
	      echo "LEAK/ERR: $$cidade-$$nome"; fail=$$((fail+1)); \
	    else \
	      pass=$$((pass+1)); \
	    fi; \
	  done; \
	done; \
	echo ""; \
	echo "Valgrind: $$pass ok, $$fail com erros"

# ─── Limpeza ─────────────────────────────────────────────────────────────────

clean:
	rm -f $(SRC)/*.o $(TST)/*.o $(UNITY)/*.o
	rm -f $(SRC)/$(PROJECT)
	rm -f $(TST)/t_hashfile $(TST)/t_hash_extensivel $(TST)/t_list $(TST)/t_paths $(TST)/t_quadra $(TST)/t_habitante $(TST)/t_system
