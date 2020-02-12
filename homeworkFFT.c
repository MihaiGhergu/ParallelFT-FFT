/*GHERGU MIHAI ADRIAN  331CB*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <complex.h>

struct myStr{
	int step;
	double complex *input;
	double complex *output;	
	int th_id;
};

int N, numThreads;
pthread_t *tid;
complex double *inValues;
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
	tid = malloc(sizeof(pthread_t) * numThreads);
	if(tid == NULL){
		printf("malloc failed!\n");
		exit(1);
	}
}

void init(){
	inFile = (char*)malloc(sizeof(char*) * 20);
	outFile = (char*)malloc(sizeof(char*) * 20);
		if(inFile == NULL || outFile == NULL ){
			printf("malloc failed!\n");
			exit(1);
		}
}
/*citirea efectiva din fisier*/
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
	inValues = malloc(sizeof(complex double) * N);
	result = malloc(sizeof(complex double) * N);
	if(inValues == NULL || result == NULL){
	printf("malloc failed!\n");
	exit(1);
	}
		for(i = 0; i < N; i++){
			retVal = getline(&line, &maximumLineLength, file);
			if (retVal == 0)
				return ;
			inValues[i] = atof(line);
			result[i] = atof(line);
		}
	fclose(file);
}
/*Scrierea in fisier*/
void writeFile(FILE *file){
	int i;
	file = fopen(outFile, "w");
	fprintf(file, "%d\n", N);
		for (i = 0; i < N; i++)
			fprintf(file, "%lf %lf\n", creal(result[i]), cimag(result[i]));					
}

/*Am rescris functia pusa la dispozitie, aducand o la un singur parametru,
prin construirea unei structuri, in care am pus totii parametrii care erau dati
in functia data*/
void* fft(void *arg){
	struct myStr s = *(struct myStr*)arg;
	if( s.step < N){
		struct myStr s1, s2;

		s1.step = 2 * s.step;
		s1.input = s.output;
		s1.output = s.input;

		s2.step = 2 * s.step;
		s2.input = &(s.output[s.step]);
		s2.output = &(s.input[s.step]);

		fft(&s1);
		fft(&s2);

		for(int i = 0; i < N; i += 2 * s.step){
			complex double t = cexp(-I * M_PI * i / N) * s.input[i + s.step];
			s.output[i / 2] = s.input[i] + t;
			s.output[(i + N) / 2] = s.input[i] - t;
		}
	}
	return NULL;
}
/*Functie auxiliara pe care o folosesc pentru cazul cand am 4 threaduri, 
aceasta este la fel ca functia anterioara, doar ca unul din apelurile 
recursive este facut pe thread*/
void* aux_fft(void *arg){
	struct myStr s = *(struct myStr*)arg;
	if( s.step < N){
		struct myStr s1, s2;

		s1.step = 2 * s.step;
		s1.input = s.output;
		s1.output = s.input;

		s2.step = 2 * s.step;
		s2.input = &(s.output[s.step]);
		s2.output = &(s.input[s.step]);
		if(s.th_id == 0){
			pthread_create(&(tid[2]), NULL, fft, &s1);	
			fft(&s2);
			pthread_join(tid[2], NULL);
		} else if(s.th_id == 1){
			pthread_create(&(tid[3]), NULL, fft, &s1);	
			fft(&s2);
			pthread_join(tid[3], NULL);
		}

		for(int i = 0; i < N; i += 2 * s.step){
			complex double t = cexp(-I * M_PI * i / N) * s.input[i + s.step];
			s.output[i / 2] = s.input[i] + t;
			s.output[(i + N) / 2] = s.input[i] - t;
		}
}
return NULL;
}

int main(int argc, char * argv[]){
	init();
	getArgs(argc, argv);
	readFile(in);

	struct myStr elem;
	elem.step = 1;
	elem.input = inValues;
	elem.output = result;
/*pentru cazul cand am un singur thread*/
	if(numThreads == 1){
		pthread_create(&(tid[0]), NULL, fft, &elem);
		pthread_join(tid[0], NULL);
	}
/*Cazul cand am 2 threaduri, le creez si apelez functia fft pe cele 2 thread uri,
iar pentru avand pasul egal cu 2, am pus ca input, rezultatul si ca output, 
voctorul de input pentru ca in recursivitate acestea se inverseaza*/
	else if(numThreads == 2){
		elem.step = 2;
		elem.input = result;
		elem.output = inValues;

		struct myStr elem2;
		elem2.step = 2;
		elem2.input= &result[1];
		elem2.output = &inValues[1];

		pthread_create(&(tid[0]), NULL, fft, &elem);
		pthread_create(&(tid[1]), NULL, fft, &elem2);

		pthread_join(tid[0], NULL);
		pthread_join(tid[1], NULL);

		for( int i = 0; i < N; i += 2){
			double complex t = cexp(-I * M_PI * i / N) * inValues[ i + 1 ];
			result[ i / 2 ] = inValues[i] + t;
			result[( i + N ) / 2 ] = inValues[i] - t;
		}
	}
/*Pentru 4 threaduri folosesc functia aux_fft in care unul din apeluri
se face pe thread, avand aici in main 2 apeluri pe thread si apoi in
aux_fft cate un apel pe thread de fiecare data cand se apeleaza din main*/
	else if(numThreads == 4){
		elem.step = 2;
		elem.input = result;
		elem.output = inValues;
		elem.th_id = 0;

		struct myStr elem2;
		elem2.step = 2;
		elem2.input= &result[1];
		elem2.output = &inValues[1];
		elem2.th_id = 1;

		pthread_create(&(tid[0]), NULL, aux_fft, &elem);
		pthread_create(&(tid[1]), NULL, aux_fft, &elem2);

		pthread_join(tid[0], NULL);
		pthread_join(tid[1], NULL);

		for( int i = 0; i < N; i += 2){
			double complex t = cexp(-I * M_PI * i / N) * inValues[ i + 1 ];
			result[ i / 2 ] = inValues[i] + t;
			result[( i + N ) / 2 ] = inValues[i] - t;
		}
	}
writeFile(out);

	return 0;
}