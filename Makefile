CFLAGS=-g --std=c++11 #-m64
INCLUDES=-I. -I./include
LDFLAGS=-m64
LIBS=
PROGNAME=BoggleSolver

all : $(PROGNAME)

run: all
	./$(PROGNAME)

time: all
	time ./$(PROGNAME)

debug: all
	gdb ./$(PROGNAME)

$(PROGNAME) : $(PROGNAME).o
	g++ $(LDFLAGS) -o $@ $< $(LIBS)

clean :
	-rm *.o $(PROGNAME) $(PROGNAME).exe

%.o : %.cpp
	g++ -c $(CFLAGS) $(INCLUDES) -o $@ $<
