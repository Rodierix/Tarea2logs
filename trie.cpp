#pragma once
#include <array>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <deque>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <functional> 


// Trie con funcionalidades de autocompletado
// Variante: modo MÁS RECIENTE o MÁS FRECUENTE

struct Trie {
    // --------------------------------------------------------
    // Tipos y estructuras
    // --------------------------------------------------------
    enum class Variant { MOST_RECENT, MOST_FREQUENT };

    struct Node {
        Node* parent = nullptr;
        std::array<Node*, 27> next;  // sigma = 26 letras + '$'
        int64_t priority = 0;          
        const std::string* str = nullptr; // puntero estable a string (solo terminal)
        Node* best_terminal = nullptr; // mejor terminal del subárbol
        int64_t best_priority = std::numeric_limits<int64_t>::min();

        Node() : next() { next.fill(nullptr); }
        bool is_terminal() const { return str != nullptr; }
    };

    // --------------------------------------------------------
    // Atributos del trie
    // --------------------------------------------------------
    Variant variant;
    Node* root_ = nullptr;
    int64_t access_counter_ = 0;       // para modo reciente
    size_t node_count_ = 0;            // cantidad de nodos
    size_t total_chars_ = 0;           // total de caracteres insertados
    std::deque<std::string> dict_;    
    size_t dict_bytes_ = 0;            // bytes de palabras almacenadas

    // --------------------------------------------------------
    // Constructor
    // --------------------------------------------------------
    Trie(Variant v) : variant(v) {
        root_ = new Node();
        node_count_ = 1;
        total_chars_ = 0;
        root_->best_priority = std::numeric_limits<int64_t>::min();
        root_->best_terminal = nullptr;
    }

    ~Trie() {
        // Función auxiliar para eliminar recursivamente
        std::function<void(Node*)> delete_node = [&](Node* node) {
            if (!node) return;
            for (int i = 0; i < 27; ++i) {
                if (node->next[i]) {
                    delete_node(node->next[i]);
                }
            }
            delete node;
        };
        delete_node(root_);
    }

    // --------------------------------------------------------
    // Funciones auxiliares
    // --------------------------------------------------------
    static int idx_of(char c) {
        if (c == '$') return 26;
        if ('a' <= c && c <= 'z') return c - 'a';
        return -1;
    }

    Node* ensure_child(Node* u, int idx) {
        if (!u->next[idx]) {
            u->next[idx] = new Node();
            u->next[idx]->parent = u;
            ++node_count_;
        }
        return u->next[idx];
    }

    // --------------------------------------------------------
    // Operaciones principales
    // --------------------------------------------------------

    // Inserta una palabra y retorna su nodo terminal
    Node* insert(const std::string& w_raw) {
        std::string w;
        w.reserve(w_raw.size());
        for (char c : w_raw)
            if (std::isalpha((unsigned char)c))
                w.push_back((char)std::tolower((unsigned char)c));

        // Si la palabra está vacía después de filtrar, no insertar
        if (w.empty()) return nullptr;

        Node* u = root_;
        for (char c : w) {
            int k = idx_of(c);
            if (k < 0) continue;
            u = ensure_child(u, k);
            total_chars_++; // Contar cada carácter insertado
        }

        // Crear o bajar al nodo '$'
        u = ensure_child(u, 26);
        total_chars_++; // Contar el carácter '$'

        // Si no era terminal, asociar string
        if (!u->is_terminal()) {
            dict_.push_back(w);            
            dict_bytes_ += w.size();
            u->str = &dict_.back();
            
            // Inicializar prioridad según variante
            switch (variant) {
                case Variant::MOST_RECENT:
                    u->priority = 0; // Se actualizará cuando se acceda
                    break;
                case Variant::MOST_FREQUENT:
                    u->priority = 0; // Empieza con frecuencia 0
                    break;
            }
            
            propagate_if_better(u);
        }
        return u;
    }

    // Descender un carácter desde nodo v
    Node* descend(Node* v, char c) const {
        if (!v) return nullptr;
        char cc = (c == '$') ? '$' : (char)std::tolower((unsigned char)c);
        int k = idx_of(cc);
        if (k < 0) return nullptr;
        return v->next[k];
    }

    // Retorna el mejor terminal en el subárbol
    Node* autocomplete(Node* v) const {
        if (!v) return nullptr;
        return v->best_terminal;
    }

    // Actualiza prioridad de un nodo terminal y propaga hacia la raíz
    void update_priority(Node* terminal) {
        assert(terminal && terminal->is_terminal());
        
        switch (variant) {
            case Variant::MOST_RECENT:
                terminal->priority = ++access_counter_;
                break;
            case Variant::MOST_FREQUENT:
                terminal->priority += 1;
                break;
        }

        // Actualizar el propio nodo terminal
        terminal->best_priority = terminal->priority;
        terminal->best_terminal = terminal;

        // Propagar hacia la raíz
        Node* cur = terminal->parent;
        while (cur) {
            bool needs_update = false;
            
            // Si el nodo actual no tiene best_terminal, asignar este
            if (cur->best_terminal == nullptr) {
                needs_update = true;
            }
            // Si la prioridad del terminal es mayor que la best_priority actual
            else if (terminal->priority > cur->best_priority) {
                needs_update = true;
            }
            // Si tienen la misma prioridad, en modo RECIENTE preferir el más reciente
            else if (terminal->priority == cur->best_priority && 
                     cur->best_terminal != terminal &&
                     variant == Variant::MOST_RECENT) {
                needs_update = true;
            }
            
            if (needs_update) {
                cur->best_priority = terminal->priority;
                cur->best_terminal = terminal;
                cur = cur->parent;
            } else {
                break;
            }
        }
    }

    // --------------------------------------------------------
    // Métricas 
    // --------------------------------------------------------
    size_t node_count() const { return node_count_; }
    size_t total_chars() const { return total_chars_; } 

    size_t approx_memory_bytes() const {
        return node_count_ * sizeof(Node) + dict_bytes_;
    }

    // --------------------------------------------------------
    // info en pantalla
    // --------------------------------------------------------
    void print_stats() const {
        std::cout << "=== Estadísticas del Trie ===" << std::endl;
        std::cout << "Nodos totales: " << node_count_ << std::endl;
        std::cout << "Palabras almacenadas: " << dict_.size() << std::endl;
        std::cout << "Caracteres totales insertados: " << total_chars_ << std::endl;
        std::cout << "Memoria aproximada: " << approx_memory_bytes() << " bytes" << std::endl;
        std::cout << "Memoria aproximada: " << approx_memory_bytes() / 1024.0 / 1024.0 << " MB" << std::endl;
        std::cout << "Modo: " << (variant == Variant::MOST_RECENT ? "MÁS RECIENTE" : "MÁS FRECUENTE") << std::endl;
        if (variant == Variant::MOST_RECENT) {
            std::cout << "Contador de accesos: " << access_counter_ << std::endl;
        }
        std::cout << "==============================" << std::endl;
    }

private:
    
    void propagate_if_better(Node* terminal) {
        
        // terminal
        terminal->best_priority = terminal->priority;
        terminal->best_terminal = terminal;

        
        Node* cur = terminal->parent;
        while (cur) {
            if (cur->best_terminal == nullptr || 
                terminal->priority > cur->best_priority) {
                cur->best_priority = terminal->priority;
                cur->best_terminal = terminal;
                cur = cur->parent;
            } else {
                break;
            }
        }
    }
};