#include "trie.cpp"
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <iostream>

// Función para cargar palabras desde archivo
std::vector<std::string> load_words_from_file(const std::string& filename) {
    std::vector<std::string> words;
    std::ifstream file(filename);
    std::string word;
    
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir " << filename << std::endl;
        return words;
    }
    
    while (std::getline(file, word)) {
        if (!word.empty()) {
            words.push_back(word);
        }
    }
    file.close();
    return words;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Uso: ./tiempo <dataset.txt> <modo>\n";
        std::cout << "  modo: 'reciente' o 'frecuente'\n";
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
        return 1;
    }
    
    // Cargar palabras
    std::cout << "Cargando palabras desde " << filename << "..." << std::endl;
    auto words = load_words_from_file(filename);
    if (words.empty()) {
        std::cerr << "Error: No se pudieron cargar palabras" << std::endl;
        return 1;
    }
    
    std::cout << "Palabras cargadas: " << words.size() << std::endl;
    
    // Medir tiempo de construcción del trie
    auto start_build = std::chrono::high_resolution_clock::now();
    
    Trie trie(variant);
    for (size_t i = 0; i < words.size(); ++i) {
        trie.insert(words[i]);
        if ((i + 1) % 10000 == 0) {
            std::cout << "Insertadas " << (i + 1) << " palabras..." << std::endl;
        }
    }
    
    auto end_build = std::chrono::high_resolution_clock::now();
    auto build_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_build - start_build);
    
    // Medir tiempo de búsquedas
    auto start_search = std::chrono::high_resolution_clock::now();
    
    size_t total_chars = 0;
    for (const auto& word : words) {
        Trie::Node* current = trie.root_;
        for (char c : word) {
            current = trie.descend(current, c);
            if (!current) break;
        }
        total_chars += word.length();
    }
    
    auto end_search = std::chrono::high_resolution_clock::now();
    auto search_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_search - start_search);
    
    // Resultados
    std::cout << "\n=== RESULTADOS DE TIEMPO ===" << std::endl;
    std::cout << "Dataset: " << filename << std::endl;
    std::cout << "Modo: " << mode_str << std::endl;
    std::cout << "Palabras: " << words.size() << std::endl;
    std::cout << "Nodos en el trie: " << trie.node_count() << std::endl;
    std::cout << "Tiempo de construcción: " << build_time.count() << " ms" << std::endl;
    std::cout << "Tiempo de búsqueda: " << search_time.count() << " ms" << std::endl;
    std::cout << "Tiempo total: " << (build_time + search_time).count() << " ms" << std::endl;
    std::cout << "Tiempo promedio por palabra: " << std::fixed << std::setprecision(4) 
              << (build_time.count() / (double)words.size()) << " ms" << std::endl;
    std::cout << "Tiempo promedio por carácter: " << std::fixed << std::setprecision(4)
              << ((build_time + search_time).count() * 1000.0 / total_chars) << " μs" << std::endl;
    
    return 0;
}