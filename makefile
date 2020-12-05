export common_gcc_flags 	= -O3 -Wall -pthread
export common_out_links 	= -lzlog -lboost_thread -lnet_message_listener -lmessage
export common_server_links 	= -lserver -lability -ljsoncpp
export stream_pusher_links 	= -lstream_pusher -lavutil -lavcodec -lavformat -lavdevice -lavfilter -lswscale	
export stream_render_links 	= -lstream_render -lSDL2

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
	$(pwd)/common/ability \
	$(pwd)/loadbalance \
	$(pwd)/resource/stream_pusher \
	$(pwd)/resource \
	$(pwd)/client_demo/stream_render \
	$(pwd)/client_demo/client_cli \
	$(pwd)/test

.PHONY: all clean env

all:
	@ $(foreach dir, $(dis2compile), $(MAKE) -C $(dir);)
	
env:
	@ echo common_gcc_flags: $(common_gcc_flags)
	@ echo $(subdirs_absolutely)

clean:
	@ $(foreach dir, $(dis2compile), $(MAKE) -C $(dir) clean;)
