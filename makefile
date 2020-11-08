export common_gcc_flags = -g3 -Wall -pthread

pwd = $(shell pwd)
subdirs = $(shell fd -t d)
export subdirs_absolutely = $(foreach subdir, $(subdirs), $(pwd)/$(subdir) )

.PHONY: all clean env

all:
	@ $(foreach subdir, $(subdirs_absolutely), $(MAKE) -C $(subdir);)
	
env:
	@ echo common_gcc_flags: $(common_gcc_flags)
	@ echo $(subdirs_absolutely)

clean:
	@ $(foreach subdir, $(subdirs_absolutely), $(MAKE) -C $(subdir) clean;)