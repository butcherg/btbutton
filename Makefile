CC=g++


btbutton: btbutton.o
	$(CC) -o btbutton btbutton.o -ludev

btbutton.o: btbutton.cpp
	$(CC) -o btbutton.o -c btbutton.cpp

clean:
	rm -rf btbutton btbutton.o
