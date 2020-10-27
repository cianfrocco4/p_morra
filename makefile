SOURCE_FILE=src/p_morra.cpp #mpi_hello_world.c
OUTPUT_FILE= p_morra #mpi_hello_world
NUM_SERVERS=4
MPICC?=mpic++
NUM_PLAYERS=10

all: p_morra

p_morra: ${SOURCE_FILE}
	${MPICC} -Iinc ${SOURCE_FILE} -o ${OUTPUT_FILE}

clean:
	rm -f ${OUTPUT_FILE}

run_local:
	mpiexec -n ${NUM_SERVERS} ./${OUTPUT_FILE} ${NUM_PLAYERS}

run_cluster:
	mpiexec --hostfile ./hostfile -n ${NUM_SERVERS} ./${OUTPUT_FILE} ${NUM_PLAYERS}

run_round_robin:
	mpiexec --map-by node:SPAN --hostfile ./hostfile -n ${NUM_SERVERS} ./${OUTPUT_FILE} ${NUM_PLAYERS}




