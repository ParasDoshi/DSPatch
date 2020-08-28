SOURCE	:= Main.cpp Trace.cpp Cache.cpp DSPatch.cpp
CC	:= g++
TARGET	:= Main
all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) -std=c++11 -g -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)
