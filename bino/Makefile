# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra

# Source files for each program
PROGRAMS = djumbai-inject djumbai-queue
SRCS_djumbai-inject = djumbai-inject.cpp
SRCS_djumbai-queue = djumbai-queue.cpp


# Object files for each program
OBJS_djumbai-inject = $(SRCS_djumbai-inject:.cpp=.o)
OBJS_djumbai-queue = $(SRCS_djumbai-queue:.cpp=.o)


# Build targets for each program
all: $(PROGRAMS)

djumbai-inject: $(OBJS_djumbai-inject)
	$(CXX) $(CXXFLAGS) -o $@ $^

djumbai-queue: $(OBJS_djumbai-queue)
	$(CXX) $(CXXFLAGS) -o $@ $^
	
# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean target
clean:
	rm -f $(PROGRAMS) $(OBJS_djumbai-inject) $(OBJS_djumbai-queue) 
#$(OBJS_program2) $(OBJS_program3)
