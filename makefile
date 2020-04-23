source  := $(wildcard *.cpp)
obj		:= $(patsubst %.c,%.o, $(patsubst %.cpp,%.c, $(source)))

CC	 	:= g++
LIBS 	:= -lpthread
INCLUDE := 
CXXFLAGS:= -std=c++11 -g -Wall -o3 $(INCLUDE)

all : $(obj)

main.o : main.cpp 
	$(CC) -c main.cpp $(CXXFLAGS) $(LIBS)
epoll.o : epoll.cpp epoll.h
	$(CC) -c epoll.cpp $(CXXFLAGS) $(LIBS)
util.o : util.h util.cpp
	$(CC) -c util.cpp $(CXXFLAGS) $(LIBS)

#$(obj): $(source)

.PHONY : clean
clean :
	-rm -f $(obj)
