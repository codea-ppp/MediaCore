export common_gcc_flags = -g3 -Wall -pthread
export common_out_links = -lzlog -lboost_thread -lnet_message_listener -lmessage -lserver

pwd = $(shell pwd)
subdirs = $(shell fd -t d)
export subdirs_absolutely = $(foreach subdir, $(subdirs), $(pwd)/$(subdir) )

dis2compile = \
	$(pwd)/common/detail_message_format \
	$(pwd)/common/connection \
	$(pwd)/common/net_message_listener \
	$(pwd)/common/peer \
	$(pwd)/common/media_chain \
	$(pwd)/common/server \
	$(pwd)/loadbalance \
	$(pwd)/test

.PHONY: all clean env

all:
	@ $(foreach dir, $(dis2compile), $(MAKE) -C $(dir);)
	
env:
	@ echo common_gcc_flags: $(common_gcc_flags)
	@ echo $(subdirs_absolutely)

clean:
	@ $(foreach dir, $(dis2compile), $(MAKE) -C $(dir) clean;)