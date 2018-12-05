CXX :=  g++
EXEC := a.out
SRC_DIR := ./src
BUILD_DIR := ./build
#SRCS := dumpDomain.cpp jdomain.h jdomain.cpp
#SRCS := $(addprefix $(SRC_DIR), $(SRCS))
OBJS := dumpDomain.o jdomain.o
OBJS := $(addprefix $(BUILD_DIR)/, $(OBJS))
CFLAGS = -std=c++11 -Wall -g
LIBS = 

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) -c $< -o $@ $(CFLAGS)

$(EXEC): $(OBJS)
	$(CXX) -o $(EXEC) $(OBJS) $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -rf $(EXEC) $(OBJS)

