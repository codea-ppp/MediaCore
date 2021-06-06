.PHONY : all clean

all : loadbalance.out 

loadbalance.out : loadbalance.o main.o
	g++ $(common_gcc_flags) -o $@ *.o $(common_out_links) $(common_server_links) $(foreach subdir, $(subdirs_absolutely), -L$(subdir))

%.o : %.cpp
	g++ $(common_gcc_flags) -o $@ -c $< $(foreach subdir, $(subdirs_absolutely), -I$(subdir))

clean :
	rm -rf *.out *.o
