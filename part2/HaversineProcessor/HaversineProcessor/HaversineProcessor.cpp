
#include "JSONParser.h"

#include <iostream>

#pragma warning(disable : 4996)
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [input_json_file]";
        return 1;
    }
    FILE* file = fopen(argv[1], "r");
    if (file == nullptr) {
        std::cout << "File error: " << stderr;
        return 1;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char* buffer = new char[size];
    if (buffer == nullptr) {
        std::cout << "Memory error";
        return 1;
    }

    int copy_result = fread(buffer, 1, size, file);
    if (copy_result + 1 != size) {
        std::cout << "Read error";
        return 1;
    }

    JSONParser parser;
    parser.Parse(buffer);

    fclose(file);
    delete[] buffer;
    return 0;
}
