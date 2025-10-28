all: echo
	g++ -o main src/main.cpp -larchive -lcurl -g
echo: src/echo.cpp
	g++ -o echo src/echo.cpp --static
