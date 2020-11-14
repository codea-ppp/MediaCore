.PHONY : all clean

all : loadbalance.out 

loadbalance.out : loadbalance.o main.o
	g++ -Wall -g3 -pthread -o $@ main.o loadbalance.o -lzlog -lboost_thread -lnet_message_listener -lmessage $(foreach subdir, $(subdirs_absolutely), -L$(subdir))

%.o : %.cpp
	g++ -Wall -g3 -pthread -o $@ -c $< $(foreach subdir, $(subdirs_absolutely), -I$(subdir))

clean :
	rm -rf *.out *.o
