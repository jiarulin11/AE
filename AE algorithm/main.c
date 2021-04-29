#include <stdio.h>
#include <stdlib.h>

float set_vers_svs[ 16 ][2] = {
        { 1.9 ,  0.2 },
        { 3.3 ,  1.0 },
};

float set_vers_alphas[ 2 ] = {
         0.8,
         -0.8,
};
float set_vers_bias =  3.169231 ;
float set_virg_svs[ 16 ][2] = {
        { 1.9 ,  0.2 },
        { 4.5 ,  1.7 },
};
float set_virg_alphas[ 2 ] = {
         0.2,
         -0.2,
};
float set_virg_bias =  2.163152 ;
float versi_virg_svs[ 16 ][2] = {
        { 4.7 ,  1.4 },
        { 4.5 ,  1.5 },
        { 4.9 ,  1.5 },
        { 4.6 ,  1.5 },
        { 4.7 ,  1.6 },
        { 4.7 ,  1.4 },
        { 4.8 ,  1.8 },
        { 4.9 ,  1.5 },
        { 5.1 ,  1.9 },
        { 4.5 ,  1.7 },
        { 5.1 ,  2.0 },
        { 5.3 ,  1.9 },
        { 5.0 ,  2.0 },
        { 5.0 ,  1.5 },
        { 4.9 ,  2.0 },
        { 4.9 ,  1.8 }
};
float versi_virg_alphas[ 16 ] = {
         1.0,
         0.2,
         1.0,
         1.0,
         1.0,
         1.0,
         1.0,
         1.0,
         -1.0,
         -1.0,
         -1.0,
         -0.2,
         -1.0,
         -1.0,
         -1.0,
         -1.0,
};
float versi_virg_bias =  10.54 ;

float svm_compute(float sample[], int n_svs, float svs[][2], float alphas[], float bias){

  int i = 0;
  float acc_sum = 0;

  for(i = 0 ; i < n_svs ; i++){

    acc_sum += ((sample[0] * svs[i][0] + sample[1]*svs[i][1])*alphas[i]);

  }

  return acc_sum + bias;
}


float samples[1][2] = {
        { 1.4 , 0.2 }
    };



typedef enum{
    SET = 0, VERS, VIRG
} final_classes_t;

final_classes_t classify(float vals[]){

    int flower[3] = {0, 0, 0};

    if(vals[0] > 0){
        flower[0] += 1;
    }
    else{
        flower[1] += 1;
    }

    if(vals[1] > 0){
        flower[0] += 1;
    }
    else{
        flower[2] += 1;
    }
    if(vals[2] > 0){
        flower[1] += 1;
    }
    else{
        flower[2] += 1;
    }

    int i;
    int final_class = 0;
    int vote_count = flower[0];
    for(i = 1 ; i < 3 ; i++){

        if(flower[i] > vote_count){
            vote_count = flower[i];
            final_class = i;
        }

    }

    return final_class;

}

char* ReadFile(char *filename)
{
   char *buffer = NULL;
   int string_size, read_size;
   FILE *handler = fopen(filename, "r");

   if (handler)
   {
       // Seek the last byte of the file
       fseek(handler, 0, SEEK_END);
       // Offset from the first to the last byte, or in other words, filesize
       string_size = ftell(handler);
       // go back to the start of the file
       rewind(handler);

       // Allocate a string that can hold it all
       buffer = (char*) malloc(sizeof(char) * (string_size + 1) );

       // Read it all in one operation
       read_size = fread(buffer, sizeof(char), string_size, handler);

       // fread doesn't set it so put a \0 in the last position
       // and buffer is now officially a string
       buffer[string_size] = '\0';

       if (string_size != read_size)
       {
           // Something went wrong, throw away the memory and set
           // the buffer to NULL
           free(buffer);
           buffer = NULL;
       }

       // Always remember to close the file.
       fclose(handler);
    }

    return buffer;
}



float * read_file(char *filename){
	static float samples[1][3];
	float num;
	int i = 0;
	FILE *file = NULL;


	file = fopen(filename,"r");		

	while( fscanf(file, "%f", &num) != EOF ) {
        samples[0][i++] = num;
    }

    fclose(file);


    
    return samples[0];
}


void write_file(char *filename, char result[30]){
  FILE *fp = NULL;

  fp=fopen(filename, "w");
  
  if(fp == NULL)
    exit(-1);
  
  fprintf(fp, "%s", result);
  
  fclose(fp);

}


int main(){
	
	float results[3];
	char output[30];
	

	// Read data from txt file (written in python script for fault injection)
  float *samples = read_file("sample.txt");
	
	// Return the results of SVM
	results[0] = svm_compute(samples, 2, set_vers_svs, set_vers_alphas, set_vers_bias);
	results[1] = svm_compute(samples, 2, set_virg_svs, set_virg_alphas, set_virg_bias);
	results[2] = svm_compute(samples, 16, versi_virg_svs, versi_virg_alphas, versi_virg_bias);

  // Save results in a string
  sprintf(output, "%.5f %.5f %.5f", results[0], results[1], results[2]);

  // Write outputfile to open in python script for fault injection
  write_file("output.txt", output);
	    //fclose(fp);
	printf("%.5f %.5f %.5f \\n", results[0], results[1], results[2]);
	    //printf("%3d: ", i);
	    //printf("%5f, ", results[0]);
	    //printf("%5f, ", results[1]);
	    //printf("%5f\n", results[2]);
	    //printf("Final class -> %d\n", classify(results));
  



}
