CXX ?= /opt/homebrew/opt/llvm/bin/clang++
CC ?= /opt/homebrew/opt/llvm/bin/clang
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude -Ithird_party/PokerHandEvaluator/cpp/include
CFLAGS ?= -std=c99 -O2 -Ithird_party/PokerHandEvaluator/cpp/include
LDFLAGS ?=

PHEVAL_DIR := third_party/PokerHandEvaluator/cpp
PHEVAL_SRCS := \
	$(PHEVAL_DIR)/src/card_sampler.cc \
	$(PHEVAL_DIR)/src/dptables.c \
	$(PHEVAL_DIR)/src/evaluator5.cc \
	$(PHEVAL_DIR)/src/evaluator5.c \
	$(PHEVAL_DIR)/src/evaluator6.cc \
	$(PHEVAL_DIR)/src/evaluator6.c \
	$(PHEVAL_DIR)/src/evaluator7.cc \
	$(PHEVAL_DIR)/src/evaluator7.c \
	$(PHEVAL_DIR)/src/tables_bitwise.c \
	$(PHEVAL_DIR)/src/hash.c \
	$(PHEVAL_DIR)/src/hashtable.c \
	$(PHEVAL_DIR)/src/hashtable5.c \
	$(PHEVAL_DIR)/src/hashtable6.c \
	$(PHEVAL_DIR)/src/hashtable7.c \
	$(PHEVAL_DIR)/src/rank.c \
	$(PHEVAL_DIR)/src/7462.c

ENGINE_SRCS := src/card.cpp src/deck.cpp src/game.cpp src/showdown.cpp

BUILD_DIR := build-make
PHEVAL_OBJS := $(addprefix $(BUILD_DIR)/pheval/,$(notdir $(PHEVAL_SRCS:.cc=.o)))
PHEVAL_OBJS := $(PHEVAL_OBJS:.c=.o)
ENGINE_OBJS := $(addprefix $(BUILD_DIR)/engine/,$(notdir $(ENGINE_SRCS:.cpp=.o)))

.PHONY: all clean test

all: $(BUILD_DIR)/libpoker_engine.a

$(BUILD_DIR)/libpoker_engine.a: $(PHEVAL_OBJS) $(ENGINE_OBJS)
	ar rcs $@ $^

$(BUILD_DIR)/pheval/%.o: $(PHEVAL_DIR)/src/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/pheval/%.o: $(PHEVAL_DIR)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/engine/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
