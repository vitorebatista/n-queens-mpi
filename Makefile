# GNU Makefile

CC = mpicc
CCFLAGS = -Wall -O3
#CCFLAGS =
LDFLAGS =
#LDFLAGS = -llmpe -lmpe
TARGET = mpi-nqueens seq-nqueens help

#%.o: %.c
#	$(CC) $(CCFLAGS) -c $<
#
#%: %.o
#	$(CC) $^ -o $@

all: $(TARGET) -d

help:
	@echo
	@echo
	@echo "####### Exemplo de Execução sequencial #######"
	@echo "./seq-nqueens"
	@echo
	@echo "####### Exemplo de Execução MPI #######"
	@echo "mpirun -np 3 -mca btl ^openib  -mca orte_base_help_aggregate 0 ./mpi-nqueens"
	
clean:
	rm -f *.o *~ $(TARGET)

run:
	mpirun -np 8 --hostfile mp -mca btl ^openib  -mca orte_base_help_aggregate 0 ./mpiqueen	16
