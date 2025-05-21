# 编译器设置
CXX := g++
CXXFLAGS := -Iinclude -g -Wall -Wextra -std=c++17

# 目录结构
SRC_DIR := src
BIN_DIR := bin
TEST_DIR := test

# 目标文件
LEX_TARGET := $(BIN_DIR)/lex
SYNTAX_TARGET := $(BIN_DIR)/syntax

# 默认目标
.PHONY: all
all: $(LEX_TARGET) $(SYNTAX_TARGET)

# 创建bin目录
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# 仅编译lex
.PHONY: lex
lex: $(LEX_TARGET)

# 仅syntax
.PHONY: syntax
syntax: $(SYNTAX_TARGET)

# 编译并运行 lex 和 syntax
.PHONY: run
run: all
	@echo "=== Running lex ==="
	./$(LEX_TARGET)
	@echo "=== Running syntax ==="
	./$(SYNTAX_TARGET)

# 构建lex可执行文件
$(LEX_TARGET): $(SRC_DIR)/lex.cpp $(SRC_DIR)/util.cpp include/lex.h | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# 构建syntax可执行文件
$(SYNTAX_TARGET): $(SRC_DIR)/syntax.cpp $(SRC_DIR)/util.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# 清理（linux|windows兼容）
.PHONY: clean
clean:
ifeq ($(OS),Windows_NT)
	@if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)
else
	rm -rf $(BIN_DIR)
endif
