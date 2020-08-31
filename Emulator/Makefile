CC = g++
CFLAGS = -g -I $(SRCDIR) -std=c++17 
LDFLAGS = -pthread -lncurses -ltinfo

SRCDIR = ./src
OBJDIR = ./bin/obj
TARGET = ./bin/emulator

OBJ = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(wildcard $(SRCDIR)/*.cpp))

$(TARGET) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $< $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(TARGET)

