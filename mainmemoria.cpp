#include "trie.cpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <functional> 
#include <cmath>      

// Función para cargar palabras desde un archivo .txt (una palabra por línea solo funciona con words.txt)
std::vector<std::string> load_words_from_file(const std::string& filename) {
    std::vector<std::string> words;
    std::ifstream file(filename);
    std::string word;
    
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << filename << std::endl;
        return words;
    }
    
    std::cout << "Cargando palabras desde " << filename << "..." << std::endl;
    
    int count = 0;
    while (std::getline(file, word)) {
        // Limpiar la palabra
        std::string clean_word;
        for (char c : word) {
            if (std::isalpha((unsigned char)c)) {
                clean_word.push_back(std::tolower((unsigned char)c));
            }
        }
        
        if (!clean_word.empty() && clean_word.length() > 1) {
            words.push_back(clean_word);
            count++;
            
            // Mostrar progreso cada 10000 palabras, cambiar el numero despues de % para reducir info en terminal
            if (count % 10000 == 0) {
                std::cout << "Cargadas " << count << " palabras..." << std::endl;
            }
        }
    }
    
    file.close();
    std::cout << "Total de palabras cargadas: " << count << std::endl;
    return words;
}

// info en pantalla
void show_usage() {
    std::cout << "Uso: ./autocomplete <dataset.txt> <modo>\n";
    std::cout << "  dataset.txt: archivo de texto con una palabra por línea\n";
    std::cout << "  modo: 'reciente' o 'frecuente'\n";
    std::cout << "Ejemplos:\n";
    std::cout << "  ./autocomplete palabras.txt frecuente\n";
    std::cout << "  ./autocomplete english_words.txt reciente\n";
}

// Función para buscar autocompletado dado un prefijo
void search_autocomplete(Trie& trie, const std::string& prefix) {
    if (prefix.empty()) {
        std::cout << "Error: Prefijo vacío" << std::endl;
        return;
    }
    
    // Descender por el trie según el prefijo
    Trie::Node* current = trie.root_;
    for (char c : prefix) {
        current = trie.descend(current, c);
        if (!current) {
            std::cout << "No se encontraron palabras con el prefijo '" << prefix << "'" << std::endl;
            return;
        }
    }
    
    // Obtener el mejor autocompletado
    Trie::Node* best = trie.autocomplete(current);
    if (best && best->str) {
        std::cout << "Autocompletado: '" << *best->str << "'";
        
        // Mostrar información adicional según el modo
        if (trie.variant == Trie::Variant::MOST_RECENT) {
            std::cout << " (timestamp: " << best->priority << ")";
        } else {
            std::cout << " (frecuencia: " << best->priority << ")";
        }
        std::cout << std::endl;
        
        // Actualizar la prioridad de la palabra encontrada
        trie.update_priority(best);
    } else {
        std::cout << "No se encontró autocompletado para '" << prefix << "'" << std::endl;
    }
}

// Función principal
void run_autocomplete(Trie& trie, const std::string& mode_name) {
    std::cout << "\n=== Motor de Autocompletado ===" << std::endl;
    std::cout << "Modo: " << mode_name << std::endl;
    std::cout << "Comandos:" << std::endl;
    std::cout << "  <prefijo>     - Buscar autocompletado para el prefijo" << std::endl;
    std::cout << "  !stats        - Mostrar estadísticas del trie" << std::endl;
    std::cout << "  !quit         - Salir del programa" << std::endl;
    std::cout << "================================\n" << std::endl;
    
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        // Convertir a minúsculas para consistencia
        for (size_t i = 0; i < input.size(); ++i) {
            input[i] = std::tolower(static_cast<unsigned char>(input[i]));
        }
        
        if (input == "!quit" || input == "exit") {
            std::cout << "¡Hasta luego!" << std::endl;
            break;
        }
        else if (input == "!stats") {
            trie.print_stats();
        }
       
        
        else if (!input.empty() && input[0] != '!') {
            // Búsqueda de autocompletado
            search_autocomplete(trie, input);
        }
        else {
            std::cout << "Comando no reconocido. Escribe !help para ver los comandos disponibles." << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        show_usage();
        return 1;
    }
    
    std::string filename = argv[1];
    std::string mode_str = argv[2];
    
    // Validar modo
    Trie::Variant variant;
    if (mode_str == "reciente") {
        variant = Trie::Variant::MOST_RECENT;
    } else if (mode_str == "frecuente") {
        variant = Trie::Variant::MOST_FREQUENT;
    } else {
        std::cerr << "Error: Modo debe ser 'reciente' o 'frecuente'" << std::endl;
        show_usage();
        return 1;
    }
    
    // Cargar palabras del dataset
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<std::string> words = load_words_from_file(filename);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    if (words.empty()) {
        std::cerr << "Error: No se pudieron cargar palabras del archivo" << std::endl;
        return 1;
    }
    
    auto load_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Tiempo de carga: " << load_duration.count() << " ms" << std::endl;
    
    // Crear el trie y insertar palabras con reporte de nodos cada 2^i palabras
    std::cout << "\nConstruyendo trie..." << std::endl;
    std::cout << "Reporte de nodos normalizado por caracteres:" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << std::setw(8) << "Palabras" << " | " 
              << std::setw(8) << "Caracteres" << " | " 
              << std::setw(8) << "Nodos" << " | " 
              << std::setw(10) << "Nodos/Car" << " | "
              << std::setw(10) << "Compresión" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    
    start_time = std::chrono::high_resolution_clock::now();
    
    Trie trie(variant);
    
    // Potencias de 2 hasta 262144
    std::vector<size_t> milestones;
    for (int i = 0; i <= 18; ++i) { // 2^18 = 262144
        size_t milestone = static_cast<size_t>(std::pow(2, i));
        if (milestone <= words.size()) {
            milestones.push_back(milestone);
        }
    }
    
    // Si hay menos de 262144 palabras
    if (words.size() < 262144) {
        milestones.push_back(words.size());
    }
    
    size_t next_milestone_idx = 0;
    
    // Insertar todas las palabras en el trie
    for (size_t i = 0; i < words.size(); ++i) {
        size_t chars_before = trie.total_chars();
        trie.insert(words[i]);
        size_t chars_after = trie.total_chars();
        size_t chars_in_word = chars_after - chars_before;
        
        // Verificar si alcanzamos un milestone de potencia de 2
        if (next_milestone_idx < milestones.size() && 
            (i + 1) == milestones[next_milestone_idx]) {
            size_t current_words = i + 1;
            size_t current_nodes = trie.node_count();
            size_t total_chars = trie.total_chars();
            
            double nodes_per_char = static_cast<double>(current_nodes) / total_chars;
            double compression_ratio = static_cast<double>(total_chars) / current_nodes;
            
            std::cout << std::setw(8) << current_words << " | " 
                      << std::setw(8) << total_chars << " | " 
                      << std::setw(8) << current_nodes << " | " 
                      << std::setw(10) << std::fixed << std::setprecision(4) << nodes_per_char << " | "
                      << std::setw(10) << std::fixed << std::setprecision(4) << compression_ratio 
                      << std::endl;
            next_milestone_idx++;
        }
        
        // Mostrar progreso adicional cada 10000 palabras
        if ((i + 1) % 10000 == 0 && next_milestone_idx >= milestones.size()) {
            std::cout << "Insertadas " << (i + 1) << " palabras..." << std::endl;
        }
    }
    
    end_time = std::chrono::high_resolution_clock::now();
    auto build_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Trie construido en " << build_duration.count() << " ms" << std::endl;
    
    // Estadísticas finales
    std::cout << "\n=== Estadísticas Finales ===" << std::endl;
    trie.print_stats();
    
    // Ejecutar la interfaz interactiva
    run_autocomplete(trie, mode_str);
    
    return 0;
}