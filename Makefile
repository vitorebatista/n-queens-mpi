# GNU Makefile

CC = mpicc
CCFLAGS = -Wall
#CCFLAGS =
LDFLAGS =
#LDFLAGS = -llmpe -lmpe
TARGET = mpiqueen help optqueen 

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
	@echo "mpirun -np 3 -mca btl ^openib  -mca orte_base_help_aggregate 0 ./mpiqueen"

clean:
	rm -f *.o *~ $(TARGET)
