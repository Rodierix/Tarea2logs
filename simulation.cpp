#include "trie.cpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <functional>
#include <cmath>
#include <algorithm>
#include <map>

// Función para cargar palabras desde un archivo .txt
std::vector<std::string> load_words_from_file(const std::string& filename) {
    std::vector<std::string> words;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << filename << std::endl;
        return words;
    }
    
    std::cout << "Cargando palabras desde " << filename << "..." << std::endl;
    
    std::string line;
    int total_words = 0;
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string word;
        
        // Dividir la línea en palabras individuales
        while (ss >> word) {
            // convertir a minusculas y limpiar
            std::string clean_word;
            for (char c : word) {
                if (std::isalpha((unsigned char)c)) {
                    clean_word.push_back(std::tolower((unsigned char)c));
                }
            }
            
            if (!clean_word.empty() && clean_word.length() >= 1) {
                words.push_back(clean_word);
                total_words++;
                
                if (total_words % 100000 == 0) {
                    std::cout << "Cargadas " << total_words << " palabras..." << std::endl;
                }
                
                // Limite de palabras para pruebas
                // if (total_words >= 100000) { // 
                //     break;
                // }
            }
        }
        
        // Limite para pruebas
        // if (total_words >= 100000) break;
    }
    
    file.close();
    std::cout << "Total de palabras cargadas: " << total_words << std::endl;
    return words;
}

// Funcion para simular la escritura de una palabra usando autocompletado
struct SimulationResult {
    size_t chars_written;    // Caracteres que el usuario tuvo que escribir
    size_t chars_saved;      // Caracteres ahorrados
    bool success;           // Si el autocompletado fue exitoso
    double time_taken_ms;   // Tiempo que tomó procesar esta palabra
};

SimulationResult simulate_word_typing(Trie& trie, const std::string& word) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    Trie::Node* current = trie.root_;
    size_t chars_typed = 0;
    bool autocomplete_success = false;
    
    // Primero verificar si la palabra existe en el trie
    Trie::Node* temp = trie.root_;
    bool word_exists = true;
    for (char c : word) {
        temp = trie.descend(temp, c);
        if (!temp) {
            word_exists = false;
            break;
        }
    }
    
    if (word_exists) {
        temp = trie.descend(temp, '$');
        word_exists = (temp && temp->str && *temp->str == word);
    }
    
    if (!word_exists) {
        // Palabra no existe en el trie, usuario debe escribirla completa
        auto end_time = std::chrono::high_resolution_clock::now();
        double time_ms = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time).count() / 1000.0;
        
        return SimulationResult{word.length(), 0, false, time_ms};
    }
    
    // La palabra existe, simular escritura con autocompletado
    for (size_t i = 0; i < word.length(); ++i) {
        char c = word[i];
        Trie::Node* next_node = trie.descend(current, c);
        
        if (!next_node) {
            // Esto no debería pasar si word_exists es true
            chars_typed = word.length();
            break;
        }
        
        current = next_node;
        chars_typed++;
        
        // Verificar autocompletado en el nodo actual
        Trie::Node* autocomplete_node = trie.autocomplete(current);
        if (autocomplete_node && autocomplete_node->str) {
            std::string completed_word = *autocomplete_node->str;
            
            if (completed_word == word) {
                // Autocompletado exitoso
                autocomplete_success = true;
                break;
            }
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    double time_ms = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time).count() / 1000.0;
    
    // Actualizar prioridad de la palabra
    Trie::Node* word_node = trie.root_;
    for (char c : word) {
        word_node = trie.descend(word_node, c);
        if (!word_node) break;
    }
    if (word_node) {
        word_node = trie.descend(word_node, '$');
        if (word_node && word_node->str && *word_node->str == word) {
            trie.update_priority(word_node);
        }
    }
    
    return SimulationResult{
        chars_typed,
        word.length() - chars_typed,
        autocomplete_success,
        time_ms
    };
}

// Función para ejecutar la simulación completa
void run_simulation(Trie& trie, const std::vector<std::string>& words, 
                   const std::string& dataset_name, const std::string& variant_name) {
    std::cout << "\n=== Simulación: " << dataset_name << " (" << variant_name << ") ===" << std::endl;
    
    const size_t L = words.size();
    std::cout << "Palabras a simular: " << L << std::endl;
    
    std::vector<double> percentages;
    std::vector<double> simulation_times;
    std::vector<size_t> milestone_indices;
    
    // contaremos desde 2^0 hasta 2^21
    for (int i = 0; i <= 21; ++i) {
        size_t milestone = static_cast<size_t>(std::pow(2, i));
        if (milestone <= L && milestone > 0) {
            milestone_indices.push_back(milestone - 1); // -1 porque indices empiezan en 0
        }
    }
    if (!milestone_indices.empty() && milestone_indices.back() != L - 1) {
        milestone_indices.push_back(L - 1);
    }
    
    size_t total_chars_without_autocomplete = 0;
    size_t total_chars_with_autocomplete = 0;
    double total_simulation_time_ms = 0;
    
    size_t next_milestone_idx = 0;
    size_t successful_autocompletes = 0;
    size_t words_not_in_trie = 0;
    
    std::cout << "\nProgreso de la simulación:" << std::endl;
    std::cout << "--------------------------" << std::endl;
    
    for (size_t i = 0; i < L; ++i) {
        const std::string& word = words[i];
        total_chars_without_autocomplete += word.length();
        
        SimulationResult result = simulate_word_typing(trie, word);
        total_chars_with_autocomplete += result.chars_written;
        total_simulation_time_ms += result.time_taken_ms;
        
        if (result.success) {
            successful_autocompletes++;
        }
        
        if (result.chars_written == word.length() && !result.success) {
            words_not_in_trie++;
        }
        
        // Verificar si alcanzamos un milestone potencia de 2
        if (next_milestone_idx < milestone_indices.size() && 
            i == milestone_indices[next_milestone_idx]) {
            double percentage = (static_cast<double>(total_chars_with_autocomplete) / 
                               total_chars_without_autocomplete) * 100.0;
            percentages.push_back(percentage);
            simulation_times.push_back(total_simulation_time_ms);
            
            std::cout << "Palabra " << std::setw(8) << (i + 1) << ": " 
                      << std::fixed << std::setprecision(2) << percentage 
                      << "% de caracteres escritos" << std::endl;
            
            next_milestone_idx++;
        }
        
        if ((i + 1) % 100000 == 0) {
            std::cout << "Procesadas " << (i + 1) << " palabras..." << std::endl;
        }
    }
    
    // info en pantalla
    std::cout << "\n=== Resultados Finales ===" << std::endl;
    std::cout << "Dataset: " << dataset_name << std::endl;
    std::cout << "Variante: " << variant_name << std::endl;
    std::cout << "Total de palabras procesadas: " << L << std::endl;
    std::cout << "Caracteres totales sin autocompletado: " << total_chars_without_autocomplete << std::endl;
    std::cout << "Caracteres totales con autocompletado: " << total_chars_with_autocomplete << std::endl;
    std::cout << "Ahorro total de caracteres: " << (total_chars_without_autocomplete - total_chars_with_autocomplete) << std::endl;
    std::cout << "Porcentaje de caracteres escritos: " << std::fixed << std::setprecision(2) 
              << (static_cast<double>(total_chars_with_autocomplete) / total_chars_without_autocomplete * 100.0) << "%" << std::endl;
    std::cout << "Autocompletados exitosos: " << successful_autocompletes << "/" << L 
              << " (" << std::fixed << std::setprecision(2) 
              << (static_cast<double>(successful_autocompletes) / L * 100.0) << "%)" << std::endl;
    std::cout << "Palabras no encontradas en el trie: " << words_not_in_trie << "/" << L 
              << " (" << std::fixed << std::setprecision(2) 
              << (static_cast<double>(words_not_in_trie) / L * 100.0) << "%)" << std::endl;
    std::cout << "Tiempo total de simulación: " << std::fixed << std::setprecision(2) 
              << total_simulation_time_ms << " ms" << std::endl;
    std::cout << "Tiempo promedio por palabra: " << std::fixed << std::setprecision(4) 
              << (total_simulation_time_ms / L) << " ms" << std::endl;
    std::cout << "Tiempo promedio por carácter: " << std::fixed << std::setprecision(4) 
              << (total_simulation_time_ms * 1000 / total_chars_without_autocomplete) << " μs" << std::endl;
    
    // Export a csv
    std::string output_filename = "resultdos/results_" + dataset_name + "_" + variant_name + ".csv";
    std::ofstream output_file(output_filename);
    if (output_file.is_open()) {
        output_file << "palabras,porcentaje_caracteres,tiempo_acumulado_ms\n";
        for (size_t i = 0; i < percentages.size(); ++i) {
            output_file << (milestone_indices[i] + 1) << "," 
                       << std::fixed << std::setprecision(4) << percentages[i] << ","
                       << std::fixed << std::setprecision(2) << simulation_times[i] << "\n";
        }
        output_file.close();
        std::cout << "Datos exportados a: " << output_filename << std::endl;
    }
}

// Función principal
int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Uso: ./simulation <dataset.txt> <modo> <nombre_dataset>\n";
        std::cout << "  dataset.txt: archivo con texto para extraer palabras\n";
        std::cout << "  modo: 'reciente' o 'frecuente'\n";
        std::cout << "  nombre_dataset: nombre para identificar el dataset\n";
        std::cout << "Ejemplos:\n";
        std::cout << "  ./simulation wikipedia.txt reciente wikipedia\n";
        std::cout << "  ./simulation random.txt frecuente random\n";
        return 1;
    }
    
    std::string filename = argv[1];
    std::string mode_str = argv[2];
    std::string dataset_name = argv[3];
    
    // Validar modo
    Trie::Variant variant;
    if (mode_str == "reciente") {
        variant = Trie::Variant::MOST_RECENT;
    } else if (mode_str == "frecuente") {
        variant = Trie::Variant::MOST_FREQUENT;
    } else {
        std::cerr << "Error: Modo debe ser 'reciente' o 'frecuente'" << std::endl;
        return 1;
    }
    
    // Cargar palabras para simulación
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<std::string> simulation_words = load_words_from_file(filename);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    if (simulation_words.empty()) {
        std::cerr << "Error: No se pudieron cargar palabras del archivo" << std::endl;
        return 1;
    }
    
    auto load_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Tiempo de carga: " << load_duration.count() << " ms" << std::endl;
    
    // Primero construir el trie con todas las palabras
    std::cout << "\nConstruyendo trie con todas las palabras..." << std::endl;
    Trie trie(variant);
    
    auto build_start_time = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < simulation_words.size(); ++i) {
        trie.insert(simulation_words[i]);
        if ((i + 1) % 100000 == 0) {
            std::cout << "Insertadas " << (i + 1) << " palabras..." << std::endl;
        }
    }
    auto build_end_time = std::chrono::high_resolution_clock::now();
    auto build_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        build_end_time - build_start_time);
    
    std::cout << "Trie construido en " << build_duration.count() << " ms" << std::endl;
    trie.print_stats();
    
    // Ejecutar simulación
    run_simulation(trie, simulation_words, dataset_name, mode_str);
    
    return 0;
}