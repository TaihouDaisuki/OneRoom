CC = g++
MAKE = make
RM = rm -f
RMRF = rm -rf
FLAGS = -Wall
CPP11FLAGS = -std=c++11
MYSQLFLAGS = $$(mysql_config --cflags --libs)

TARGET = server
SRC = main.cpp server.cpp tinyxml2.cpp
H = server.h tinyxml2.h

all: $(TARGET)

# $@-target
# $^-all prerequisites
# $<-first prerequisite

$(TARGET): $(SRC) $(H)
	$(CC) $(FLAGS) $(CPP11FLAGS) $(MYSQLFLAGS) -o $@ $(SRC)

.PHONY: clean
	#avoid to make clean.*

clean:
	$(RM) $(TARGET) *.o
