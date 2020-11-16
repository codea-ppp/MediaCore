headers = $(shell fd -t f '.*\.h')
sources = $(shell fd -t f '.*\.cpp')
objs	= $(patsubst %.cpp, %.o, $(sources))

.PHONY: all clean

all: $(objs)

clean:
	rm -rf *.so *.o

%.o : %.cpp %.h
	g++ -fPIC $(common_gcc_flags) -o $@ -c $< $(foreach subdir, $(subdirs_absolutely), -I$(subdir))