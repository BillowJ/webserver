source  := $(wildcard *.cpp)
obj		:= $(patsubst %.c,%.o, $(patsubst %.cpp,%.c, $(source)))

CC	 	:= g++
LIBS 	:= -lpthread
INCLUDE := 
CXXFLAGS:= -std=c++11 -g -Wall -o3 $(INCLUDE)

test: $(obj)
	$(CC) $(source) $(CXXFLAGS) -o $@ $(obj) $(LIBS)

#$(obj): $(source)
#	$(CC) $(CXXFLAGS) -MMH -c $< -o $@

.PHONY : clean
clean :
	-rm -f $(obj)
