# If called from main makefile, compiles .o files into parse.a, otherwise 
# compiles stand-alone test program

NAME = parse

# you must manually set the LIB var when compiling the test program, and
# comment it out for compiling the main program.  There should be a way to
# set this up as a parameter, but I haven't figured out how yet...

LIB = no

CFLAGS += -D_IS_C

# MPI = /usr/mpich/c
MPI = /usr/local/mpich-1.2.4
CC  =  $(MPI)/bin/mpicc
CPP =  $(MPI)/bin/mpiCC
MPI_INCLUDES = -I$(MPI)/include
MPI_LIBPATH  = -L$(MPI)/lib

# Pentium Pro/Pentium II/Pentium III/Xeon/K6/K7/6x86mx

CFLAGS += -mpentiumpro -malign-functions=2 -malign-loops=2 \
          -malign-jumps=2 -march=i686 -mcpu=i686

# Optimization flags; these are very important

CFLAGS += -O5 -finline-functions -ffast-math -funroll-loops \
          -fexpensive-optimizations


ifeq ($(LIB),yes)

  SRC = $(NAME)(parse.o) $(NAME)(scan.o) $(NAME)(ParseInput.o) $(NAME)(elements.o) \
        $(NAME)(list.o) $(NAME)(connect.o) $(NAME)(SymTree.o) $(NAME)(tree.o)      \
        $(NAME)(ConvertIn.o) $(NAME)(defines.o) $(NAME)(dump.o)   \
        $(NAME)(NameDefine.o) $(NAME)(ReadFile.o) $(NAME)(printerr.o)
else

  SRC = parse.o scan.o ParseInput.o elements.o \
        list.o connect.o SymTree.o tree.o      \
        ConvertIn.o defines.o dump.o   \
        NameDefine.o ReadFile.o printerr.o

  YACC = bison -v -d -y
  LEX  = flex
endif

#---------------------------------------------------------------

TOBJS = maintst.o 


$(NAME).a: $(SRC)
	@echo "parse: compiling lib"
	@echo "parse: flags = " $(CFLAGS)
	$(CC) -c $(CFLAGS) $(INCLUDES)  $(?:.o=.c)
#	ar -rv $@.a $?
	ar -rv parse.a $?


$(NAME): $(TOBJS) $(NAME).a
	@echo "parse: compiling test"
	@echo "parse: flags = " $(CFLAGS)
	$(CC) -o $@ $(TOBJS) $(LFLAGS) $(INCLUDES) $(NAME).a $(LIBS)

clean:
	 rm -f *.o *~ core gmon.out scan.c $(NAME) parse.a parse.c parse.h

maintst.o:   maintst.c options.h
parse.o:     parse.y input.h
scan.o:      scan.c parse.h  input.h
Input.o:     Input.c Input.h options.h  input.h SymTree.h arrays.h dump.h
elements.o:  elements.c  input.h ../defines.h parse.h
list.o:      list.h
connect.o:    input.h
SymTree.o:    input.h SymTree.h
tree.o:      tree.c tree.h
ConvertIn.o:  input.h SymTree.h ../InitStruct.h arrays.h
defines.o :  ../defines.h

parse.h:    parse.c

scan.c:     scan.l
                              
.y.c: Makefile
	  bison  -d -o $@ $<

