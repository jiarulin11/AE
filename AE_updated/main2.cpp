#include <iostream>
#include<stdio.h>
#include<stdlib.h>
#include "functions.h"
#include<time.h>
#include<math.h>
#include<string.h>
#include <sys/stat.h>
#include<stdlib.h>


using namespace std;

double* read_file(char *filename);
double** NQKF(double *samples,char *ite);
double** get_P(char *ite);
double** get_x(char *ite);

int main(int argc, char** argv) {


  char filename[50];
  sprintf(filename, "sample.txt");
  double *samples = read_file(filename);
  
  static double **X;

  X = NQKF(samples, argv[1]);

  char file[] = "Xoutside.txt";
  
  FILE *p=fopen(file, "w");
	for(int i=0;i<4;i++){
		for(int j=0;j<1;j++){ //dimensoes da matrix row,col
			fprintf(p,"%le\t",X[i][j]); //salva linha por linha em uma linha apenas
		}
	}
	fprintf(p,"\n");
	fclose(p);

   return 0;
}





double** NQKF(double *samples, char * ite){
	
	//**********************************************	
		// It depends of the sensors
		int number_of_tests=1;
		double h=0.01;
		int length_t=1000;
	//**********************************************	
		
		double **I3=eye(3);
		double **I4=eye(4);
	
		//noise:
		double **n_G, sigma_G=0.03, moy=0.0;
		double **n_acc, sigma_acc=0.03;	
		double **n_mg, sigma_mg=0.03;
		//Covariance matrix acc noise measurements
		double **Racc=mcm(sigma_acc*sigma_acc,I3,3,3);
		//Covariance matrix of gyros noise
		double **Qg=mcm(sigma_G*sigma_G,I3,3,3); 
		// Covariance matrix of magnetometer measurement noise
		double **R_mg=mcm(sigma_mg*sigma_mg,I3,3,3); 
		// Initial Error estimation covariance matrix    
		double **P;
		
		// Acceleration dans le repere fixe
		double **an_vect=alloc(3,1);
		an_vect[0][0]=0;an_vect[1][0]=0;an_vect[2][0]=1;
		an_vect=mcm(1/norm(an_vect,3),an_vect,3,1);
		
		// Magnetic field in the reference frame
		double m=0.48076;                          // Norm
		double I=64.759*3.1415/180;                           // Inclinaison
		double **m_vect=alloc(3,1);
		double m_vect_[]={m*cos(I),0,m*sin(I)};
		m_vect[0][0]=m_vect_[0];m_vect[1][0]=m_vect_[1];m_vect[2][0]=m_vect_[2];
		m_vect=mcm(1/norm(m_vect,3),m_vect,3,1);         // Normalization
		
		
		//variables declaration 
		double alpha=0.000005;
		double **xm; //store a priori quaternion
		double **Pm; //error covariance matrix
		double **hm;
		double **ba;
		double **ba_vect;
		double **w;
		double **omega_g;
		double **phi;
		double **y_vect=alloc(3,1),y0;
		double **E;
		double **Q1;
		double **xm_vect=alloc(3,1),xm0;
		double **s;
		double **d;
		double **H;
		double **R1;
		double **E1;
		double **s2;
		double **d2;
		double **H2;
		double **R2;
		double **H_bar;
		double **R_bar;
		double **S;
		double **K;
		double **HM; 
		char write[]="w";
		char append[]="a";
	 	char salvar[200]; 
		double **temp,**temp1,**temp2,**y1,**y2;
		int number;
		double **y;
		
	    static double **X=alloc(4,1);
		
		//X = (double*) malloc(4*sizeof(double));

		// ---------------------------------------------------------------------------
		
		y = get_x(ite); 	
		P = get_P(ite);    
		
		
		HM=alloc(3,1);
		HM[0][0]=samples[0];HM[1][0]=samples[1];HM[2][0]=samples[2];
		ba=alloc(3,1);
		ba[0][0]=samples[3];ba[1][0]=samples[4];ba[2][0]=samples[5];
		w=alloc(3,1);
		w[0][0]=samples[6];w[1][0]=samples[7];w[2][0]=samples[8];
		
		
		//-------------------------------------------------------------------------------
		
		//here starts the AE algorithm (1 run)
	
		
				//normalization
				hm=mcm(1/norm(HM,3),HM,3,1);dalloc(HM,3); //normalized magnetic field vector with noise (body frame)
				ba_vect=mcm(1/norm(ba,3),ba,3,1); dalloc(ba,3);//normalized acceleration vector with noise (body frame)
				
		    	//%%%%%%%%%%%%%%% Filter %%%%%%%%%%%%%%%%%%%%%
		    	
		    	//%Process model (prediction) -----------------------------------------------------------------------------------------------------------------------------------
		    	
		    	omega_g=omega_gen(w);dalloc(w,3);
		    
		    	temp=mcm(0.5*h,omega_g,4,4);
		    	phi=expm(temp,4);dalloc(temp,4); ////////////////////////////////////////////////////////
		    	//sprintf(salvar,"./outputs/RUN_%s_%d/phi.txt",argv[1], number);
		    	//store(salvar,phi,4,4,append);
		    	
		    	
		    	xm=mm(phi,4,4,y,4,1); //% a priori state %%%%%%%%%%%%%%%%%%%%%%% prediction           ////////////////////////////////////////////////////////
		    	//sprintf(salvar,"./outputs/RUN_%s_%d/Xm.txt",argv[1],number);
		    	//store(salvar,xm,4,1,append);
		    	
		    	
		    	y_vect[0][0]=y[0][0];y_vect[1][0]=y[1][0];y_vect[2][0]=y[2][0];y0=y[3][0];  
		    	E=E1_gen(y_vect,y0);  
		    
		    	temp=mm(E,4,3,Qg,3,3);
		    	temp1=transpose(E,4,3);
		    	temp2=mm(temp,4,3,temp1,3,4);dalloc(temp,4);dalloc(temp1,3);
		    	Q1=mcm(h*h*0.25,temp2,4,4);dalloc(temp2,4);                // % Process noise covariance matrix     ////////////////////////////////////////////////////////
		    	//sprintf(salvar,"./outputs/RUN_%s_%d/Q1.txt",argv[1],number);
		    	//store(salvar,Q1,4,4,append);
		    
		    
		    
		    	temp1=transpose(phi,4,4);
		    	temp=mm(phi,4,4,P,4,4);
		
		    	temp2=mm(temp,4,4,temp1,4,4);dalloc(temp,4);dalloc(temp1,4);
		    	Pm=sumM(temp2,Q1,4,4,1);dalloc(temp2,4);dalloc(Q1,4);                                ////////////////////////////////////////////////////////
		        dalloc(P,4);
				//sprintf(salvar,"./outputs/RUN_%s_%d/Pm.txt",argv[1],number);
		    	//store(salvar,Pm,4,4,append);
		    	
				
			   // ****Measurement update****** --------------------------------------------------------------------------------------------------------------
			   
			    xm_vect[0][0]=xm[0][0];
			    xm_vect[1][0]=xm[1][0];
			    xm_vect[2][0]=xm[2][0];
				xm0=xm[3][0];
			    
				E1=E1_gen(xm_vect,xm0); 
			    
			    temp=sumM(hm,m_vect,3,1,1);
		    	s=mcm(0.5,temp,3,1);dalloc(temp,3); //3x1
				temp=sumM(hm,m_vect,3,1,-1);
				d=mcm(0.5,temp,3,1);dalloc(temp,3);//3x1
				H=H_gen(s,d);//4x4 Observation matrix
				dalloc(hm,3);
				
				temp1=mm(E1,4,3,R_mg,3,3);
				temp=transpose(E1,4,3);
				temp2=mm(temp1,4,3,temp,3,4);dalloc(temp1,4);dalloc(temp,3);
				temp=mcm(0.25,temp2,4,4);dalloc(temp2,4);
				temp1=mcm(alpha,I4,4,4);
				R1=sumM(temp,temp1,4,4,1); //4x4
				dalloc(d,3);dalloc(s,3);dalloc(temp,4);dalloc(temp1,4);
				
				//	 %accelero
				temp=sumM(ba_vect,an_vect,3,1,1);
			    s2=mcm(0.5,temp,3,1);dalloc(temp,3);//3x1	
				temp1=sumM(ba_vect,an_vect,3,1,-1);
				d2=mcm(0.5,temp1,3,1);dalloc(temp1,3);//3x1
				H2=H_gen(s2,d2); //4x4
				dalloc(ba_vect,3);
					
				temp1=mm(E1,4,3,Racc,3,3);
				temp=transpose(E1,4,3);
				temp2=mm(temp1,4,3,temp,3,4);dalloc(temp1,4);dalloc(temp,3);
				temp=mcm(0.25,temp2,4,4);dalloc(temp2,4);
				temp1=mcm(alpha,I4,4,4);
				R2=sumM(temp,temp1,4,4,1); 
				dalloc(d2,3);dalloc(s2,3);dalloc(temp,4);dalloc(temp1,4);
				
				H_bar=H_bar_gen(H,H2);//[H;H2];8x4                                        ////////////////////////////////////////////////////////
				//sprintf(salvar,"./outputs/RUN_%s_%d/H_bar.txt",argv[1],number);
		    	//store(salvar,H_bar,8,4,append);
		    
				R_bar=R_bar_gen(R1,R2);// % covariance totale (mag+accelero) 8x8          ////////////////////////////////////////////////////////          		
				dalloc(H,4);dalloc(H2,4);dalloc(R1,4);dalloc(R2,4);dalloc(E1,4);
				//sprintf(salvar,"./outputs/RUN_%s_%d/R_bar.txt",argv[1], number);
		    	//store(salvar,R_bar,8,8,append);
		    
				
				temp=transpose(H_bar,8,4);
				temp1=mm(H_bar,8,4,Pm,4,4);
				temp2=mm(temp1,8,4,temp,4,8);dalloc(temp,4);dalloc(temp1,8);
				S=sumM(temp2,R_bar,8,8,1);dalloc(temp2,8);
				
				temp=transpose(H_bar,8,4);
				temp1=mm(Pm,4,4,temp,4,8);
				temp2=inv(S,8);
				K=mm(temp1,4,8,temp2,8,8);dalloc(temp,4);dalloc(temp1,4);dalloc(temp2,8);        ////////////////////////////////////////////////////////
				//sprintf(salvar,"./outputs/RUN_%s_%d/K.txt",argv[1],number);
		    	//store(salvar,K,4,8,append);
		    
				// covariance matrix update
				temp1=mm(K,4,8,H_bar,8,4); 
				temp=sumM(I4,temp1,4,4,-1);dalloc(temp1,4);
				temp1=mm(temp,4,4,Pm,4,4);dalloc(temp,4);
				temp=mm(K,4,8,H_bar,8,4);
				temp2=sumM(I4,temp,4,4,-1);dalloc(temp,4);
				temp=transpose(temp2,4,4);dalloc(temp2,4);
				temp2=mm(temp1,4,4,temp,4,4);dalloc(temp,4);dalloc(temp1,4);
				temp1=mm(K,4,8,R_bar,8,8);
				temp=transpose(K,4,8);
				y1=mm(temp1,4,8,temp,8,4);
				P=sumM(temp2,y1,4,4,1);dalloc(temp2,4);dalloc(y1,4);dalloc(temp,8);dalloc(temp1,4);              ////////////////////////////////////////////////////////
				//sprintf(salvar,"./outputs/RUN_%s_%d/P.txt", argv[1],number);
				sprintf(salvar,"P.txt");
		    	store(salvar,P,4,4,write);
		    
				temp1=mm(K,4,8,H_bar,8,4);
				temp2=mm(temp1,4,4,xm,4,1);dalloc(temp1,4);
				y1=sumM(xm,temp2,4,1,-1);dalloc(temp2,4); 
				y2=mcm(1/norm(y1,4),y1,4,1);
				for(int j=0;j<4;j++){
					y[j][0]=y2[j][0]; // a posteriori state estimation                              ////////////////////////////////////////////////////////
					X[j][0] = y[j][0];
				}
				//sprintf(salvar,"./outputs/RUN_%s_%d/X.txt", argv[1],number);
		    	sprintf(salvar,"X.txt");
				store(salvar,y,4,1,write);
				sprintf(salvar,"Xtotal.txt");
				store(salvar,y,4,1,append);
		    
				dalloc(y1,4);dalloc(y2,4);
		
		    	dalloc(xm,4);
		    	dalloc(K,4);dalloc(H_bar,8);dalloc(R_bar,8);dalloc(S,8);
		    	dalloc(I3,3);dalloc(I4,4);
		    	
		    	return X;
		   	
	
}


double** get_P(char * ite){

//ler o arquivo
  
  
  double **P;
  int i, j;
  double **I4;
  I4 = eye(4);
  
  	
  if(*ite == '0'){
  //	printf("%s\n",ite);
  	P = mcm(0.01, I4, 4, 4);	    
  }
  else{
  	FILE *myfile;
	myfile=fopen("P.txt", "r");
	P=alloc(4,4);
	  for(i=0; i<4; i++){
	  	for(j=0; j<4; j++){		
	  		fscanf(myfile,"%le",&P[i][j]);	
		  }
	  	}
	   fclose(myfile);	
  }
 
  return P;
}


double** get_x(char * ite){

//ler o arquivo
  
  double **x=alloc(4,1);
  int i;
  int j;
  double aux;
  char salvar[50];
  char write[] = "w";
  
 // printf("%s\n",ite);
  
  if(*ite == '0'){
  	//printf("oi\n");
  	x[0][0]=0;x[1][0]=0;x[2][0]=0;x[3][0]=1; 
  	sprintf(salvar,"Xtotal.txt");
	store(salvar,x,4,1,write);
  }
  else{
	FILE *myfile;
  
	myfile=fopen("X.txt", "r");
	  for(i=0; i<4; i++){
	  	for(j=0; j<1; j++){		
	  		fscanf(myfile,"%le",&x[i][j]);	
		  }
	  	}
  fclose(myfile);
  
  }
  return x;
}


double * read_file(char filename[50]){
	static double samples[1][9];
	int k = 0;
	FILE *file;

	file = fopen(filename,"r");	
			
	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){
			fscanf(file,"%le", &samples[0][k]);
			k++;
		}
	}
	
    fclose(file);

    return samples[0];
}


