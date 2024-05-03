# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -Wextra

# Source files for each program
PROGRAMS = client# program2 program3
SRCS_client = client.cpp
# SRCS_program2 = program2.cpp
# SRCS_program3 = program3.cpp

# Object files for each program
OBJS_client = $(SRCS_client:.cpp=.o)
# OBJS_program2 = $(SRCS_program2:.cpp=.o)
# OBJS_program3 = $(SRCS_program3:.cpp=.o)

# Build targets for each program
all: $(PROGRAMS)

client: $(OBJS_client)
	$(CXX) $(CXXFLAGS) -o $@ $^

# program2: $(OBJS_program2)
# 	$(CXX) $(CXXFLAGS) -o $@ $^

# program3: $(OBJS_program3)
# 	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean target
clean:
	rm -f $(PROGRAMS) $(OBJS_client) 
#$(OBJS_program2) $(OBJS_program3)
