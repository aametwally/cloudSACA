EXECS     = FAK

OPT         = -O2
CFLAGS      = $(OPT)
CC          = mpic++

OBJS	    =  helpers.o main.o stringSorting.o bucketSorting.o bucketConstruction.o
$(EXECS): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) 

.SUFFIXES: .cpp

.cpp.o : 
	$(CC) $(CFLAGS) -c $< 

.PHONY: clean

clean:
	rm -f $(OBJS) $(EXECS) *.o
