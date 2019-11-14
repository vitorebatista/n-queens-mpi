# GNU Makefile

CC = mpicc
CCFLAGS = -Wall
#CCFLAGS =
LDFLAGS =
#LDFLAGS = -llmpe -lmpe
TARGET = nqueens

#%.o: %.c
#	$(CC) $(CCFLAGS) -c $<
#
#%: %.o
#	$(CC) $^ -o $@

all: $(TARGET)

help:
	@echo
	@echo
	@echo "####### Exemplo de Execução #######"
	@echo "./nqueens"
	
	
clean:
	rm -f *.o *~ $(TARGET)

run:
	./nqueens
