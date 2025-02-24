all: clean compile link run

link:
	g++ -o bin/warehouse bin/main.o bin/WareHouse.o bin/Customer.o bin/Order.o bin/Action.o bin/Volunteer.o

compile:
	g++ -g -Wall -Weffc++ -c -o bin/main.o src/main.cpp
	g++ -g -Wall -Weffc++ -c -o bin/WareHouse.o src/WareHouse.cpp
	g++ -g -Wall -Weffc++ -c -o bin/Customer.o src/Customer.cpp
	g++ -g -Wall -Weffc++ -c -o bin/Volunteer.o src/Volunteer.cpp
	g++ -g -Wall -Weffc++ -c -o bin/Order.o src/Order.cpp
	g++ -g -Wall -Weffc++ -c -o bin/Action.o src/Action.cpp

clean:
	rm -f bin/*

run:
	./bin/warehouse configFileExample.txt
