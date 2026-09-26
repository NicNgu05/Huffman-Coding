#include "huffman.hpp"

    void huffman_tree::destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    huffman_tree::~huffman_tree() {
        destroy(root);
    }
    
    std::unordered_map<char, int> huffman_tree::frequency_data (const std::string& input) {
        std::unordered_map <char, int> frequencies;

        for (auto& element : input) {
            frequencies[element]++;
        }

        return frequencies;
    }

    void huffman_tree::build_huffman_tree (const std::unordered_map<char, int>& frequency_data) {

        // cleans the memory so when a new tree is created the old memory 
        // doesn't become unreachable 
        if (root) {  
            destroy(root); 
            root = nullptr; 
        }

        // just a guard for no inputs
        if (frequency_data.empty()) {
            return;
        }

        std::vector<std::pair<char, int>> sorted_freq(frequency_data.begin(), frequency_data.end());
        std::sort(sorted_freq.begin(), sorted_freq.end(), [](const auto& a, const auto& b) { return a.first < b.first; });


        std::priority_queue<Node*, std::vector<Node*>, compare_frequency> pq;

        for (auto& [ch, freq] : sorted_freq) {
            pq.push(new Node{ch, freq});
        }
        
        // loops until just the root remains
        while (pq.size() > 1) { 
            // removes the 2 lowest elements 
            Node* first = pq.top();
            pq.pop();

            Node* second = pq.top(); 
            pq.pop();

            // combines the elements together and add pointers so it can be reference later 
            Node* merged = new Node{'\0', first->frequency + second->frequency};
            merged->left = first;
            merged->right = second;

            pq.push(merged);
        }

        this->root = pq.top();
    }

    void huffman_tree::huffman_codes (Node* node, std::string code, std::unordered_map<char, std::string>& code_table) {
        if (!node) {
            return;
        }

        // checks whether there are no children nodes left
        if (!node->left && !node->right) {
            
            if (code.empty()) {
                code_table[node->symbol] = "0";
            }  else {
                code_table[node->symbol] = code;
            }
        }  

        // extends the code whenever it traverse down the tree
        huffman_codes(node->left, code + '0', code_table);
        huffman_codes(node->right, code + '1', code_table);
    }


    std::string huffman_tree::encode (const std::string& input) {
        auto freq = frequency_data(input);
        build_huffman_tree(freq);

        std::unordered_map<char, std::string> code_table;
        huffman_codes(root, "", code_table);

        std::string bits;
        for (auto& ch : input) {
            // assuming ch will always be in the input as the table was built off it
            auto key = code_table.find(ch);
            bits.append(key->second);
        }
        return bits;
    }


    std::string huffman_tree::decode (const std::string& bits) {
        std::string result;
        Node* current = root;

        for (auto& num : bits) {

            if (num == '0') {
                current = current->left;
            } else {
                current = current->right;
            }

            if (current == nullptr) {
                // When it goes off the tree, reset back to root as an invalid input happened
                current = root;
                result += current->symbol;
            } else if (!current->left && !current->right) {
                //once it hits the end of the tree node add the character to the result
                result += current->symbol;
                current = root;
            } 
        }
        return result;
    }

    std::vector<uint8_t> huffman_tree::pack (const std::string& bits, int& valid_bits) {
        std::vector<uint8_t> bytes;
        uint8_t current_byte = 0;
        int bit_count = 0;

        for (auto& ch : bits) {
            //pushes element far left by 1 and checks whether the element is 1 if not its 0
            current_byte = (current_byte << 1) | (ch == '1' ? 1 : 0);
            bit_count++;

            if (bit_count == 8) {
                bytes.push_back(current_byte);
                bit_count = 0;
                current_byte = 0;
            }
        }

        // used to assign padding to the last byte if needed 
        if (bit_count > 0) {
            current_byte <<= (8 - bit_count);   
            bytes.push_back(current_byte);
            valid_bits = bit_count;
        } else if (!bytes.empty()) {
            valid_bits = 8;        
        } else {
            valid_bits = 0;
        }

        return bytes;

    }

    std::string huffman_tree::unpack (const std::vector<uint8_t>& bytes, int valid_bits) {
        std::string bits;

        for (size_t i = 0; i < bytes.size(); i++) {
            // used to check whether the bit contains padding or not 
            int current_byte = (i == bytes.size() - 1 ? valid_bits : 8);

            // iterates over the bits based off the bytes valid size
            for (int b = 7; b >= 8 - current_byte; b--) {
                //add '1' or '0' from the rightmost side of the current_byte to the str bit
                bits += ((bytes[i] >> b) & 1) ? '1' : '0';
            }
        }

       return bits;
    }

    
    // File layout, in order:
    // [uint32_t]                number of distinct symbols, N
    // N x { [char] [uint32_t] } symbol, frequency pairs
    // [uint8_t]                 valid_bits (bits used in the last packed byte)
    // [uint32_t]                number of packed bytes, M
    // M x [uint8_t]             the packed bytes themselves
  
    void huffman_tree::write_frequency_table (std::ostream& out, const std::unordered_map<char, int>& freq) {
        uint32_t count = static_cast<uint32_t>(freq.size());
        // writes the total number of characters as a 4-byte number
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        
        //write the character and frequency of it in 5 bytes (1 byte: char, 4 bytes: frequency)
        for (const auto& [ch, frequency] : freq) {
            out.write(&ch, sizeof(char));
 
            uint32_t freq32 = static_cast<uint32_t>(frequency);
            out.write(reinterpret_cast<const char*>(&freq32), sizeof(freq32));
        }
    }
 

    std::unordered_map<char, int> huffman_tree::read_frequency_table (std::istream& in) {
        std::unordered_map<char, int> freq;
 
        uint32_t count = 0;
        // read the total number of characters as 4-byte numbers
        in.read(reinterpret_cast<char*>(&count), sizeof(count));
        
        // reads the character and frequency of it in 5 bytes (1 byte: char, 4 bytes: frequency)
        for (uint32_t i = 0; i < count; i++) {
            char ch;
            in.read(&ch, sizeof(char));
 
            uint32_t frequency = 0;
            in.read(reinterpret_cast<char*>(&frequency), sizeof(frequency));
 
            freq[ch] = static_cast<int>(frequency);
        }
 
        return freq;
    }
    
    // compresses frequency_table, encoded data and packed bits into the binary file
    void huffman_tree::compress_to_file (const std::string& input, const std::string& filename) {
        auto freq = frequency_data(input);
        std::string bits = encode(input);
 
        int valid_bits = 0;
        std::vector<uint8_t> packed = pack(bits, valid_bits);
 
        std::ofstream out(filename, std::ios::binary);
        if (!out) {
            throw std::runtime_error("could not open file for writing: " + filename);
        }
 
        write_frequency_table(out, freq);
 
        uint8_t valid_bits_u8 = static_cast<uint8_t>(valid_bits);
        out.write(reinterpret_cast<const char*>(&valid_bits_u8), sizeof(valid_bits_u8));
 
        uint32_t byte_count = static_cast<uint32_t>(packed.size());
        out.write(reinterpret_cast<const char*>(&byte_count), sizeof(byte_count));
 
        if (byte_count > 0) {
            out.write(reinterpret_cast<const char*>(packed.data()), packed.size());
        }
    }      
    
    // decompresses frequency_table, encoded data and packed bits from the binary file
    std::string huffman_tree::decompress_from_file (const std::string& filename) {
        std::ifstream in(filename, std::ios::binary);
        if (!in) {
            throw std::runtime_error("could not open file for reading: " + filename);
        }
 
        auto freq = read_frequency_table(in);
        build_huffman_tree(freq);
 
        uint8_t valid_bits_u8 = 0;
        in.read(reinterpret_cast<char*>(&valid_bits_u8), sizeof(valid_bits_u8));
 
        uint32_t byte_count = 0;
        in.read(reinterpret_cast<char*>(&byte_count), sizeof(byte_count));
 
        std::vector<uint8_t> packed(byte_count);
        if (byte_count > 0) {
            in.read(reinterpret_cast<char*>(packed.data()), byte_count);
        }
 
        std::string bits = unpack(packed, static_cast<int>(valid_bits_u8));
        return decode(bits);
    }
 





    
