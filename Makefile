.PHONY : all clean
TARGET = netservice

all: $(TARGET)
CXX = g++
RM := rm
VPATH =. ./src
#CXXPPFLAGS := -D__RELEASE_VERSION__
CXXFLAGS = -Wall -g -I ./ -I ./include/
LIBDIRS := -L ./lib
CXXLDFLAGS = -lpthread -lrt
DIRS :=. ./src
FILES = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
OBJS = $(patsubst %.cpp, %.o,$(FILES))

$(TARGET):$(OBJS)
	@echo $(FILES)
	@echo $(OBJS)
	$(CXX) -o $@ $(LIBDIRS) $^ $(CXXLDFLAGS) 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	-@$(RM) $(TARGET) $(OBJS)

