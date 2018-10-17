#include <stdlib.h>
#include <stdio.h>
#include "MPIe.h"
#include "tasks_comm.h"

#define NUM_NODES                  16		//16 for small input; 160 for large input; 30 for medium input;
#define MAXPROCESSORS			   64		//The amount of processor
#define NPROC 						3

int PROCESSORS;
int pthread_n_workers;
int paths;
int tasks[MAXPROCESSORS][2];
int nodes_tasks[NUM_NODES*(NUM_NODES-1)/2][2];
int AdjMatrix[NUM_NODES][NUM_NODES];
int *result;
int ended = 0;

void startThreads(void) {
	int i, j;
	Message msg;

	/* SEND nodes_tasks[NUM_NODES*(NUM_NODES-1)/2][2] */
	msg.length = NUM_NODES*(NUM_NODES-1)/2;
	// Send X of nodes_tasks
	for (i=0; i<(NUM_NODES*(NUM_NODES-1)/2); i++)
		msg.msg[i] = nodes_tasks[i][0];
	Send(msg, dijkstra_0);
	Send(msg, dijkstra_1);
	Send(msg, dijkstra_2);

	// Send Y of nodes_tasks
	for (i=0; i<(NUM_NODES*(NUM_NODES-1)/2); i++)
		msg.msg[i] = nodes_tasks[i][1];
	Send(msg, dijkstra_0);
	Send(msg, dijkstra_1);
	Send(msg, dijkstra_2);

	/* SEND tasks[MAXPROCESSORS][2] */
	msg.length = MAXPROCESSORS;
	// Send X of tasks
	for (i=0; i<MAXPROCESSORS; i++)
		msg.msg[i] = tasks[i][0];
	Send(msg, dijkstra_0);
	Send(msg, dijkstra_1);
	Send(msg, dijkstra_2);

	// Send Y of tasks
	for (i=0; i<MAXPROCESSORS; i++)
		msg.msg[i] = tasks[i][1];	Send(msg, dijkstra_0);
	Send(msg, dijkstra_1);
	Send(msg, dijkstra_2);

	/* SEND AdjMatrix[NUM_NODES][NUM_NODES] */
	msg.length = NUM_NODES;
	for (i=0; i<NUM_NODES; i++) {
		//Echo(" SEND: ");
		for (j=0; j<NUM_NODES; j++) {
			msg.msg[j] = AdjMatrix[j][i];
			//Echo(itoa(msg.msg[j]));
			//Echo(" ");
		}
		Send(msg, dijkstra_0);
		Send(msg, dijkstra_1);
		Send(msg, dijkstra_2);
		//Echo("\n");
	}
}

void divide_task_group(int task) {
	int i;
	for(i=0;i<PROCESSORS;i++){
		tasks[i][0] = task/PROCESSORS* (i);
		tasks[i][1] = task/PROCESSORS* (i+1) + (i+1==PROCESSORS&task%PROCESSORS!=0?task%PROCESSORS:0);
	}
}

void ProcessMessage() {
	int paths = 0;

	if (result[0] == -1) {
		ended++;
		printf("\nENDED %d", ended);
		//Echo(itoa(GetTick()));
		//Echo("Divider finished.");
	}
	else {
		printf("From %d to %d, shortest path is %d in cost. Path is: %d", result[0]+1, result[1]+1, result[2], result[0]+1);
		//Echo(itoa(result[0]+1));
		//Echo(" to ");
		//Echo(itoa(result[1]+1));
		//Echo(", shortest path is ");
		//Echo(itoa(result[2]));
		//Echo(" in cost. ");
		//Echo("Path is: ");
		//Echo(itoa(result[0]+1));
		for(paths=3; paths<33; paths++) {
			if (result[paths] > 0) {
				printf(" %d", result[paths]);
			}
		}
		printf("\n");
	}
}

int main0(int argc, char *argv[]) {
	int i=0,j=0,k=0;

	int fpTrix[NUM_NODES*NUM_NODES] = { 0,    6,    3,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										6,    0,    2,    5,    9999, 9999, 1,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										3,    2,    0,    3,    4,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										9999, 5,    3,    0,    2,    3,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										9999, 9999, 4,    2,    0,    5,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										9999, 9999, 9999, 3,    5,    0,    3,    2,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										9999, 1,    9999, 9999, 9999, 3,    0,    4,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										9999, 9999, 9999, 9999, 9999, 2,    4,    0,    7,    9999, 9999, 9999, 9999, 9999, 9999, 9999,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 7,    0,    5,    1,    9999, 9999, 9999, 9999, 9999,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 5,    0,    9999, 3,    9999, 9999, 9999, 9999,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 1,    9999, 0,    9999, 4,    9999, 9999, 8,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 3,    9999, 0,    9999, 2,    9999, 9999,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 4,    9999, 0,    1,    9999, 2,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 2,    1,    0,    6,    9999,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 6,    0,    3,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 8,    9999, 2,    9999, 3,    0 };

	/* Step 1: geting the working vertexs and assigning values */
	for (i=0;i<NUM_NODES;i++) {
		//Echo(" MATRIX: ");
		for (j=0;j<NUM_NODES;j++) {
			AdjMatrix[i][j]= fpTrix[k];
			//Echo(itoa(AdjMatrix[i][j]));
			//Echo(" ");
			k++;
		}
		//Echo("\n");
	}

	int tasks = NUM_NODES*(NUM_NODES-1)/2;

	int x=0;
	for(i=0;i<NUM_NODES-1;i++){ //small:15; large:159
		for(j=i+1;j<NUM_NODES;j++){	//small:16; large:160
			nodes_tasks[x][0] = i;
			nodes_tasks[x][1] = j;
			x++;
		}
	}

	divide_task_group(tasks);
	startThreads();

	//Echo("\n");
	Message msg;
	msg.length = 33;
	while (1) {
		msg = Receive();
		for (k=0; k<33; k++)
			result[k] = msg.msg[k];
		ProcessMessage();
		msg = Receive();
		for (k=0; k<33; k++)
			result[k] = msg.msg[k];
		ProcessMessage();
		msg = Receive();
		for (k=0; k<33; k++)
			result[k] = msg.msg[k];
		ProcessMessage();

		if (ended == (NPROC)) {
			return 0;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
    MemoryWrite(INITIALIZE_ROUTER, 2);
	int m_argc,size,i;
	char *m_argv[3];

	pthread_n_workers = NPROC;

	PROCESSORS = pthread_n_workers;

	m_argc = 2;
	m_argv[1] = "input_small.dat";

	main0(m_argc,m_argv);

    MemoryWrite(END_SIM, 2);
    return 0;
}
