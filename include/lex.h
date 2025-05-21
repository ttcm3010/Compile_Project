#ifndef _LEX_H_
#define _LEX_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <unordered_map>
using namespace std;


// 编号化的关键字、运算符、界符
const unordered_map<string, int> keyword_map = {
    {"int", 1}, {"void", 2}, {"return", 3}, {"const", 4},
    {"main", 5}, {"float", 6}, {"if", 7}, {"else", 8}
};

const unordered_map<string, int> operator_map = {
    {"+", 6}, {"-", 7}, {"*", 8}, {"/", 9}, {"%", 10}, {"=", 11},
    {">", 12}, {"<", 13}, {"==", 14}, {"<=", 15}, {">=", 16}, {"!=", 17},
    {"&&", 18}, {"||", 19}
};

const unordered_map<char, int> delimiter_map = {
    {'(', 20}, {')', 21}, {'{', 22}, {'}', 23},
    {';', 24}, {',', 25}
};

// token序列（可选）
vector<string> tokens;

enum State {
    START,
    IDENTIFIER,
    NUMBER,
    FLOAT_NUMBER,
    OPERATOR,
    DELIMITER,
    STRING_LITERAL,
    CHAR_LITERAL,
    DONE
};

void FSM(const string& input);

#endif