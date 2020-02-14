#ifndef HUFFMAN_H_
#define HUFFMAN_H_
#include <streambuf>
#include <cerrno>
#include <array>
#include <cstddef>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

#include "bstream.h"
#include "pqueue.h"

class HuffmanNode {
public:
  explicit HuffmanNode(char ch, size_t freq,
                       HuffmanNode *left = nullptr,
                       HuffmanNode *right = nullptr)
          : ch_(ch), freq_(freq), left_(left), right_(right) { }


  bool IsLeaf() {
    // Node is a leaf if it doesn't have any children
    return left_ == nullptr && right_ == nullptr;
  }

  bool operator < (const HuffmanNode &n) const {
    // In case of equality, make it deterministic based on character
    if (freq_ == n.freq_)
      return ch_ < n.ch_;
    // Otherwise compare frequencies
    return freq_ < n.freq_;
  }

  size_t freq() { return freq_; }
  char data() { return ch_; };
  HuffmanNode* left() { return left_; }
  HuffmanNode* right() { return right_; }

private:
  char ch_;
  size_t freq_;
  HuffmanNode *left_, *right_;
};

class Huffman {
public:
  static void Compress(std::ifstream &ifs, std::ofstream &ofs);

  static void Decompress(std::ifstream &ifs, std::ofstream &ofs);

private:
  static void ReadFile(std::ifstream &ifs, std::map<char, size_t> &Nodemap, std::string &text);
  static void FormQueue(PQueue<HuffmanNode, std::less<HuffmanNode >>  &MinQueue, std::map<char, size_t> &Nodemap);
  static HuffmanNode* FormTree(PQueue<HuffmanNode, std::less<HuffmanNode >>  &MinQueue);
  static void OutPutTree(HuffmanNode* Root, BinaryOutputStream &outputStream);
  static void OutPutSize(std::map<char, size_t> &Nodemap, BinaryOutputStream &outputStream);
  static void BuildTable(std::map<char, std::string> &Codemap, HuffmanNode* Root, std::string &notation);
  static void OutputString(std::string &text, BinaryOutputStream &outputStream, std::map<char, std::string> &CodeMap);
};
//helper function to read the file and count the frequency
void Huffman::ReadFile(std::ifstream &ifs, std::map<char, size_t> &Nodemap, std::string &text) {
  char content;
  while (ifs.peek() != EOF) {
    content = ifs.get();
    text.push_back(content);
    if(Nodemap.count(content))
      Nodemap[content]++;
    else
      Nodemap[content] = 1;
  }

}
//helper function to from the min priority queue
void Huffman::FormQueue(PQueue<HuffmanNode, std::less<HuffmanNode >>  &MinQueue, std::map<char, size_t> &Nodemap) {
  for (auto itr = Nodemap.cbegin(); itr != Nodemap.cend(); itr++) {
    HuffmanNode HuffmanNodeEach(itr->first, itr->second);
    MinQueue.Push(HuffmanNodeEach);
  }
}
//helper function to form the huffman tree
HuffmanNode* Huffman::FormTree(PQueue<HuffmanNode, std::less<HuffmanNode >> &MinQueue) {
  HuffmanNode* Newnode;
  if (MinQueue.Size() == 1) {
    HuffmanNode* Newnode = new HuffmanNode(MinQueue.Top().data(), MinQueue.Top().freq());
    MinQueue.Pop();
    MinQueue.Push(*Newnode);
    return Newnode;
  }

  while (MinQueue.Size() != 1) {
    HuffmanNode* Node1 = new HuffmanNode(MinQueue.Top().data(), MinQueue.Top().freq(),
                                         MinQueue.Top().left(), MinQueue.Top().right());
    MinQueue.Pop();
    HuffmanNode* Node2 = new HuffmanNode(MinQueue.Top().data(), MinQueue.Top().freq(),
                                         MinQueue.Top().left(), MinQueue.Top().right());
    MinQueue.Pop();
    Newnode = new HuffmanNode('\0', (Node1->freq() + Node2->freq()), Node1, Node2);
    MinQueue.Push(*Newnode);

  }
  return Newnode;
}
//helper function to output the huffman tree
void Huffman::OutPutTree(HuffmanNode* n, BinaryOutputStream &outputStream) {
  if(!n) return;

  if (!n->IsLeaf())
    outputStream.PutBit(false);
  else {
    outputStream.PutBit(true);
    outputStream.PutChar(n->data());
  }
  OutPutTree(n->left(), outputStream);
  OutPutTree(n->right(), outputStream);
}
//helper function to output the size of the file
void Huffman::OutPutSize(std::map<char, size_t> &Nodemap, BinaryOutputStream &outputStream){
  int count = 0;
  for (auto itr = Nodemap.cbegin(); itr != Nodemap.cend(); itr++)
    count += itr->second;
  outputStream.PutInt(count);
}
//helper function to build the code table
void Huffman::BuildTable(std::map<char, std::string> &Codemap,
                         HuffmanNode* n, std::string &notation){
  if(!n) return;


  if(n->IsLeaf()) {
    Codemap[n->data()] = notation;
  }
  if (n->left()) {
    std::string original = notation;
    notation.append("0");
    BuildTable(Codemap, n->left(), notation);
    if (n->right()) {
      original.append("1");
      BuildTable(Codemap, n->right(), original);
    }
  }

}
//helper function to output the strings
void Huffman::OutputString(std::string &text, BinaryOutputStream &outputStream,
                           std::map<char, std::string> &CodeMap) {
  for (unsigned int i = 0; i < text.length(); i++) {
    char zimu = text[i];
    std::string temps = CodeMap[zimu];

    for (unsigned int j = 0; j < temps.length(); j++){
      outputStream.PutBit((temps[j] == '1'));
    }
  }

}
void Huffman::Compress(std::ifstream &ifs, std::ofstream &ofs){
  //to read the file and count the frequency with the help of map
  std::map <char, size_t >Nodemap;
  std::string text;
  ReadFile(ifs, Nodemap, text);
  //to build a min priority queue of Huffman leaf nodes
  PQueue<HuffmanNode, std::less<HuffmanNode >> MinQueue;
  FormQueue(MinQueue, Nodemap);
  //to build a huffman tree
  HuffmanNode* Root = FormTree(MinQueue);

  //to output the huffman tree
  BinaryOutputStream outputStream(ofs);
  OutPutTree(Root, outputStream);
  //to output the number of total characters
  OutPutSize(Nodemap, outputStream);
  //to build the map containing the huffman code
  std::map<char, std::string>Codemap;
  std::string notation;
  if (Root->IsLeaf()){
    notation = "0";
    Codemap[Root->data()] = notation;
  } else {
    BuildTable(Codemap, Root, notation);
    OutputString(text, outputStream, Codemap);
  }
  //to OutPut the encoded string


}

void Huffman::Decompress(std::ifstream &ifs, std::ofstream &ofs) {
  BinaryInputStream inputStream(ifs);
  BinaryOutputStream outputStream(ofs);

  std::map<std::string, char> Nodemap;
  std::string notation;

  size_t index = 0;
  bool bit = inputStream.GetBit();
  do {
    notation.resize(index);
    if (bit){
      notation = notation + '1';
    } else {
      notation = notation + '0';
    }
    bit = inputStream.GetBit();
    if (bit){
      Nodemap[notation] = inputStream.GetChar();
    }
    if (bit){
      index = notation.find_last_not_of('1');
    } else {
      index = notation.size();
    }
  } while (index != std::string::npos);

  int total_num = inputStream.GetInt();

  std::map<std::string, char>::iterator temp_map;
  notation.resize(0);

  bool tempb = inputStream.GetBit();
  while (ifs.peek() != EOF) {
    if (tempb){
      notation = notation + '1';
    } else {
      notation = notation + '0';
    }

    temp_map = Nodemap.find(notation);
    if (temp_map != Nodemap.end()) {
      notation.resize(0);
      outputStream.PutChar(temp_map->second);
    }
    tempb = inputStream.GetBit();
  }
  if (Nodemap.count('\n'){

  }
}

/*void Huffman::Decompress(std::ifstream &ifs, std::ofstream &ofs) {
  BinaryInputStream inputStream(ifs);
  BinaryOutputStream outputStream(ofs);
  std::map <char, size_t >Nodemap;
  char content = '\0';
  size_t order = 1;
  bool ts = false;

  while (!ts) {
    ts = inputStream.GetBit();
  }
  int upnum = 5;
  while(true){
    if (ts){
      content = inputStream.GetChar();
      Nodemap[content] = order;
      upnum = 5;
      order = order + 1;
      ts = inputStream.GetBit();
    } else {
      upnum = upnum - 1;
      if (upnum == 0){
        break;
      }
      ts = inputStream.GetBit();
    }
  }

  PQueue<HuffmanNode> MinQueue;
  FormQueue(MinQueue, Nodemap);
  HuffmanNode* Root = FormTree(MinQueue);

  int total_num = 0;
  for(int i = 0; i < 27;i++){
    if (ts){
      total_num = total_num + pow(2, (27 - i));

    }
    ts = inputStream.GetBit();
  }

  std::map<char, std::string>Codemap;
  std::string notation;
  BuildTable(Codemap, Root, notation);

  HuffmanNode* tempnode = Root;

  if(Root->IsLeaf()){
    for (int i = 0; i < total_num; i++){
      ofs << tempnode->data();
    }
  } else {
    while(total_num > 0){
      if (tempnode->data() != '\0'){
        ofs << tempnode->data();
        tempnode = Root;
        total_num = total_num - 1;
      }
      if(inputStream.GetBit()){
        tempnode = tempnode->right();
      } else {
        tempnode = tempnode->left();
      }
    }
  }
}*/



#endif  // HUFFMAN_H_