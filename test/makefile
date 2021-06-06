source = $(shell ls *.cpp)
target = $(patsubst %.cpp, %.out, $(source))

.PHONY : all clean

all : $(target)

%.out : %.cpp
	g++ $(common_gcc_flags) -o $@ $< $(common_out_links) $(stream_pusher_links) $(stream_render_links) $(foreach subdir, $(subdirs_absolutely), -L$(subdir)) $(foreach subdir, $(subdirs_absolutely), -I$(subdir))

clean :
	rm -rf *.out *.o