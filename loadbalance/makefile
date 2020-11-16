.PHONY : all clean

all : loadbalance.out 

loadbalance.out : loadbalance.o main.o
	g++ $(common_gcc_flags) -o $@ main.o loadbalance.o $(common_out_links) $(foreach subdir, $(subdirs_absolutely), -L$(subdir))

%.o : %.cpp %.h
	g++ $(common_gcc_flags) -o $@ -c $< $(foreach subdir, $(subdirs_absolutely), -I$(subdir))

clean :
	rm -rf *.out *.o
