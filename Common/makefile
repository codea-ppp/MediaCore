libNetMessageListener.so : NetMessageListener.o
	g++ -shared -o $@ $<

#libNetMessageListener.a : NetMessageListener.o
#	ar -crs $@ $<

%.o : %.cpp %.h
	g++ -fPIC -pthread -g3 -Wall -o $@ -c $< -I . -I ./DetailMessageFormat 