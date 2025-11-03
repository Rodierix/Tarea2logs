# Compilador y flags
CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall

# Nombres de ejecutables
AUTOCOMPLETE = autocomplete
SIMULATION = simulation
COMPARE = compare
TIEMPO = tiempo
MEMORIA = memoria

# Carpetas
TEXTOS = textos
RESULTADOS = resultados

# Target principal
all: $(AUTOCOMPLETE) $(SIMULATION) $(COMPARE) $(TIEMPO) $(MEMORIA)

# Reglas de compilación
$(AUTOCOMPLETE): main.cpp trie.cpp | $(RESULTADOS)
	$(CXX) $(CXXFLAGS) -o $@ main.cpp

$(SIMULATION): simulation.cpp trie.cpp | $(RESULTADOS)
	$(CXX) $(CXXFLAGS) -o $@ simulation.cpp

$(COMPARE): compare_simulations.cpp trie.cpp | $(RESULTADOS)
	$(CXX) $(CXXFLAGS) -o $@ compare_simulations.cpp

$(TIEMPO): maintiempo.cpp trie.cpp | $(RESULTADOS)
	$(CXX) $(CXXFLAGS) -o $@ maintiempo.cpp

$(MEMORIA): mainmemoria.cpp trie.cpp | $(RESULTADOS)
	$(CXX) $(CXXFLAGS) -o $@ mainmemoria.cpp

# Crear carpetas 
$(RESULTADOS):
	mkdir -p $(RESULTADOS)

# Comandos de ejecución
run-autocomplete: $(AUTOCOMPLETE)
	./$(AUTOCOMPLETE) $(TEXTOS)/words.txt frecuente

run-simulation: $(SIMULATION)
	./$(SIMULATION) $(TEXTOS)/wikipedia.txt reciente wikipedia

run-compare: $(COMPARE)
	./$(COMPARE)

run-tiempo: $(TIEMPO)
	./$(TIEMPO) $(TEXTOS)/words.txt frecuente

run-memoria: $(MEMORIA)
	./$(MEMORIA) $(TEXTOS)/words.txt frecuente

# Ejecutar todo
run-all: run-autocomplete run-simulation run-compare run-tiempo run-memoria

# Limpieza
clean:
	rm -f $(AUTOCOMPLETE) $(SIMULATION) $(COMPARE) $(TIEMPO) $(MEMORIA)

clean-resultados:
	rm -rf $(RESULTADOS)

clean-all: clean clean-resultados


