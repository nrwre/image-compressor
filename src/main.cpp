#include <cstdlib>
#include <iostream>

#include "decode.h"
#include "encode.h"

namespace {

void print_usage() {
    std::cout << "usage:\n"
              << "  imgcomp compress <in.bmp> <out.icc> [quality 1-100]\n"
              << "  imgcomp decompress <in.icc> <out.bmp>\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 4) {
        print_usage();
        return 1;
    }

    std::string mode = argv[1];
    std::string in_path = argv[2];
    std::string out_path = argv[3];

    try {
        if (mode == "compress") {
            int quality = argc > 4 ? std::atoi(argv[4]) : 75;
            compress_bmp(in_path, out_path, quality);
        } else if (mode == "decompress") {
            decompress_file(in_path, out_path);
        } else {
            print_usage();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
