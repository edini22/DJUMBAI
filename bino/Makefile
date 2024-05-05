# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra

DIRECTORIES = ../queue/mess/* ../queue/pid/* ../queue/local/* ../queue/todo/* ../queue/intd/* ../queue/info/*

PIPES = /tmp/clean_pipe

# Source files for each program
PROGRAMS = djumbai-inject djumbai-queue djumbai-send djumbai-clean
SRCS_djumbai-inject = djumbai-inject.cpp
SRCS_djumbai-queue = djumbai-queue.cpp
SRCS_djumbai-send = djumbai-send.cpp
SRCS_djumbai-clean = djumbai-clean.cpp


# Object files for each program
OBJS_djumbai-inject = $(SRCS_djumbai-inject:.cpp=.o)
OBJS_djumbai-queue = $(SRCS_djumbai-queue:.cpp=.o)
OBJS_djumbai-send = $(SRCS_djumbai-send:.cpp=.o)
OBJS_djumbai-clean = $(SRCS_djumbai-clean:.cpp=.o)


# Build targets for each program
all: clean $(PROGRAMS)

djumbai-inject: $(OBJS_djumbai-inject)
	$(CXX) $(CXXFLAGS) -o $@ $^

djumbai-queue: $(OBJS_djumbai-queue)
	$(CXX) $(CXXFLAGS) -o $@ $^	

djumbai-send: $(OBJS_djumbai-send)
	$(CXX) $(CXXFLAGS) -o $@ $^
	
djumbai-clean: $(OBJS_djumbai-clean)
	$(CXX) $(CXXFLAGS) -o $@ $^
	
# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean target
clean:
	rm -f $(PROGRAMS) $(OBJS_djumbai-inject) $(OBJS_djumbai-queue) $(OBJS_djumbai-send) $(OBJS_djumbai-clean) $(DIRECTORIES) $(PIPES)
