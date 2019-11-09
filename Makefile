# GNU Makefile

CC = mpicc
CCFLAGS = -Wall
#CCFLAGS =
LDFLAGS =
#LDFLAGS = -llmpe -lmpe
TARGET = mpiqueen

#%.o: %.c
#	$(CC) $(CCFLAGS) -c $<
#
#%: %.o
#	$(CC) $^ -o $@

all: $(TARGET)

clean:
	rm -f *.o *~ $(TARGET)
