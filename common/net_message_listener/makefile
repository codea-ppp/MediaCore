headers = $(shell fd -t f '.*\.h')
sources = $(shell fd -t f '.*\.cpp')
objs	= $(patsubst %.cpp, %.o, $(sources))
target	= libnet_message_listener.so

.PHONY: all clean

all: $(target)

clean:
	rm -rf *.so *.o

$(target): $(objs) $(headers)
	g++ -shared $(common_gcc_flags) -o $@ $(objs) ../connection/*.o

%.o : %.cpp %.h
	g++ -fPIC $(common_gcc_flags) -o $@ -c $< $(foreach subdir, $(subdirs_absolutely), -I$(subdir))
