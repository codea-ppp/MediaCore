

NetListenerTest.out : NetListenerTest.o
	g++ -Wall -o $@ $< -lzlog -lNetMessageListener -L../Common/ -L../Common/DetailMessageFormat

%.o : %.cpp
	g++ -Wall -g3 -pthread -o $@ -c $< -I../Common/ -I../Common/DetailMessageFormat