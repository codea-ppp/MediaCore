
.PHONY : all

all : NetListenerTest.out NetListenerTest2.out 

NetListenerTest.out : NetListenerTest.o
	g++ -Wall -g3 -pthread -o $@ $< -lzlog -lboost_thread -lNetMessageListener -L../Common/ -L../Common/DetailMessageFormat

NetListenerTest2.out : NetListenerTest2.o
	g++ -Wall -g3 -pthread -o $@ $< -lzlog -lboost_thread -lNetMessageListener -L../Common/ -L../Common/DetailMessageFormat

%.o : %.cpp
	g++ -Wall -g3 -pthread -o $@ -c $< -I../Common/ -I../Common/DetailMessageFormat
