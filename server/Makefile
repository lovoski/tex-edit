CC=gcc
CPP=g++
C_FILES=test_server.c \
		threadpool.c \
		str_utils.c \
		access_sql.c
C_OBJ=test_server.o \
		threadpool.o \
		str_utils.o \
		access_sql.o
TARGET=test_server

all:
	rm -rf *.o test_server
	$(CC) -c $(C_FILES) -I. -I /usr/include/mysql
	$(CC) $(C_OBJ) -o $(TARGET) -lpthread -lmysqlclient

clean:
	rm -rf *.o test_server