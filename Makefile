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

# Archivos de gráficos
GRAFICOS = graficos_*.png
GRAFICOS_INDIVIDUALES = graficos_individuales_*.png
SCRIPTS_GRAFICOS = graficar.py graficar_simple.py graficar_metricas.py

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

# Gráficos de métricas
install-python-deps:
	pip install pandas matplotlib seaborn numpy



graficar: $(RESULTADOS) 
	python3 graficar.py

# Target completo: compilar + simular + gráficos
completo: all run-compare graficar

# Limpieza
clean:
	rm -f $(AUTOCOMPLETE) $(SIMULATION) $(COMPARE) $(TIEMPO) $(MEMORIA)

clean-resultados:
	rm -rf $(RESULTADOS)

clean-graficos:
	rm -f $(GRAFICOS) $(GRAFICOS_INDIVIDUALES)

clean-csv:
	rm -f resultados/*.csv

clean-all: clean clean-resultados clean-graficos clean-csv



.PHONY: all clean clean-resultados clean-graficos clean-csv clean-all help \
        run-autocomplete run-simulation run-compare run-tiempo run-memoria run-all \
        install-python-deps graficos graficos-simple graficos-metricas completo