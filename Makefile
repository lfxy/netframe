.PHONY : all clean
TARGET = netservice

all: $(TARGET)
CXX = g++
RM := rm
VPATH =. ./src
INCLUDE_PATH:=./
LIBRARY_PATH:=./lib
#CXXPPFLAGS := -D__RELEASE_VERSION__
CXXFLAGS = -Wall -O2 -I$(INCLUDE_PATH)
LIBDIRS := -L$(LIBRARY_PATH)/
CXXLDFLAGS = -lpthread
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

