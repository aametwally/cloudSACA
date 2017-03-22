EXECS     = KS_Optimized

OPT         = -O2
CFLAGS      = $(OPT)
CC          = mpic++

OBJS	    = main.o pDC3.o utilities.o psrs.o duplicateChecker.o helpers.o readFile.o seqSort.o tuplesConst.o
$(EXECS): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) 

.SUFFIXES: .cpp

.cpp.o : 
	$(CC) $(CFLAGS) -c $< 

.PHONY: clean

clean:
	rm -f $(OBJS) $(EXECS) *.o
