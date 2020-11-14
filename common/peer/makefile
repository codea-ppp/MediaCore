headers = $(shell fd -t f '.*\.h')
sources = $(shell fd -t f '.*\.cpp')
objs	= $(patsubst %.cpp, %.o, $(sources))

.PHONY: all clean

all: $(objs)

clean:
	rm -rf *.so *.o

$(target): $(objs) $(headers)
	g++ -shared $(common_gcc_flags) -o $@ $(objs)

%.o : %.cpp %.h
	g++ -fPIC $(common_gcc_flags) -o $@ -c $< $(foreach subdir, $(subdirs_absolutely), -I$(subdir))