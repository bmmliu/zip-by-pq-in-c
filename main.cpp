#include <iostream>
#include "pqueue.h"
#include "bstream.h"
#include "huffman.h"
int main(int argc, char* argv[]) {

  if (argc < 3) {
    std::cerr << "Usage: " << argv[0]
              << " <zapfile> <outputfile>"
              << std::endl;
    exit(1);
  }

  //std::string filename = argv[1];
  std::ofstream ofs;
  std::ifstream ifs;
  //std::ifstream ifs(filename, std::ios::in | std::ios::binary);

  ifs.open(argv[1]);
  ofs.open(argv[2]);

  Huffman huf;
  huf.Decompress(ifs, ofs);

  ifs.close();
  ofs.close();

  std::cout << "Decompressed zap file " << argv[1]
            << " into output file " << argv[2]
            << std::endl;

  return 0;
}
