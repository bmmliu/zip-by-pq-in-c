#ifndef BSTREAM_H_
#define BSTREAM_H_

#include <cstddef>
#include <fstream>
#include <iostream>
#include <cmath>

class BinaryInputStream {
public:
  explicit BinaryInputStream(std::ifstream &ifs);

  bool GetBit();
  char GetChar();
  int GetInt();

private:
  std::ifstream &ifs;
  char buffer = 0;
  size_t avail = 0;

  // Helpers
  void RefillBuffer();
};

BinaryInputStream::BinaryInputStream(std::ifstream &ifs) : ifs(ifs) { }

void BinaryInputStream::RefillBuffer() {
  // Read the next byte from the input stream
  ifs.get(buffer);
  if (ifs.gcount() != 1)
    throw std::underflow_error("No more characters to read");
  avail = 8;
}
bool BinaryInputStream::GetBit() {
  bool bit;

  if (!avail)
    RefillBuffer();

  avail--;
  bit = ((buffer >> avail) & 1) == 1;

#if 0  // Switch to 1 for debug purposes
  if (bit)
    std::cout << '1' << std::endl;
  else
    std::cout << '0' << std::endl;
#endif

  return bit;
}

char BinaryInputStream::GetChar() {
  int eight_bits = 0;

  for (int j = 0; j < 8; j++){
    if (GetBit()){
      eight_bits = eight_bits + pow(2, (7 - j));
    }
  }

  return static_cast<char>(eight_bits);
  // To be completed
}

int BinaryInputStream::GetInt() {
  int thirty_two_bits = 0;
  for (int j = 0; j < 32; j++){
    if (GetBit()){
      thirty_two_bits = thirty_two_bits + pow(2, (31 - j));
    }
  }

  return thirty_two_bits;
  // To be completed
}

class BinaryOutputStream {
public:
  explicit BinaryOutputStream(std::ofstream &ofs);
  ~BinaryOutputStream();

  void Close();

  void PutBit(bool bit);
  void PutChar(char byte);
  void PutInt(int word);

private:
  std::ofstream &ofs;
  char buffer = 0;
  size_t count = 0;

  // Helpers
  void FlushBuffer();
};

BinaryOutputStream::BinaryOutputStream(std::ofstream &ofs) : ofs(ofs) { }

BinaryOutputStream::~BinaryOutputStream() {
  Close();
}

void BinaryOutputStream::Close() {
  FlushBuffer();
}

void BinaryOutputStream::FlushBuffer() {
  // Nothing to flush
  if (!count)
    return;
  // If buffer isn't complete, pad with 0s before writing
  if (count > 0)
    buffer <<= (8 - count);

  // Write to output stream
  ofs.put(buffer);

  // Reset buffer
  buffer = 0;
  count = 0;
}

void BinaryOutputStream::PutBit(bool bit) {
  // Make some space and add bit to buffer
  buffer <<= 1;
  if (bit)
    buffer |= 1;

  // If buffer is full, write it
  if (++count == 8)
    FlushBuffer();
}

void BinaryOutputStream::PutChar(char byte) {
  int i_byte = int(byte);
  int temp = 0;
  for (int i = 8; i > 0 ; i--){
    temp = i - 1;
    if(i_byte & (1 << temp )){
      PutBit(true);
    } else{
      PutBit(false);
    }
  }
}

void BinaryOutputStream::PutInt(int word) {
  int temp = 0;
  for (int i = 32; i > 0 ; i--){
    temp = i - 1;
    if(word & (1 << temp )){
      PutBit(true);
    } else{
      PutBit(false);
    }

  }
}

#endif  // BSTREAM_H_