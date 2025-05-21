#include"util.h"


// 从文件读取token
vector<Token> readTokensFromFile(const string& filename) {
    vector<Token> tokens;
    ifstream file(filename);
    string content, type;

    while (file >> content >> type) {
        tokens.push_back({content, type});
    }

    return tokens;
}

// 插入缺失的else结构
void insertMissingElses(vector<Token>& tokens) {
    stack<int> ifStack; // 存储每个if对应的索引
    vector<int> unmatchedIfIndices; // 存储无对应else的if块的闭合位置

    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].content == "if" && tokens[i].type == "<KW,7>") {
            ifStack.push(i);
        } else if (tokens[i].content == "else" && tokens[i].type == "<KW,8>") {
            if (!ifStack.empty()) {
                ifStack.pop(); // 匹配一个if
            }
        }
    }

    // 处理剩余未匹配的if
    while (!ifStack.empty()) {
        int ifIndex = ifStack.top();
        ifStack.pop();

        // 找到该if语句对应的最外层 } 位置
        int braceCount = 0;
        size_t insertPos = ifIndex;

        // 向后寻找if块结束位置（匹配花括号）
        for (size_t i = ifIndex; i < tokens.size(); ++i) {
            if (tokens[i].content == "{" && tokens[i].type == "<SE,22>") {
                braceCount++;
            } else if (tokens[i].content == "}" && tokens[i].type == "<SE,23>") {
                braceCount--;
                if (braceCount == 0) {
                    insertPos = i + 1;
                    break;
                }
            }
        }

        // 插入else块
        vector<Token> elseTokens = {
            {"else", "<KW,8>"},
            {"{", "<SE,22>"},
            {"}", "<SE,23>"}
        };
        tokens.insert(tokens.begin() + insertPos, elseTokens.begin(), elseTokens.end());
    }
}

// 打印tokens（调试或输出结果）
void printTokens(const vector<Token>& tokens) {
    for (const auto& token : tokens) {
        cout << token.content << "\t" << token.type << endl;
    }
}

// 写入token到文件
void writeTokensToFile(const string& filename, const vector<Token>& tokens) {
    ofstream file(filename);
    for (const auto& token : tokens) {
        file << token.content << "\t" << token.type << endl;
    }
}

// 主函数
void handle() {
    string filename = "lexical.txt"; 
    string outputFile = "lexical.txt";
    vector<Token> tokens = readTokensFromFile(filename);

    insertMissingElses(tokens);

    writeTokensToFile(outputFile, tokens);

    // printTokens(tokens); // 打印结果
    cout << "Success! " << endl;
    return ;
}


