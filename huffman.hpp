#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP


#include <unordered_map>
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <queue>
#include <fstream>    
#include <stdexcept> 
#include <iostream>
#include <cassert>
#include <utility>
#include <sstream>
#include <cstdlib>
#include <filesystem>



struct Node {
    char symbol;
    int frequency;
    Node* left = nullptr;
    Node* right = nullptr;
};

struct huffman_tree {
        ~huffman_tree();
        void destroy(Node* node);

        //root
        Node* root = nullptr; 
        
        //used to create the data for the frequency table 
        std::unordered_map<char, int> frequency_data (const std::string& input);

        //used to build the nodes using the freqency table as a parameter 
        void build_huffman_tree (const std::unordered_map<char, int>& frequency_data);

        //used to store the code for each char  
        void huffman_codes (Node* node, std::string code, std::unordered_map<char, std::string>& code_table);

        std::string encode(const std::string&);
      
        // pack into uin8_t vectors to compress the size
        std::vector<uint8_t> pack (const std::string& bits, int& valid_bits);

        // unpack the the vectors so that it can be decoded
        std::string unpack (const std::vector<uint8_t>& bytes, int valid_bits);

        std::string decode(const std::string& bits);

        void write_frequency_table(std::ostream& out, const std::unordered_map<char, int>& freq);

        std::unordered_map<char, int> read_frequency_table(std::istream& in);

        void compress_to_file(const std::string& input, const std::string& filename);

        std::string decompress_from_file(const std::string& filename);

       
};

struct compare_frequency {
    bool operator() (Node* a, Node* b) {
        return a->frequency > b->frequency;
    }
};


#endif