CPPFILE=build.c
CC = gcc
MYCXXFLAGS  = -Wall -Wextra -pedantic -O3 -march=native -mtune=native
MYCXXFLAGS  = -ggdb -Wall -Wextra -pedantic -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC 
TARGET = pbwt.x
all: $(TARGET)

$(TARGET): $(CPPFILE)
	$(CC) $(MYCXXFLAGS) $(CXXFLAGS) -o $(TARGET) $(CPPFILE)

clean:
	$(RM) $(TARGET)
