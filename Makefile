SHELL = /bin/sh

# Executable name
EXENAME = single_pion_prod_cmp

# Source and object files
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)

# Output directory
BIN = bin

# Include GENIE machine-specific flags and variables (e.g. INCLUDES, CXX, CXXFLAGS, LD, LDFLAGS)
include $(GENIE_COMPARISONS)/src/make/Make.include

# GENIE and GENIE Comparisons libraries
GENIE_GENERATOR_LIBS   = $(shell $(GENIE)/src/scripts/setup/genie-config --libs)
GENIE_COMPARISONS_LIBS = $(shell $(GENIE_COMPARISONS)/src/scripts/setup/genie-comparisons-config --libs)

# Append to LIBRARIES (important: must come after Make.include sets it)
LIBRARIES := $(GENIE_GENERATOR_LIBS) $(GENIE_COMPARISONS_LIBS) $(LIBRARIES)

# Ensure bin exists
$(BIN):
	mkdir -p $(BIN)

# Default target
all: $(BIN)/$(EXENAME)

# Compile each .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Link all .o into the executable
$(BIN)/$(EXENAME): $(OBJ) | $(BIN)
	$(LD) $(LDFLAGS) $(OBJ) $(LIBRARIES) -o $@

# Clean rule
clean:
	rm -f src/*.o $(BIN)/$(EXENAME)

# Force rule
FORCE:
