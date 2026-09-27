#include "huffman.hpp"

static std::string read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("could not open file for reading: " + path);
    }
 
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}
 
static void write_file(const std::string& path, const std::string& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("could not open file for writing: " + path);
    }
    out.write(data.data(), static_cast<std::streamsize>(data.size()));
}
 
static void print_usage(const char* program_name) {
    std::cerr << "Usage:\n"
              << "  " << program_name << " compress <input file> <output file>\n"
              << "  " << program_name << " decompress <input file> <output file>\n"
              << "  " << program_name << " compare <input file>\n";
}
 
static int run_compress(const std::string& input_path, const std::string& output_path) {
    std::string content = read_file(input_path);
 
    huffman_tree tree;
    tree.compress_to_file(content, output_path);
 
    uintmax_t original_size = std::filesystem::file_size(input_path);
    uintmax_t compressed_size = std::filesystem::file_size(output_path);
 
    std::cout << "Compressed " << input_path << " -> " << output_path << "\n"
              << "  original:   " << original_size << " bytes\n"
              << "  compressed: " << compressed_size << " bytes\n";
 
    if (original_size > 0) {
        double ratio = 100.0 * (1.0 - static_cast<double>(compressed_size) / static_cast<double>(original_size));
        std::cout << "  reduction:  " << ratio << "%\n";
    }
 
    return 0;
}
 
static int run_decompress(const std::string& input_path, const std::string& output_path) {
    huffman_tree tree;
    std::string decoded = tree.decompress_from_file(input_path);
    write_file(output_path, decoded);
 
    std::cout << "Decompressed " << input_path << " -> " << output_path << "\n";
    return 0;
}
 
static int run_compare(const std::string& input_path) {
    std::string content = read_file(input_path);
 
    std::string huffman_path = input_path + ".huff";
    huffman_tree tree;
    tree.compress_to_file(content, huffman_path);
 
    std::string gzip_command = "gzip -k -f \"" + input_path + "\"";
    int gzip_result = std::system(gzip_command.c_str());
    if (gzip_result != 0) {
        std::cerr << "warning: gzip command failed (is gzip installed and on PATH?)\n";
    }
 
    uintmax_t original_size = std::filesystem::file_size(input_path);
    uintmax_t huffman_size = std::filesystem::file_size(huffman_path);
 
    std::cout << "Input:        " << input_path << " (" << original_size << " bytes)\n"
              << "Huffman:      " << huffman_path << " (" << huffman_size << " bytes)\n";
 
    std::filesystem::path gzip_path = input_path + ".gz";
    if (std::filesystem::exists(gzip_path)) {
        uintmax_t gzip_size = std::filesystem::file_size(gzip_path);
        std::cout << "gzip:         " << gzip_path.string() << " (" << gzip_size << " bytes)\n";
    }
 
    return 0;
}

int main(int argc, char* argv[]) {
   
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
 
    std::string command = argv[1];
 
    try {
        if (command == "compress" && argc == 4) {
            return run_compress(argv[2], argv[3]);
        } else if (command == "decompress" && argc == 4) {
            return run_decompress(argv[2], argv[3]);
        } else if (command == "compare" && argc == 3) {
            return run_compare(argv[2]);
        } else {
            print_usage(argv[0]);
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
        
}

// used to test if Huffman Algorithm worked properly
    
/*
void print_code_table(const std::unordered_map<char, std::string>& codes) {
    for (auto& [ch, code] : codes) {
        if (!code.empty()) {
            std::cout << "  [" << ch << "]: " << code << "\n";
        } else {
            std::cout << "  Input is empty\n";
        }
        
    }
}

// used to test huffman coding algorithm
void test_huffman_code (const std::string& input) {
    huffman_tree huff;

    std::cout << "\nInput: " << input << "(" << input.size() << ")\n";

    std::cout << "\n*** Frequencies ***\n";
    auto freqs = huff.frequency_data(input);
    for (auto& [ch, count] : freqs) {
        std::cout << "  [" << ch << "]: " << count << "\n";
    }


    std::cout << "\n*** Tree Build ***\n";
    huff.build_huffman_tree(freqs);

    if (!freqs.empty()) {
        std::cout << "  root frequency: " << huff.root->frequency << "\n";
    } else {
        std::cout << "  Input is empty \n";
    }
    

    std::cout << "\n*** Codes ***\n";
    std::unordered_map<char, std::string> code_table;
    huff.huffman_codes(huff.root, "", code_table);
    print_code_table(code_table);


    std::cout << "\n*** Encodes ***\n";
    std::string encoded = huff.encode(input);
    std::string excepted;
    for (auto& ch : input) {
        excepted += code_table.at(ch);
    }
    assert(encoded == excepted);
    std::cout << "  encoded: " << encoded << " (" << encoded.size() << " bits)\n";


    std::cout << "\n*** Pack Encoded Output ***\n";
    int valid_bits = 0;
    auto packed_output = huff.pack(encoded, valid_bits);
    std::cout << "  Packed: " << packed_output.size() << " (" << valid_bits << " last byte total)\n";

    std::cout << "\n*** Unpack Encoded Output ***\n";
    auto unpacked_output = huff.unpack(packed_output, valid_bits);
    std::cout << "  Unpacked: " <<  unpacked_output << " (" << unpacked_output.size() << " bits)\n";

    std::cout << "\n*** Decodes ***\n";
    std::string decoded = huff.decode(encoded);
    std::cout << "  decoded: " << decoded << "(" << decoded.size() << ")\n";
    assert(decoded == input);
}
    
    int main() {
        huffman_tree huff;
        int num = 1;
        std::vector<std::string> cases = {
            "awadsdawdacsax",
            "aaaab",
            "aaaa",
            "a",
            "abababab",
            "abcdefg",
            "mississippimississippi",
            "hello, world!",
            "adwnoadnoaodin",
            " ",
            "12039djdcj=>/a!mcsd",
            "aaaaaaaaaaaaaaaaaabbb",
            "",
            "ggffeeddccbbaa"
        };

        for (auto& input : cases) {
            std::cout << "\n******************* TEST CASE " << num << " *******************\n";
            test_huffman_code(input);
            num++;
        }

        return 0;
    }
    */