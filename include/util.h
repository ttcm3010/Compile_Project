#ifndef __UTIL_H__
#define __UTIL_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <string>

using namespace std;

// 用于表示一个Token结构
struct Token {
    string content; // 实际内容（如 if, else, {, } 等）
    string type;    // 如 <KW,7>
};

const char lexicalTxtPath[] = "lexical.txt";

// 从文件读取token
vector<Token> readTokensFromFile(const string& filename);
// 插入缺失的else结构
void insertMissingElses(vector<Token>& tokens) ;
// 打印tokens（调试或输出结果）
void printTokens(const vector<Token>& tokens);
// 写入token到文件
void writeTokensToFile(const string& filename, const vector<Token>& tokens);
// 主函数
void handle();


#endif