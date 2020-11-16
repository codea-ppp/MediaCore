.PHONY : all clean

all : net_listener_server.out net_listener_client.out 

net_listener_server.out : net_listener_server.o
	g++ $(common_gcc_flags) -o $@ $< -lzlog -lboost_thread -lnet_message_listener -lmessage $(foreach subdir, $(subdirs_absolutely), -L$(subdir))

net_listener_client.out : net_listener_client.o
	g++ $(common_gcc_flags) -o $@ $< -lzlog -lboost_thread -lnet_message_listener -lmessage $(foreach subdir, $(subdirs_absolutely), -L$(subdir))

%.o : %.cpp
	g++ $(common_gcc_flags) -o $@ -c $< $(foreach subdir, $(subdirs_absolutely), -I$(subdir))

clean :
	rm -rf *.out *.o