.PHONY : all clean

all : net_listener_server.out net_listener_client.out 

net_listener_server.out : net_listener_server.o
	g++ -Wall -g3 -pthread -o $@ $< -lzlog -lboost_thread -lnet_message_listener -lmessage $(foreach subdir, $(subdirs_absolutely), -L$(subdir))

net_listener_client.out : net_listener_client.o
	g++ -Wall -g3 -pthread -o $@ $< -lzlog -lboost_thread -lnet_message_listener -lmessage $(foreach subdir, $(subdirs_absolutely), -L$(subdir))

%.o : %.cpp
	g++ -Wall -g3 -pthread -o $@ -c $< $(foreach subdir, $(subdirs_absolutely), -I$(subdir))

clean :
	rm -rf *.out *.o