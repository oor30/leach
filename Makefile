# Makefile for chen25
-include config.mk

CXX = g++
GUROBI = $(GUROBI_PATH)
INCLUDE = $(GUROBI)/include
LIBRARY_DIR = $(GUROBI)/lib
LIBRARIES = -lgurobi_c++ -lgurobi120

CXXFLAGS = -std=c++14 -Wall -Wextra -I$(INCLUDE) -DSAMPLES_PATH=\"$(SAMPLES_PATH)\"
LDFLAGS = -L$(LIBRARY_DIR) $(LIBRARIES)

SRCS = src/main.cpp
OBJS = $(SRCS:src/%.cpp=obj/%.o)
TARGET = out/main


.PHONY: all exec clean help

all: exec

exec: $(TARGET)
	@echo "✅️ ビルドが完了しました。実行中..."
	@./$(TARGET)
	@echo "✅️ 実行が完了しました。"

$(TARGET): $(OBJS)
	@echo "🔨 ビルド中..."
	@mkdir -p out/csv
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

obj/%.o: src/%.cpp
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf obj out

help:
	@echo "利用可能なターゲット:"
	@echo "  all     - デフォルトターゲット（execと同じ）"
	@echo "  exec    - プログラムをビルドして実行"
	@echo "  clean   - ビルドファイルを削除"
	@echo "  help    - このヘルプメッセージを表示"
