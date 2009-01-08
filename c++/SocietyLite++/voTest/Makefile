#
#			Preliminary (Very Basic) Makefile for voTest software
#							-- CMT
#
EXEC = server
OBJS = source/voTest.o source/user_input.o  source/tester.o source/reader.o source/writer.o source/loopback.o

CC= 		gcc
STD= 		_GNU_SOURCE
CFLAGS+=	-O0 -g3 -fmessage-length=0 -MMD -MP -MF
WARN=		-Wall

.c.o:
			$(CC) -c $(CFLAGS) -D$(STD) $(WARN) $<

all:		voTest move_obj

voTest: 	$(OBJS)

			@echo "Building voTest Application"
			$(CC) $(LDFLAGS) $(OBJS) -o bin/voTest -lstdc++

move_obj:	
			mv source/*.o obj/

clean:
			-rm -f $(EXEC) *.elf *.gdb source/*.o

clobber:	clean	
			rm -f obj/*.o
