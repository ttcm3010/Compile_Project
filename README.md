# C-- Compiler

TJU 计科编译原理与技术大作业：实现C--语言的编译器前端。

## 文件结构

```
.
│  Makefile
│  README.md
│  TEST.sy			// 默认解析文件
|
├─data
│      grammer.txt		// 文法文件
│  
├─include
│      util.h
│  
├─src
│      lex.cpp		// 词法分析器
│      syntax.cpp		// 语法分析器
│      util.cpp     //辅助处理词法分析结果
│  
└─test
        test.sy
```

## 文件说明

+ TEST.sy为当前分析文件
+ 词法分析结果可输出到lexical.txt，保存token序列
+ 语法分析结果分为first集、follow集、预测分析表table.txt和分析结果syntax_analysis.txt，分别输出到一个txt文件

## 编译方式

+ 编译生成词法分析器：（在bin目录下生成lex.exe文件）

  ```
  make lex
  ```


+ 编译生成语法分析器：（在bin目录下生成syntax.exe文件）

  ```
  make syntax
  ```

+ 同时编译出词法分析器和语法分析器：（在bin目录下生成lex.exe和syntax.exe）

  ```
  make / make all
  ```

## 运行方式


+ 词法分析器运行方式：

    +默认运行lex：（分析主目录下的TEST.sy）

    ```
    ./bin/lex.exe
    ```

    +传入测试输入test/test.sy编译lex：
   
    ```
    ./bin/lex.exe test.sy
    ```

  词法分析器的输出为lexical.txt，存放解析的token序列。

+ 语法分析器运行方式：

  ```
  ./bin/syntax.exe
  ```

  注：必须先运行词法分析器得到lexical.txt后才能运行语法分析器。

+ 同时运行词法分析器和语法分析器，解析主目录下的默认文件TEST.sy，将词法分析器结果输出到lexical.txt，将语法分析结果输出到syntax_analysis.txt。

  ```
  make run
  ```

+ 清除可执行文件和输出结果

  ```
  make clean
  ```

  
