CXX      = g++
CTT      = aarch64-linux-gnu-g++
CTTLIBS  = --sysroot=$(HOME)/rpi-build/staging -lncursesw -pthread -lgpiodcxx
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinc -Iinc/common-protocol
LDFLAGS  = -lncurses -pthread

TARGET      = tui_demo
TARGET_PROD = sirius
SOURCES     = $(wildcard src/*.cpp)

.PHONY: all clean run prod

all: $(TARGET)

# Rule for local development build
$(TARGET): $(SOURCES) inc/tui.h
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

run: all
	./$(TARGET)

# Alias for the production build
prod: $(TARGET_PROD)

# Rule for production cross-compilation (builds 'init')
$(TARGET_PROD): $(SOURCES) inc/tui.h
	$(CTT) $(CXXFLAGS) --sysroot=$(HOME)/rpi-build/staging -DUSING_LIBGPIOD -o $@ $(SOURCES) $(CTTLIBS)

clean:
	rm -f $(TARGET) $(TARGET_PROD)