.PHONY : all clean
TARGET = myprocess


all: $(TARGET)
CXX = g++
RM := rm
VPATH =.
CXXPPFLAGS :=
CXXFLAGS = -Wall -O2
CXXLDFLAGS = -lpthread
LIBDIRS :=

DIRS :=.
FILES = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
OBJS = $(patsubst %.cpp, %.o, $(FILES))

$(TARGET):$(OBJS)
	$(CXX) -o $@ $(LIBDIRS) $^ $(CXXLDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	-@$(RM) $(TARGET) $(OBJS)
