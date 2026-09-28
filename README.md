# Huffman-Coding

A Huffman coding implementation in C++ with a command-line interface, built to compress files and compare the result against gzip.

Build:
    make

This produces an executable called 'huffman' in the project directory. The Makefile compiles with (CFLAGS were jusst taken from a quiz):
    g++ -std=c++23 -Wall -Wextra -fsanitize=address,undefined -g -o huffman main.cpp huffman.cpp

- std=c++23:  required language standard.
- Wall -Wextra: enables extra compiler warnings.
- fsanitize=address,undefined: builds with AddressSanitizer and UndefinedBehaviorSanitizer, which catches memory errors and undefined behaviour at runtime. This makes the binary slower and larger than a normal release build -> intended for development/debugging, not for measuring real compression performance.
- g: includes debug symbols.

Other Makefile targets:
    make test    # builds and runs ./huffman with no arguments 
    make clean   # removes the built executable

Usage
    ./huffman compress <input file> <output file>
    ./huffman decompress <input file> <output file>
    ./huffman compare <input file>

Examples:
    ./huffman compress test.txt test.huff
    ./huffman decompress test.huff output.txt
    ./huffman compare test.txt

compress/decompress round-trip a file through the compressed .huff format described below. compare compresses the file with this implementation, then runs the system gzip on the same file and reports both file sizes side by side.

NOTE: compare uses terminal command "gzip -k -f ..." to keep the file and overwrite any leftover .gz files from previous runs.

Dependencies
A C++23-capable compiler (tested with g++).
gzip must be installed and available on PATH for the compare subcommand. If it isn't, compare will still run and compress with this program, but will print a warning and skip the gzip comparison.
File format (.huff)

Compressed files store the character frequency table rather than the tree itself - the tree is rebuilt deterministically from the frequencies on decompression. Layout, in order:

- Number of distinct symbols: uint32_t (n)
- n × (symbol, frequency) pairs: n × (char + uint32_t)
- Valid bits in the last packed byte: uint8_t
- Number of packed bytes: uint32_t (m)
- Packed data: m × uint8_t

Known limitations
Per-file overhead is 9 + 5n bytes, where n is the number of distinct characters in the input. This means that small or highly varied inputs can end up larger after compression than the original.

Only exploits single-character frequency, not repeated substrings which means that compression ratio will generally be worse than gzip on files with repeated multi-character patterns.
