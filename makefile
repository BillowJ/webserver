SOURCE  := $(wildcard *.cpp)
OBJ		:= $(patsubst %.c,%.o, $(patsubst %.cpp,%.c, $(SOURCE)))

TARGET  := myserver
CC	 	:= g++
LIBS 	:= -lpthread
INCLUDE := 
CXXFLAGS:= -std=c++11 -g -Wall -o3 $(INCLUDE)

#all : $(TAGET)

$(TARGET) : $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $(OBJ) $(LIBS)

#main.o : main.cpp 
#	$(CC) -c main.cpp $(CXXFLAGS) $(LIBS)
#epoll.o : epoll.cpp epoll.h
#	$(CC) -c epoll.cpp $(CXXFLAGS) $(LIBS)
#util.o : util.h util.cpp
#	$(CC) -c util.cpp $(CXXFLAGS) $(LIBS)
#RequestData.o : RequestData.cpp Request.h
#	$(CC) -c Request.cpp $(CXXFLAGS) $(LIBS)


#$(obj): $(source)

.PHONY : clean
clean :
	-rm -f $(OBJ)
