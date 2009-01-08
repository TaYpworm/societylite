SOURCE = servermain.cpp server_listen.c manage_server.c socket_accept.c servertype.o connection.o reportinfo.o singlestream.o

#FLAGS = -DQQ_ENABLE
#CC = /opt/mpich/gnu/bin/mpiCC
CC = c++

all: $(SOURCE)
	c++ $(SOURCE) -o server

debug: $(SOURCE)
	g++ -g $(SOURCE) -o serverDebugger

profile: $(SOURCE) QQ.o dprintf.o
	$(CC) $(SOURCE) -DQQ_ENABLE QQ.o dprintf.o -o pServer

QQ.o: QQ.cpp QQ.h
	$(CC) -c -o QQ.o -DQQ_ENABLE QQ.cpp

.cpp.o:
	$(CC) -c $< $(FLAGS)

.c.o:
	$(CC) -c $< $(FLAGS)

run:
	/opt/mpich/gnu/bin/mpirun -np 1 -machinefile ~/mach.mpi pServer empty.txt 20000 v

clean:
	rm *.o server serverDebugger

