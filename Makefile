all:
	g++ -o example/NeoSPI_Standarttest example/main.cpp src/NeoSPI.cpp -std=c++20 -Isrc

clean:
	rm example/NeoSPI_Standarttest
