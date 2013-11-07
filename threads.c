#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <errno.h>

#define FILENAME "matrix"
#define ST_OUT 1
#define ANSWERNAME "answer"

// AP: задание - составьте график зависимости времени работы от m для матриц болього размера и с RESTRICT = 1000

struct targ {
	int n; // matrix size
	int repeat; // how many operations this thread has to carry out
	int* matrix; // array of two matrixes
	int* result; // result array
	int line; // first line (numeration starts with 0)
	int column; // first column (numeration starts with 0)
};

void* threadf (void * temporal){ // the thread function
	// AP: используйте оператор -> вместо .
	struct targ * arguments = (struct targ *) temporal;
	int j;
	int res, i;
	int counter = ((*arguments).n)*((*arguments).n);
	int n = (*arguments).n;
	int rp = (*arguments).repeat;
	int ln = (*arguments).line;
	int cl = (*arguments).column;
	int integer = sizeof(int);

	for(j = 0; j < rp; j++){
		res = 0;	
		for(i=0; i<n; i++){
			res += (arguments->matrix[n*ln+i] * arguments->matrix[counter+cl+n*i]);
		}
		arguments->result[n*ln+cl] = res;
		cl++;
		if(cl == n){
			cl = 0;
			ln ++;
		} // order: line and all columns
	}
	return;
}

int main(int argc, char* argv[]){
	
	if (argc != 2){
		printf("Error: invalid number of arguments\n");
		exit(-1);
	}

	errno = 0; // getting the argument correctly
	int step = sizeof(char);
	char** unvalid = (char**)malloc(step);
	int m = strtol(argv[1], unvalid, 10);
	if ((errno != 0) || (**unvalid != '\0')){
		printf("Error: invalid number of threads\n");
		exit(-1);
	}
	
	int fd;

	(void)umask(0);

	if((fd = open(FILENAME, O_RDONLY)) < 0){
		printf("Error: cannot open the matrix file\n");
		exit(-1);
	}

	int* matrix;
	int integer = sizeof(int);
	matrix = (int*)malloc(integer);

	size_t size;
	int counter = 0;
	// AP: организуетй чтение с произвольным буфером, а не по одной цифре
	while((size = read(fd, matrix+counter, integer)) != 0){ // reading the matrix file into one big array
		if(size != integer){
			printf("Error:cannot read the matrix file\n");
			exit(-1);
		}
		counter ++;
		matrix = (int*)realloc(matrix, (counter + 1)*integer);
	}
	counter /= 2;
	int n = (int)sqrt((double)counter);

	if(m > counter) {
		printf("Warning: the number of threads is excessive for this task; only %d threads will be used\n", counter);
		m = counter;
	}
	
	int* result = (int*) malloc(counter*integer);

	int uneven = counter % m;
	int portion = (counter - uneven) / m; // uneven threads will have portion+1 repeats, m-uneven will have portion repeats
	
	struct targ arg[m];
	pthread_t names[m];
	int k, rest;

	for(k=0; k<(m-uneven); k++){ // creating threads
		arg[k].repeat = portion;
		arg[k].n = n;
		arg[k].matrix = matrix;
		arg[k].result = result;
		arg[k].line = (portion*k)/n;
		arg[k].column = (portion*k)%n;
		if(pthread_create(&(names[k]), NULL, threadf, (void *)(arg+k)) != 0){
			printf("Error: cannot create the new thread\n");
			exit(-1);
		}
	}

	for(k=m-uneven; k<m; k++){ // creating threads
		arg[k].repeat = portion + 1;
		arg[k].n = n;
		arg[k].matrix = matrix;
		arg[k].result = result;
		arg[k].line = ((m-uneven)*portion + (portion+1)*(k-m+uneven))/n;
		arg[k].column = ((m-uneven)*portion + (portion+1)*(k-m+uneven))%n;
		if(pthread_create(&(names[k]), NULL, threadf, (void *)(arg+k)) != 0){
			printf("Error: cannot create the new thread\n");
			exit(-1);
		}	
	}
	
	for(k=0; k<m; k++){
		if(pthread_join(names[k], NULL) != 0){
			printf("Error: cannot create the new thread\n");
			exit(-1);
		}
	}

	if(close(fd) < 0){
		printf("Error: cannot close standard output\n");
		exit(-1);
	}
	

	if(close(ST_OUT) < 0){
		printf("Error: cannot close standard output\n");
		exit(-1);
	}
	

	if((fd = open(ANSWERNAME, O_WRONLY|O_CREAT, 0666)) < 0){
		printf("Error: cannot create the matrix file\n");
		exit(-1);
	}

	// AP: объясните почему result, находящийся в динмаической памяти, тем не менее содержит в себе результаты работы всех потоков - разделяется же только статическая память
	for(k = 0; k < counter; k++){
		printf("%d ", *(result + k));
		if((k+1)%n == 0){
			printf("\n");
		}
	}
	// AP: а где определение времени работы?
	
	return 0;
}
