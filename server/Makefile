# author: lovoski

CC=gcc
CXX=g++

TARGET=server
C_SRC=access_sql.c server.c str_utils.c
C_OBJ=access_sql.o server.o str_utils.o

IC_CMD=-I /usr/include/mysql -I . 

# the params `-L. -lmysqlclient` tells the compiler to
# search for library at current diretory, and link the lib
# named `mysqlclient`
# 
# the params `-WL,-rpath=<path-to-lib>` tells the compiler
# to find shared library at the destinated path
LK_CMD=-L. -lmysqlclient -Wl,-rpath=./lib/

# `-Wall` means show all warnings
all:
	echo "   build c executable"
	$(CC) -Wall -c $(C_SRC) $(IC_CMD)
	$(CC) -Wall $(C_OBJ) $(LK_CMD) -o $(TARGET)

# echo "   build cpp executable"
# $(CXX) client.cpp -o client

clean:
	rm -rf *.o $(TARGET) client