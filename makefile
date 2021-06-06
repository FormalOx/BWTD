all : Source
	cp ./Source ../Source
	
Source : Source.o
	g++ -g -Wall ./Source.o -o ./Source -I/usr/local/lib -lsfml-graphics -lsfml-window -lsfml-system
	rm ./Source.o
	
Source.o : Source.cpp
	g++ -g -Wall -c ./Source.cpp -I/usr/local/include 

clean :
