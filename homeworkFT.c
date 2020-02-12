/*GHERGU MIHAI ADRIAN  331CB*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <complex.h>

int N, numThreads;
double *inValues;
complex double *result;
char *inFile, *outFile;
FILE *in, *out;

void getArgs(int argc, char **argv){
	if (argc < 3) {
	    fprintf(stdout, "Usage: %s <inputValues> <outputValues> <numThreads>\n", argv[0]);
	    exit(1);
	}
	strcpy(inFile, argv[1]);
	strcpy(outFile, argv[2]);
	numThreads = atoi(argv[3]);
}

void init(){
	inFile = (char*)malloc(sizeof(char*) * 20);
	outFile = (char*)malloc(sizeof(char*) * 20);
		if(inFile == NULL || outFile == NULL ){
			printf("malloc failed!\n");
			exit(1);
		}
}
/*Citirea din fisier*/
void readFile(FILE *file){
	int i, retVal;
	size_t maximumLineLength = 500;
	char *line = (char*)malloc(sizeof(char*) * maximumLineLength);
	if(line == NULL){
		printf("malloc failed!\n");
		exit(1);
	}
	file = fopen(inFile,"r");
		if(file == NULL)
			return ;

	retVal = getline(&line, &maximumLineLength, file);
		if (retVal == 0)
			return ;

	N = atoi(line);
	/* inValues, vectorul in care salvez valorile din fisierul de input */
	inValues = malloc(sizeof(double*) * N);
	/*in result pun rezultatul pe care trebuie sa il printez*/
	result = malloc( sizeof(complex double) * N);
	if(inValues == NULL || result == NULL){
		printf("malloc failed!\n");
		exit(1);
	}
	/*citirea linie cu linie*/
		for(i = 0; i < N; i++){
			retVal = getline(&line, &maximumLineLength, file);
			if (retVal == 0)
				return ;
			inValues[i] = atof(line);
		}
	fclose(file);
}
/*scrierea in fisierul de outpu*/
void writeFile(FILE *file){
	int i;
	file = fopen(outFile, "w");
	fprintf(file, "%d\n", N);
		for (i = 0; i < N; i++)
			fprintf(file, "%lf %lf\n", creal(result[i]), cimag(result[i]));				
}
/* In cadrul acestei functii calculez indicii intre care calculeaza fiecare
thread in parte(start, end). apoi calculez efectiv rezultatul dupa formula data*/
void* threadFunction(void *var){
	int thread_id = *(int*)var;

	int  start, end, i, j;
	double angle;
	complex double sum;
	start = N * thread_id / numThreads;
	end = N * (thread_id + 1) / numThreads;

	for (i = start; i < end; i++){
			sum = 0.f;
		for (j = 0; j < N; j++) { 
			angle = -2 * M_PI * i * j / N;
			sum += inValues[j] * cexp(angle * I);
		}
	result[i] = sum;
	}
	return NULL;
}

int main(int argc, char * argv[]){
	init();
	getArgs(argc, argv);
	readFile(in);

	int i;
	pthread_t tid[numThreads];
	int thread_id[numThreads];

	for(i = 0; i < numThreads; i++)
		thread_id[i] = i;

	for(i = 0; i < numThreads; i++)
		pthread_create(&(tid[i]), NULL, threadFunction, &(thread_id[i]));
	
	for(i = 0; i < numThreads; i++) 
		pthread_join(tid[i], NULL);
	
	writeFile(out);

	return 0;
}