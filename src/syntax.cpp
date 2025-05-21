#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility> // for std::pair
#include <cctype>
#include <cstdio>  // for FILE*, freopen
#include <cstdlib> // for exit
#include "util.h"

using namespace std;

const int MAX_NT = 100; // 非终结符的最大数量
const char GRAMMAR_FILE[] = "data/grammer.txt";//文法文件路径
const char LEXICAL_FILE[] = "lexical.txt";
const char SYNTAX_OUTPUT_FILE[] = "syntax_analysis.txt";
const char FIRST_FILE[] = "first.txt";
const char FOLLOW_FILE[] = "follow.txt";
const char TABLE_FILE[] = "table.txt";

set<string> nonTerminals, terminals;// 非终结符结合和终结符集合
map<string, int> nonTerminalId; //符号转编号
map<int, string> idToNonTerminal;   // 编号转符号
vector<vector<string>> productions[MAX_NT]; //production[Id] 存储非终结符编号 Id 的所有产生式右部
int nonTerminalCount;   // 非终结符的数量

map<string, set<string>> firstSet;  // 存储每个符号的FIRST集合的映射
map<string, set<string>> followSet; // 存储每个符号的FOLLOW集合的映射

pair<int, int> parseTable[MAX_NT][MAX_NT];  // 预测分析表（LL(1)分析表）
map<string, int> nonTermIndex; // 非终结符集合的符号到索引的映射
map<string, int> termIndex;    // 终结符集合的符号到索引的映射
int cntNonTerm, cntTerm;    // 非终结符集合、终结符集合中元素个数
const pair<int, int> EMPTY_CELL = {0, -1};  // 表示表格未填状态（用于初始化）

string parseStack[MAX_NT];  // 符号栈
int stackTop = 0;   // 栈顶指针
vector<string> inputTokens;
int analysisStepCount = 0;  //分析步数

/*
    1.从grammar.txt中读取文法规则
    2.将左部符号放入非终结符集合nonTerminals
    3.将右部符号初步放入终结符集合terminals
    4.构造产生式（production[Id].pushback()）
    5.terminals通过差集去除nonTerminals，这样一来nonTerminals和terminals分别存储文法中的非终结符和终结符
*/
void loadGrammar()
{
    //处理一下词法结果
    handle();
    //打开文法文件
    FILE *fp = freopen(GRAMMAR_FILE, "r", stdin);
    if (!fp)
    {
        cerr << "Error opening grammar file: " << GRAMMAR_FILE << endl;
        exit(1);
    }
    while (true)
    {
        int c = getchar();
        if (c == EOF)
            break;
        string curToken;
        bool beforeArrow = true;
        int lhsId = 0;
        vector<string> *rhs = nullptr;
        while (c != EOF && c != '\n')
        {   //未读到文件结束符和换行符
            if (isspace(c))
            {   //若读到空格
                if (!curToken.empty())
                {
                    if (curToken == "->")
                    {   
                        beforeArrow = false;    //转为右部解析
                    }
                    else if (beforeArrow)
                    {
                        //当前为左部非终结符
                        lhsId = nonTerminalId[curToken];
                        if (lhsId == 0)
                        {   
                            //建立映射
                            lhsId = ++nonTerminalCount;
                            nonTerminalId[curToken] = lhsId;
                            idToNonTerminal[lhsId] = curToken;
                        }
                        nonTerminals.insert(curToken);  // 加入非终结符集合
                        rhs = new vector<string>;   // 新建右部产生式容器
                    }
                    else
                    {   //当前是一个右部的符号（可能为终结符或非终结符）
                        rhs->push_back(curToken);
                        terminals.insert(curToken); // 暂时加入终结符集合
                    }
                    curToken.clear();
                }
            }
            else
            {   //未读到空格则正常读取
                curToken.push_back(char(c));
            }
            c = getchar();
        }
        if (!curToken.empty())
        {
            if (rhs)
            {
                rhs->push_back(curToken);   //将最后一个符号加入右部
                terminals.insert(curToken); //放入终结符集合
            }
        }
        if (lhsId > 0 && rhs)
        {
            productions[lhsId].push_back(*rhs); //将该产生式加入对应非终结符
        }
    }
    //从终结符集合中删除非终结符
    for (const auto &nt : nonTerminals)
    {
        terminals.erase(nt);
    }
    fclose(fp);
    //这样一来就得到终结符集合与非终结符集合
}

//获得每个终结符和非终结符的FIRST集
void computeFirst(const string &symbol)
{
    // 若当前 token 的 FIRST 集已经求过则返回，避免重复递归
    if (firstSet.count(symbol) > 0)
        return;
    // 若是终结符，FIRST集就是他本身
    if (terminals.count(symbol))
    {
        firstSet[symbol] = {symbol};
        return;
    }
    int id = nonTerminalId[symbol];
    set<string> resultSet;
    for (const auto &prod : productions[id])
    {
        bool allDeriveEpsilon = true;
        for (const string &sym : prod)
        {
            computeFirst(sym);
            const set<string> &symFirst = firstSet[sym];
            bool hasEpsilon = false;
            for (const string &s : symFirst)
            {   // 将 str 的 FIRST 集加入 A 的 FIRST 集（除 "$"）
                if (s == "$")
                {
                    hasEpsilon = true;
                }
                else
                {
                    resultSet.insert(s);
                }
            }
            //若 str 的 FIRST 集中无 "$"，AllHasE = false，break
            if (!hasEpsilon)
            {
                allDeriveEpsilon = false;
                break;
            }
        }
        if (allDeriveEpsilon)
        {
            resultSet.insert("$");
        }
    }
    firstSet[symbol] = resultSet;

    //确认所有右部符号的FIRST集计算完毕
    for (const auto &prod : productions[id])
    {
        for (const string &sym : prod)
        {
            computeFirst(sym);
        }
    }
}

//构造所有非终结符的FOLLOW集
bool computeFollow()
{   
    /*
     * 如果当前这一轮 FOLLOW 集有更新，则返回 true；否则返回 false
     * 用于驱动外层循环直到 FOLLOW 集不再变化
    */
    //标记是否有FOLLOW集发生了变化
    bool changed = false;
    for (const auto &nt : nonTerminals)
    {   // 如果当前非终结符没有 FOLLOW 集，先初始化为空集合
        if (!followSet.count(nt))
        {
            followSet[nt] = {};
        }
    }
    for (const auto &nt : nonTerminals)
    {
        int A = nonTerminalId[nt];
        for (const auto &prod : productions[A])
        {
            for (int i = 0; i < (int)prod.size(); ++i)
            {
                const string &B = prod[i];
                if (nonTerminals.count(B))
                {
                    bool allEpsilon = true;
                    for (int j = i + 1; j < (int)prod.size(); ++j)
                    {
                        const string &sym = prod[j];
                        const set<string> &firstBeta = firstSet[sym];
                        for (const auto &f : firstBeta)
                        {
                            if (f != "$")
                            {
                                if (!followSet[B].count(f))
                                {
                                    followSet[B].insert(f);
                                    changed = true;
                                }
                            }
                        }
                        if (!firstBeta.count("$"))
                        {
                            allEpsilon = false;
                            break;
                        }
                    }
                    if (allEpsilon)
                    {
                        for (const auto &f : followSet[nt])
                        {
                            if (!followSet[B].count(f))
                            {
                                followSet[B].insert(f);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
    return changed;
}

//用课堂上的LL(1)算法构建预测分析表
bool buildParseTable()
{
    cntNonTerm = 0;
    //给所有非终结符编号
    for (auto &nt : nonTerminals)
    {
        nonTermIndex[nt] = ++cntNonTerm;
    }
    cntTerm = 0;
    //加入"#"代表结束符，然后对所有终结符编号
    terminals.insert("#");
    for (auto &t : terminals)
    {
        termIndex[t] = ++cntTerm;
    }
    //初始化，将所有格子设置为未填状态
    for (int i = 0; i <= cntNonTerm; ++i)
    {
        for (int j = 0; j <= cntTerm; ++j)
        {
            //{0,-1}
            parseTable[i][j] = EMPTY_CELL;
        }
    }
    /*
    // 查看映射
    for (auto it = nonTerminalId.begin(); it != nonTerminalId.end(); ++it)
    {
        const string& nt = it->first;
        int id = it->second;
        cout << "NonTerminal ID " << id << ": " << nt << endl;
    }
    for (auto it = termIndex.begin(); it != termIndex.end(); ++it)
    {
        const string& term = it->first;
        int id = it->second;
        cout << "Terminal ID " << id << ": " << term << endl;
    }
    */
    for (auto &nt : nonTerminals)
    {
        int A = nonTerminalId[nt];  //当前非终结符对应在 v 中的索引
        bool hasEpsilonProd = false;    // 是否存在可以推出 ε（空串）
        for (int idx = 0; idx < (int)productions[A].size(); ++idx)
        {
            const auto &prod = productions[A][idx]; // 第 i 条产生式右部
            set<string> firstAlpha; // 预测集
            bool allEps = true; // 当前产生式是否能推出 ε（初始设为能）
            for (const string &sym : prod)
            {   // 遍历当前符号的 FIRST 集
                for (const auto &f : firstSet[sym])
                {
                    if (f == "$")
                        continue;
                    firstAlpha.insert(f);   // 加入预测集
                }
                if (!firstSet[sym].count("$"))
                {   
                    //当前符号的first集中没有 ε
                    allEps = false;
                    break;
                }
            }
            //用预测集firstAlpha来填表
            for (const auto &t : firstAlpha)
            {
                int col = termIndex[t];
                if (parseTable[A][col] != EMPTY_CELL)
                {   // 表格中已有项，说明冲突
                    cout<<"P1"<<endl;
                    cout<<"Debug:"<<A<<" "<<col<<endl;
                    return true;
                }
                parseTable[A][col] = {A, idx};  // 填入表格：使用第 idx 条产生式
            }
            if (allEps)
            {
                hasEpsilonProd = true;
            }
        }
        if (hasEpsilonProd)
        {
            for (const auto &f : followSet[nt])
            {
                int col = termIndex[f];
                if (parseTable[A][col] != EMPTY_CELL)
                {   
                    cout<<"P2"<<endl;
                    cout<<"Debug:"<<A<<" "<<col<<endl;
                    return true;
                }
                parseTable[A][col] = {A, MAX_NT - 1}; // 用特定编号MAX_NT-1 表示 ε 产生式
            }
        }
    }
    return false;
}

// 解析词法分析的单行内容，提取用于语法分析的token
void parseLexicalLine(const string &line)
{
    // 假设行格式为："词素 token类型, ... >" 的形式
    string lexeme, tokenType;
    int i = 0;
    // 读取词素（直到遇到空白符）
    while (i < (int)line.size() && !isspace(line[i]))
    {
        lexeme.push_back(line[i++]);
    }
    // 跳过空格/制表符
    while (i < (int)line.size() && isspace(line[i]))
        i++;
    // 跳过1个字符（此处是'<'）
    if (i < (int)line.size())
        i++;
    // 读取token类型（直到逗号','）
    while (i < (int)line.size() && line[i] != ',')
    {
        tokenType.push_back(line[i++]);
    }
    // 跳过逗号及后续内容直到'>'
    if (i < (int)line.size() && line[i] == ',')
        i++;
    while (i < (int)line.size() && line[i] != '>')
        i++;
    // 决定压入哪种token：如果token类型是IDN、FLOAT或INT，压入类型；否则压入词素
    if (tokenType != "IDN" && tokenType != "INT"&& tokenType != "FLOAT")
    {
        inputTokens.push_back(lexeme);
    }
    else
    {
        inputTokens.push_back(tokenType);
    }
}

//用预测分析表和输入的tokens做语法分析
bool runParsing()
{
    //将 stdout 重定向到输出文件，便于记录分析过程
    FILE *fp = freopen(SYNTAX_OUTPUT_FILE, "w", stdout);
    if (!fp)
    {
        cerr << "Error opening syntax output file" << endl;
        return false;
    }
    //读入串的指针
    /*
        begin()返回指向容器第一个元素的迭代器
        这里的auto为自动推导变量类型，这里为std::vector<string>::iterator
    */
    auto it = inputTokens.begin();
    while (true)
    {
        string topSymbol = parseStack[stackTop - 1];    // 当前栈顶符号
        if (terminals.count(topSymbol))
        {   //若栈顶为终结符(存在于终结符集合中)
            if (topSymbol == *it)
            {
                if (topSymbol == "#")
                {
                    // 匹配成功并结束
                    cout << ++analysisStepCount << " " << "EOF#EOF" << "\t" << "accept" << endl;
                    fflush(fp);
                    fclose(fp);
                    freopen("CON", "w", stdout);
                    return true;
                }
                else
                {   
                    //终结符匹配，移进
                    cout << ++analysisStepCount << " " << topSymbol << "#" << *it << "\t" << "move" << endl;
                    stackTop--;
                    ++it;
                }
            }
            else
            {   
                // 栈顶终结符 != 当前输入，出错
                cout << ++analysisStepCount << " " << topSymbol << "#" << *it << "\t" << "error" << endl;
                fflush(fp);
                fclose(fp);
                freopen("CON", "w", stdout);
                cout<<"DeBug:P1"<<endl;
                return false;
            }
        }
        else
        {
            //若栈顶为非终结符(存在于非终结符集合中)
            int A = nonTerminalId[topSymbol];   // 非终结符映射为预测表行
            int col = termIndex[*it];   // 当前输入符号映射为预测表列
            auto entry = parseTable[A][col];
            if (entry.second == -1)
            {   //无法推导，报错
                cout << ++analysisStepCount << " " << topSymbol << "#" << *it << "\t" << "error" << endl;
                fflush(fp);
                fclose(fp);
                freopen("CON", "w", stdout);
                cout << A << " " << col << endl;
                cout << "error info : " << topSymbol << " " << *it << endl;
                return false;
            }
            else
            {    // 使用产生式进行替换
                cout << ++analysisStepCount << " " << topSymbol << "#" << *it << "\t" << "reduction" << endl;
                stackTop--; // 弹出非终结符
                int prodIndex = entry.second;
                if (prodIndex != MAX_NT - 1)
                {   //不为空产生式
                    const auto &prod = productions[A][prodIndex];   // 得到右部
                    for (int i = (int)prod.size() - 1; i >= 0; --i)
                    {
                        parseStack[stackTop++] = prod[i];   // 右部倒序压栈
                    }
                }
            }
        }
    }
}

//读取词法分析结果，并开始分析
bool performAnalysis()
{
    stackTop = 0;
    parseStack[stackTop++] = "#";   // 压入栈底标志
    parseStack[stackTop++] = "program"; // 压入文法开始符号
    FILE *fp = freopen(LEXICAL_FILE, "r", stdin);
    if (!fp)
    {   
        cout<<"DeBug:P2"<<endl;
        cerr << "Error opening lexical file: " << LEXICAL_FILE << endl;
        return false;
    }
    char ch;
    string current;
    //用于从标准输入（通常是键盘输入或重定向的文件输入）中读取下一个字符
    //它返回一个int类型的字符值
    //在读到文件终止符EOF前都不会停止
    while ((ch = getchar()) != EOF)
    {
        if (ch == '\n')
        {   // 分割每一行的token
            /*
                示例： int a = 10 ;
                vector中： int IDN = INT ;
                (一一对应)
            */
            if (!current.empty())
                parseLexicalLine(current);
            current.clear();
        }
        else
        {
            current.push_back(ch);
        }
    }
    fclose(fp);
    //加入结束标志
    inputTokens.push_back("#");
    return runParsing();
}

// Print FIRST sets to file
void printFirstSets()
{
    FILE *fp = freopen(FIRST_FILE, "w", stdout);
    for (const auto &nt : nonTerminals)
    {
        cout << nt << endl
             << " -- ";
        for (const auto &f : firstSet[nt])
        {
            cout << "'" << f << "' ";
        }
        cout << endl;
    }
    fflush(fp);
    fclose(fp);
    freopen("CON", "w", stdout);
}

// Print FOLLOW sets to file
void printFollowSets()
{
    FILE *fp = freopen(FOLLOW_FILE, "w", stdout);
    for (const auto &nt : nonTerminals)
    {
        cout << nt << endl
             << " -- ";
        for (const auto &f : followSet[nt])
        {
            cout << "'" << f << "' ";
        }
        cout << endl;
    }
    fflush(fp);
    fclose(fp);
    freopen("CON", "w", stdout);
}

// Print parse table to file
void printParseTable()
{
    FILE *fp = freopen(TABLE_FILE, "w", stdout);
    cout << "   ";
    for (const auto &t : terminals)
    {
        cout << t << " ";
    }
    cout << endl;
    for (const auto &nt : nonTerminals)
    {
        int A = nonTerminalId[nt];
        cout << nt << " ";
        for (const auto &t : terminals)
        {
            int j = termIndex[t];
            cout << parseTable[A][j].second << " ";
        }
        cout << endl;
    }
    fflush(fp);
    fclose(fp);
    freopen("CON", "w", stdout);
}
//打印终止符与非终止符
void printSet(const set<string>& s) {
    for (auto it = s.begin(); it != s.end(); ++it) {
        cout << *it << " ";
    }
    cout << endl;
}

// Main process: read grammar, compute FIRST/FOLLOW, build table, and parse tokens
void syntax()
{
    loadGrammar();
    firstSet["$"] = {"$"};
    //printSet(terminals);    // 打印终止符集合
    //cout<<"======================================"<<endl;
    //printSet(nonTerminals); // 打印非终止符集合
    computeFirst("program");
    printFirstSets();
    followSet["program"].insert("#");
    while (computeFollow())
        ;
    printFollowSets();
    if (buildParseTable())
    {
        cout << "the grammar is not LL(1)" << endl;
        exit(0);
    }
    printParseTable();
    if (!performAnalysis())
    {
        cout << "the lexical info has error(s)" << endl;
    }
}

int main()
{
    syntax();
    cout << "Accept! " << endl;
    return 0;
}
