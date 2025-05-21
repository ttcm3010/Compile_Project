#include "lex.h"


bool isKeyword(const string& s) {
    return keyword_map.find(s) != keyword_map.end();
}

bool isOperator(const string& s) {
    return operator_map.find(s) != operator_map.end();
}

bool isDelimiter(char c) {
    return delimiter_map.find(c) != delimiter_map.end();
}


void FSM(const string& input) {
    size_t i = 0;
    State state = START;
    string current = "";
    while (i < input.length()) {
        char ch = input[i];

        switch (state) {
            case START:
                if (isspace(ch)) {
                    i++;
                } else if (isalpha(ch) || ch == '_') {
                    current += ch;
                    state = IDENTIFIER;
                    i++;
                } else if (isdigit(ch)) {
                    current += ch;
                    state = NUMBER;
                    i++;
                } else if (ch == '\"') {
                    current += ch;
                    state = STRING_LITERAL;
                    i++;
                } else if (ch == '\'') {
                    current += ch;
                    state = CHAR_LITERAL;
                    i++;
                } else if (isDelimiter(ch)) {
                    cout << ch << "\t<SE," << delimiter_map.at(ch) << ">" << endl;
                    tokens.push_back(string(1, ch));
                    i++;
                } else {
                    current += ch;
                    state = OPERATOR;
                    i++;
                }
                break;

            case IDENTIFIER:
                if (isalnum(ch) || ch == '_') {
                    current += ch;
                    i++;
                } else {
                    if (isKeyword(current)) {
                        cout << current << "\t<KW," << keyword_map.at(current) << ">" << endl;
                        tokens.push_back(current);
                    } else {
                        cout << current << "\t<IDN," << current << ">" << endl;
                        tokens.push_back("IDN");
                    }
                    current.clear();
                    state = START;
                }
                break;

            case NUMBER:
                if (isdigit(ch)) {
                    current += ch;
                    i++;
                } else if (ch == '.') {
                    current += ch;
                    i++;
                    state = FLOAT_NUMBER;
                } else {
                    cout << current << "\t<INT," << current << ">" << endl;
                    tokens.push_back("INT");
                    current.clear();
                    state = START;
                }
                break;

            case FLOAT_NUMBER:
                if (isdigit(ch)) {
                    current += ch;
                    i++;
                } else {
                    cout << current << "\t<FLOAT," << current << ">" << endl;
                    tokens.push_back("FLOAT");
                    current.clear();
                    state = START;
                }
                break;

            case OPERATOR:
                if (i < input.length()) {
                    string temp = current + input[i];
                    if (isOperator(temp)) {
                        current = temp;
                        i++;
                    }
                }
                if (isOperator(current)) {
                    cout << current << "\t<OP," << operator_map.at(current) << ">" << endl;
                    tokens.push_back(current);
                }
                current.clear();
                state = START;
                break;

            case STRING_LITERAL:
                current += ch;
                i++;
                while (i < input.length() && input[i] != '\"') {
                    current += input[i++];
                }
                if (i < input.length()) {
                    current += input[i++];
                }
                cout << current << "\t<STR," << current << ">" << endl;
                tokens.push_back("STR");
                current.clear();
                state = START;
                break;

            case CHAR_LITERAL:
                current += ch;
                i++;
                while (i < input.length() && input[i] != '\'') {
                    current += input[i++];
                }
                if (i < input.length()) {
                    current += input[i++];
                }
                cout << current << "\t<CHAR," << current << ">" << endl;
                tokens.push_back("CHAR");
                current.clear();
                state = START;
                break;

            default:
                break;
        }
    }

    // Final check
    if (!current.empty()) {
        if (state == IDENTIFIER) {
            if (isKeyword(current)) {
                cout << current << "\t<KW," << keyword_map.at(current) << ">" << endl;
                tokens.push_back(current);
            } else {
                cout << current << "\t<IDN," << current << ">" << endl;
                tokens.push_back("IDN");
            }
        } else if (state == NUMBER) {
            cout << current << "\t<INT," << current << ">" << endl;
            tokens.push_back("INT");
        } else if (state == FLOAT_NUMBER) {
            cout << current << "\t<FLOAT," << current << ">" << endl;
            tokens.push_back("FLOAT");
        }
    }
}

int main(int argc, char* argv[]) {
    string filename;
    if(argc==1){
        //没有参数，使用主目录下的test.sy
        filename = "TEST.sy";
    }else if(argc == 2){
        // 有一个参数
        filename = string("test/") + argv[1];
    }else{
        cerr << "Error: Too many parameter" << endl;
        return 1;
    }
    ifstream infile(filename);
    ofstream outfile("lexical.txt");
    if (!infile.is_open() || !outfile.is_open()) {
        cerr << "文件打开失败！" << endl;
        return 1;
    }

    string input((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
    input.erase(remove(input.begin(), input.end(), '\n'), input.end());

    streambuf* coutbuf = cout.rdbuf();
    cout.rdbuf(outfile.rdbuf());

    FSM(input);
    
    cout.rdbuf(coutbuf);
    infile.close();
    outfile.close();
    return 0;
}
