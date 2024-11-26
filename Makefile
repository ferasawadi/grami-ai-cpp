CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Target executable name
TARGET = ai_agent

# Source files
SRCS = main.cpp AIAgent.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Library paths (adjust if needed)
CPPREST_INCLUDE = /usr/local/include
CPPREST_LIB = /usr/local/lib

# Libraries
LIBS = -lcpr -lssl -lcrypto

# Include paths
INCLUDES = -I/usr/local/include -I/usr/local/include/nlohmann

# Default target
all: $(TARGET)

# Linking the target executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

# Compiling source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

# Clean up
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
