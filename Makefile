all: echo
	g++ -o main src/main.cpp
echo: src/echo.cpp
	g++ -o echo src/echo.cpp --static