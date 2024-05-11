# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra

DIRECTORIES = ../queue/mess/* ../queue/pid/* ../queue/local/* ../queue/todo/* ../queue/intd/* ../queue/info/*

PIPES = /tmp/clean_pipe0 /tmp/clean_pipe1 /tmp/spawn_pipe0 /tmp/spawn_pipe1 /tmp/local_pipe0 /tmp/local_pipe1

# Source files for each program
PROGRAMS = djumbai-inject djumbai-queue djumbai-send djumbai-clean djumbai-lspawn djumbai-local djumbai-check djumbai-groups djumbai-group-manager djumbai-start
SRCS_djumbai-inject = djumbai-inject.cpp
SRCS_djumbai-queue = djumbai-queue.cpp
SRCS_djumbai-send = djumbai-send.cpp
SRCS_djumbai-clean = djumbai-clean.cpp
SRCS_djumbai-lspawn = djumbai-lspawn.cpp
SRCS_djumbai-local = djumbai-local.cpp
SRCS_djumbai-check = djumbai-check.cpp
SRCS_djumbai-groups = djumbai-groups.cpp
SRCS_djumbai-group-manager = djumbai-group-manager.cpp
SRCS_djumbai-start = ../boot/djumbai-start.cpp
# SRCS_djumbai-stop = ../boot/djumbai-stop.cpp


# Object files for each program
OBJS_djumbai-inject = $(SRCS_djumbai-inject:.cpp=.o)
OBJS_djumbai-queue = $(SRCS_djumbai-queue:.cpp=.o)
OBJS_djumbai-send = $(SRCS_djumbai-send:.cpp=.o)
OBJS_djumbai-clean = $(SRCS_djumbai-clean:.cpp=.o)
OBJS_djumbai-lspawn = $(SRCS_djumbai-lspawn:.cpp=.o)
OBJS_djumbai-local = $(SRCS_djumbai-local:.cpp=.o)
OBJS_djumbai-check = $(SRCS_djumbai-check:.cpp=.o)
OBJS_djumbai-groups = $(SRCS_djumbai-groups:.cpp=.o)
OBJS_djumbai-group-manager = $(SRCS_djumbai-group-manager:.cpp=.o)
OBJS_djumbai-start = $(SRCS_djumbai-start:.cpp=.o)
# OBJS_djumbai-stop = $(SRCS_djumbai-stop:.cpp=.o)


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

djumbai-lspawn: $(OBJS_djumbai-lspawn)
	$(CXX) $(CXXFLAGS) -o $@ $^
	
djumbai-local: $(OBJS_djumbai-local)
	$(CXX) $(CXXFLAGS) -o $@ $^
	
djumbai-check: $(OBJS_djumbai-check)
	$(CXX) $(CXXFLAGS) -o $@ $^

djumbai-groups: $(OBJS_djumbai-groups)
	$(CXX) $(CXXFLAGS) -o $@ $^

djumbai-group-manager: $(OBJS_djumbai-group-manager)
	$(CXX) $(CXXFLAGS) -o $@ $^

djumbai-start: $(OBJS_djumbai-start)
	$(CXX) $(CXXFLAGS) -o $@ $^

# djumbai-stop: $(OBJS_djumbai-stop)
# 	$(CXX) $(CXXFLAGS) -o $@ $^
	
# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean target
clean:
	rm -f $(PROGRAMS) $(OBJS_djumbai-inject) $(OBJS_djumbai-queue) $(OBJS_djumbai-send) $(OBJS_djumbai-clean) $(OBJS_djumbai-lspawn) $(OBJS_djumbai-local) $(OBJS_djumbai-check) $(OBJS_djumbai-groups) $(OBJS_djumbai-group-manager) $(OBJS_djumbai-start) $(DIRECTORIES) $(PIPES)
