EXECS     = KS_Merge

OPT         = -O2
# OPT       = -g
CFLAGS      = $(OPT)
CC          = mpic++

#OBJS	    = psrs.o mpi-printf.o seq_sort.o main.o
OBJS	    = main.o pDC3.o utilities.o psrs.o NameingConstructMapper.o helpers.o readFile.o seq_sort.o tuplesConst.o
$(EXECS): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) 

.SUFFIXES: .cpp

.cpp.o : 
	$(CC) $(CFLAGS) -c $< 

.PHONY: clean

clean:
	rm -f $(OBJS) $(EXECS) *.o
