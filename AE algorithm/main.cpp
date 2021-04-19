#include <iostream>
#include<stdio.h>
#include<stdlib.h>
#include "functions.h"
#include<time.h>
#include<math.h>
#include<string.h>
#include <sys/stat.h>
#include<stdlib.h>

void input(int ite,int n,char* process);
int create_directory(char *id);
void comparison(int ite);


using namespace std;


int main(int argc, char** argv) {
	
	if(argc <= 1){
		printf("Identifier needed\n");
		exit(0);
	}
	
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
	
	//create de directories 
	number=create_directory(argv[1]);
	
	// Filter Initialisation 
	double y_[]={0,0,0,1}; //Initial estimate 	
	double **y=alloc(4,1);
	
	
	// Initial estimatives for P and X
	P=mcm(0.01,I4,4,4); // initial error covariance matrix
	sprintf(salvar,"./outputs/RUN_%s_%d/P.txt",argv[1],number);
	store(salvar,P,4,4,append);
	y[0][0]=y_[0];y[1][0]=y_[1];y[2][0]=y_[2];y[3][0]=y_[3];
	y=mcm(1/norm(y,4),y,4,1);
	sprintf(salvar,"./outputs/RUN_%s_%d/X.txt",argv[1],number);
	store(salvar,y,4,1,append);
	    
		
	char HM_[100];
	sprintf(HM_,"./temp_%s/HM.txt",argv[1]);
	char ba_[100];
	sprintf(ba_,"./temp_%s/ba.txt",argv[1]);
	char w_[100];	
	sprintf(w_,"./temp_%s/w.txt",argv[1]);
	
	char data[300];
	sprintf(data,"date +%%Y-%%m-%%d-%%H%%M-%%S-%%N");
	sprintf(salvar, " > ./output_data_set_timestamps/timestamp_%s_%d/begin_run.txt", argv[1], number);
	strcat(data,salvar);
	system(data);//inicio do run
	
	sprintf(salvar,"echo 0 >./output_data_set_timestamps/timestamp_%s_%d/ask_input.txt", argv[1], number);
	system(salvar);//pedido do input
	
	sprintf(salvar,"echo 0 >./output_data_set_timestamps/timestamp_%s_%d/receive_input.txt", argv[1],number);
	system(salvar);//recebimento input
	
	sprintf(salvar,"echo 0 >./output_data_set_timestamps/timestamp_%s_%d/output.txt", argv[1],number);
	system(salvar); //calculo do output
	
	//	----------------------------------------------------------------------------
	
	HM=alloc(3,1);
	ba=alloc(3,1);
	w=alloc(3,1);
	HM[0][0]=0.578270;HM[1][0]=-0.632796;HM[2][0]=0.511008; 	 
	ba[0][0]=0.872348 ;ba[1][0]= -0.321262;ba[2][0]=0.469674;	 	 	 
	w[0][0]= 0.008239;w[1][0]=0.047402;w[2][0]= 0.006209;
	length_t=2;
	
	//-------------------------------------------------------------------------------
	
	//here starts the AE algorithm (1 run)
	for(int i=1;i<length_t;i++){

			sprintf(data,"date +%%Y-%%m-%%d-%%H%%M-%%S-%%N");
			sprintf(salvar," >> ./output_data_set_timestamps/timestamp_%s_%d/ask_input.txt",argv[1],number);
			strcat(data,salvar);
			system(data);

//-----------------------------------------------------------------------------------------------------------------------------------		

// If you want to simulate all de input data set, uncomment lines 160 up to 168 and comment lines 138 up to 146
/*
			 //sensors input -> this function gets the sensors data from the .txt file
			 input(i,1000,argv[1]);

			//getting data from the sensors + noise
	    		HM=read_temp(HM_,3,1); 
			    ba=read_temp(ba_,3,1);
	    		w=read_temp(w_,3,1);
*/			

//-----------------------------------------------------------------------------------------------------------------------------------		
	    		
	    		sprintf(data,"date +%%Y-%%m-%%d-%%H%%M-%%S-%%N");
	    		sprintf(salvar," >>./output_data_set_timestamps/timestamp_%s_%d/receive_input.txt", argv[1],number);
	    		strcat(data,salvar);
	    		system(data);

			//normalization
			hm=mcm(1/norm(HM,3),HM,3,1);dalloc(HM,3); //normalized magnetic field vector with noise (body frame)
			ba_vect=mcm(1/norm(ba,3),ba,3,1); dalloc(ba,3);//normalized acceleration vector with noise (body frame)
			
	    	//%%%%%%%%%%%%%%% Filter %%%%%%%%%%%%%%%%%%%%%
	    	
	    	//%Process model (prediction) -----------------------------------------------------------------------------------------------------------------------------------
	    	
		
	    	omega_g=omega_gen(w);dalloc(w,3);
	    
	    	temp=mcm(0.5*h,omega_g,4,4);
	    	phi=expm(temp,4);dalloc(temp,4); ////////////////////////////////////////////////////////
	    	sprintf(salvar,"./outputs/RUN_%s_%d/phi.txt",argv[1], number);
	    	store(salvar,phi,4,4,append);
	    	
	    	
	    	xm=mm(phi,4,4,y,4,1); //% a priori state %%%%%%%%%%%%%%%%%%%%%%% prediction           ////////////////////////////////////////////////////////
	    	sprintf(salvar,"./outputs/RUN_%s_%d/Xm.txt",argv[1],number);
	    	store(salvar,xm,4,1,append);
	    	
	    	
	    	y_vect[0][0]=y[0][0];y_vect[1][0]=y[1][0];y_vect[2][0]=y[2][0];y0=y[3][0];  
	    	E=E1_gen(y_vect,y0);  
	    
	    	temp=mm(E,4,3,Qg,3,3);
	    	temp1=transpose(E,4,3);
	    	temp2=mm(temp,4,3,temp1,3,4);dalloc(temp,4);dalloc(temp1,3);
	    	Q1=mcm(h*h*0.25,temp2,4,4);dalloc(temp2,4);                // % Process noise covariance matrix     ////////////////////////////////////////////////////////
	    	sprintf(salvar,"./outputs/RUN_%s_%d/Q1.txt",argv[1],number);
	    	store(salvar,Q1,4,4,append);
	    
	    
	    
	    	temp1=transpose(phi,4,4);
	    	temp=mm(phi,4,4,P,4,4);
	
	    	temp2=mm(temp,4,4,temp1,4,4);dalloc(temp,4);dalloc(temp1,4);
	    	Pm=sumM(temp2,Q1,4,4,1);dalloc(temp2,4);dalloc(Q1,4);                                ////////////////////////////////////////////////////////
	        dalloc(P,4);
			sprintf(salvar,"./outputs/RUN_%s_%d/Pm.txt",argv[1],number);
	    	store(salvar,Pm,4,4,append);
	    	
	
						
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
			sprintf(salvar,"./outputs/RUN_%s_%d/H_bar.txt",argv[1],number);
	    	store(salvar,H_bar,8,4,append);
	    
			R_bar=R_bar_gen(R1,R2);// % covariance totale (mag+accelero) 8x8          ////////////////////////////////////////////////////////          		
			dalloc(H,4);dalloc(H2,4);dalloc(R1,4);dalloc(R2,4);dalloc(E1,4);
			sprintf(salvar,"./outputs/RUN_%s_%d/R_bar.txt",argv[1], number);
	    	store(salvar,R_bar,8,8,append);
	    
			
			temp=transpose(H_bar,8,4);
			temp1=mm(H_bar,8,4,Pm,4,4);
			temp2=mm(temp1,8,4,temp,4,8);dalloc(temp,4);dalloc(temp1,8);
			S=sumM(temp2,R_bar,8,8,1);dalloc(temp2,8);
			
			temp=transpose(H_bar,8,4);
			temp1=mm(Pm,4,4,temp,4,8);
			temp2=inv(S,8);
			K=mm(temp1,4,8,temp2,8,8);dalloc(temp,4);dalloc(temp1,4);dalloc(temp2,8);        ////////////////////////////////////////////////////////
			sprintf(salvar,"./outputs/RUN_%s_%d/K.txt",argv[1],number);
	    	store(salvar,K,4,8,append);
	    
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
			sprintf(salvar,"./outputs/RUN_%s_%d/P.txt", argv[1],number);
	    	store(salvar,P,4,4,append);
	    
			temp1=mm(K,4,8,H_bar,8,4);
			temp2=mm(temp1,4,4,xm,4,1);dalloc(temp1,4);
			y1=sumM(xm,temp2,4,1,-1);dalloc(temp2,4); 
			y2=mcm(1/norm(y1,4),y1,4,1);
			for(int j=0;j<4;j++){
				y[j][0]=y2[j][0]; // a posteriori state estimation                              ////////////////////////////////////////////////////////
			}
			sprintf(salvar,"./outputs/RUN_%s_%d/X.txt", argv[1],number);
	    	store(salvar,y,4,1,append);
	    
			dalloc(y1,4);dalloc(y2,4);
	
	    	dalloc(xm,4);
	    	dalloc(K,4);dalloc(H_bar,8);dalloc(R_bar,8);dalloc(S,8);
	    	
	
			sprintf(data,"date +%%Y-%%m-%%d-%%H%%M-%%S-%%N");
			sprintf(salvar," >> ./output_data_set_timestamps/timestamp_%s_%d/output.txt",argv[1],number);
			strcat(data,salvar);
			system(data);
	
			
		//out=function(samples)  
		}
		
dalloc(I3,3);dalloc(I4,4);

sprintf(data,"date +%%Y-%%m-%%d-%%H%%M-%%S-%%N");
sprintf(salvar," > ./output_data_set_timestamps/timestamp_%s_%d/end_run.txt",argv[1],number);
strcat(data,salvar);
system(data);//fim do run

/*
sprintf(salvar, "ssh %s \"nohup mkdir %s >/dev/null 2>/dev/null </dev/null &\"", argv[2], argv[3]);
system(salvar);

sprintf(salvar,"scp -r ./output_data_set_timestamps/timestamp_%s_%d %s:%s",argv[1], number ,argv[2], argv[3]);	
system(salvar);

sprintf(salvar,"scp -r ./outputs/RUN_%s_%d %s:%s", argv[1], number ,argv[2], argv[3]);
system(salvar);
*/
	
//system("date +%Y-%m-%d-%H%M-%S-%N >> ./output_data_set_timestamps/$output_data_set_timestamps.log");
//system("scp -r ./output_data_set_timestamps/$output_data_set_timestamps.log tarso@10.42.0.1:/home/tarso/Desktop/output_data_set_timestamps/");
	return 0;
}


void input(int ite,int n,char *process){
	// n is the number of points
		//GETTING ALL THE SENSORS DATA FOR THE TEST
		double **HM,**w,**ba;
		
		HM=get_mag(n,ite-1,1);
		ba=get_acc(n,ite-1,1);
		w=get_gyros(n,ite-1,1);
	
	    
	    char write[]="w";
		char HM_[100];
		sprintf(HM_,"./temp_%s/HM.txt",process);
	    char ba_[100];
	    sprintf(ba_,"./temp_%s/ba.txt",process);
		char w_[100];
		sprintf(w_,"./temp_%s/w.txt",process);
		store(HM_,HM,3,1,write);
	    store(ba_,ba,3,1,write);
	    store(w_,w,3,1,write);
	    
	    
	    dalloc(HM,3);dalloc(ba,3);dalloc(w,3);	
}

void comparison(int ite){
	FILE *p0=fopen("./GOLD/X.txt","r");
	double **var=alloc(4,1);
	for(int i=0;i<1000;i++){
		for(int j=0;j<4;j++){
			if(i==ite){
				fscanf(p0,"%lf",&var[j][0]);
			}
		}	
	}
	char sla[]="./temp/Xtemp.txt";
	double **v=read_temp(sla,4,1);
	
	double dif=fabs(norm(var,4)-norm(v,4));
	
		if(dif>1e-6){
			printf("Error in X = %f\n",dif);
		}
	fclose(p0);
	dalloc(v,4);dalloc(var,4);
}

int create_directory(char* id){

	int number=0;
	int check,check1,check2,check3,check4;
	char run[100];//="./outputs/RUN_0";
	sprintf(run,"./outputs/RUN_%s_0", id);

	char timestamp[100];//
        sprintf(timestamp, "./output_data_set_timestamps/timestamp_%s_0", id);
    
    char temp[100];
    	sprintf(temp,"./temp_%s",id);
    	
    char output_data_set_timestamps[] = "./output_data_set_timestamps";
    char outputs[] = "./outputs"; 
//	FILE *p;
	while(true){
		check = mkdir(run,0777);
		check1 = mkdir(timestamp,0777);
		check2 = mkdir(temp,0777);
		check3 = mkdir(output_data_set_timestamps,0777);
		check4 = mkdir(outputs,0777);
		if(!check){
			
			break;			
		}else{
			number++;
			sprintf(run,"./outputs/RUN_%s_%d",id,number);
			sprintf(timestamp,"./output_data_set_timestamps/timestamp_%s_%d",id,number);
		}
	}
return number;
}

