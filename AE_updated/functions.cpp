#include <iostream>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<assert.h>

void dalloc(double **a,int row){
	if(a!=NULL){
		for(int i=0;i<row;i++){
			free(a[i]);
		}
		free(a);
	}
}

double** alloc(int row,int col){
	double **x= (double**) malloc(row*sizeof(double*));
	for(int i=0;i<row;i++){
		x[i]=(double*) malloc(col*sizeof(double));
	}
	return x;
}

double** create_vec(double x0,double p,double xf){
	int length_v=round((xf-x0)/p);
	double **v=alloc(length_v,1);
	v[0][0]=x0;
	for(int i=1;i<length_v;i++){
		v[i][0]=v[i-1][0]+p;
	}	
	return v;
}

//Marsaglia polar method to generate random normal numbers
double** randn(int n){
	int i,k=0;
	double **a=alloc(n,1);
        for ( i = 0; i < n; i += 2 )
        {
            double x,y,rsq,f;
            do {
                x = 2.0 * rand() / (double)RAND_MAX - 1.0;  
				y = 2.0 * rand() / (double)RAND_MAX - 1.0;
                rsq = x * x + y * y;       
            }while( rsq >= 1. || rsq == 0. );
            
			f = sqrt(-2.0 * log(rsq) / rsq);
		
            a[i][0]= x*f;
            a[i+1][0]= y*f;
           assert(isnan(a[i][0])==0);
           assert(isnan(a[i+1][0])==0);
	    }

    return a;
}


void store(char *filename,double **var,int row,int col,char option[]){
	FILE *p=fopen(filename, option);
	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++){ //dimensoes da matrix row,col
			fprintf(p,"%le\t",var[i][j]); //salva linha por linha em uma linha apenas
		}
	}
	fprintf(p,"\n");
	fclose(p);
}


double** read_temp(char name[20],int row,int col){
	FILE *p=fopen(name,"r"); //ENTRAR COM A DIMENSAO DA MATRIZ DESEJADA
	double **var=alloc(row,col);
	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++){
			fscanf(p,"%lf",&var[i][j]);
		}
	}
	fclose(p);
	return var;
}


double** noise(char filename[30],int n){
	
 //ler o arquivo
  FILE *myfile;
  double myvariable;
  double **noise=alloc(3,n);
  int i;
  int j;

  myfile=fopen(filename, "r");

  for(i = 0; i < 3; i++){
    for (j = 0 ; j < n; j++)
    {
      fscanf(myfile,"%lf",&noise[i][j]);
      //printf("%f ",noise[i][j]);
    }
    //printf("\n");
  }

  fclose(myfile);
  return noise;

}




//sum
double sum(double **v,int N){
	double c=0;
	int k=0;
	for(int i=0;i<N;i++){
		c=c+v[i][0];
	}
	assert(isnan(c)==0);
           
	return c;
}

//mean
double mean(double **v,int N){
	double c,n;
	n=(double) N;
	c=sum(v,N)/n;
	
	assert(isnan(c)==0);
	return c;
}

//variance
double var(double **v, int N){
	
	double **aux=alloc(N,1);
	double u=mean(v,N),n=(double) N;
	for(int i=0;i<N;i++){
		aux[i][0]=pow(v[i][0]-u,2);	
	}
	double c=1/(n-1)*sum(aux,N);
	dalloc(aux,N);
	assert(isnan(c)==0);
	return c;	
}


//print matrix
void printM(double **v,int row,int col){
	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++){
			printf("%f  ",v[i][j]);
		}
		printf("\n");
	}
	printf("\n");

}



//generate noise
double** noise_generation(double sigma_G,double moy, int n){

	double **n_G_x;
	double **n_G_y;
	double **n_G_z;
	double **n_G=alloc(3,n);
	//it takes normal random distribution vectors
	n_G_x=randn(n);n_G_y=randn(n);n_G_z=randn(n);
	
	double sigmax=sqrt(var(n_G_x,n)),meanx=mean(n_G_x,n);
	double sigmay=sqrt(var(n_G_y,n)),meany=mean(n_G_y,n);
	double sigmaz=sqrt(var(n_G_z,n)),meanz=mean(n_G_z,n);
	
	//modify the mean and variance
	for(int i=0;i<n;i++){
		n_G_x[i][0]=(n_G_x[i][0]/sigmax-meanx)*sigma_G+moy;
		assert(isnan(n_G_x[i][0])==0);
		n_G_y[i][0]=(n_G_y[i][0]/sigmay-meany)*sigma_G+moy;
		assert(isnan(n_G_y[i][0])==0);
		n_G_z[i][0]=(n_G_z[i][0]/sigmaz-meanz)*sigma_G+moy;
		assert(isnan(n_G_z[i][0])==0);
	}
	
	//putting inside a matrix n_G[3][length_t]=n_G[i*length_t+j]
	for(int t=0;t<3;t++){
		for(int j=0;j<n;j++){
			if(t==0){
				n_G[t][j]=n_G_x[j][0];
				assert(isnan(n_G[t][j])==0);
			}else if(t==1){
				n_G[t][j]=n_G_y[j][0];
				assert(isnan(n_G[t][j])==0);
			}else if(t==2){
				n_G[t][j]=n_G_z[j][0];
				assert(isnan(n_G[t][j])==0);
			}
			
		}
	}
	dalloc(n_G_x,n);dalloc(n_G_y,n);dalloc(n_G_z,n);
	return n_G;
}

//--------

//matrix multiplication
double** mm(double **A,int row_a,int col_a,double **B,int row_b,int col_b){
		if(col_a!=row_b){
			printf("Matrix dimensions mismatch!");
			return NULL;
		}
		else{
			int i,j,k;
			double sum=0.0,a,b;
			double **result=alloc(row_a,col_b);
			for(i=0;i<row_a;i++){
				for(j=0;j<col_b;j++){
					for(k=0;k<row_b;k++){
						a=A[i][k];
						b=B[k][j];
						sum+=A[i][k]*B[k][j];
					}	
					if(isnan(sum)!=0){
						printf("a=%f\n",a);
						printf("b=%f\n",b);
						assert(0);					
					}
					result[i][j]=sum;
					sum=0;
				}
			}
		return result;
		}	
}

double** eye(int dim){
	int i,j;
	double **I=alloc(dim,dim);
	for(i=0;i<dim;i++){
		for(j=0;j<dim;j++){
			if(i==j){
				I[i][j]=1;
				}
			else{
				I[i][j]=0;		
				}
	}
	}
	return I;	
}


//multiplication constant x matrix 
double** mcm(double c,double **A,int row, int col){
	double **result=alloc(row,col);
	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++){
			result[i][j]=c*A[i][j];
			assert(isnan(result[i][j])==0);
		}
	}
	return result;
	
}


//vector norm
double norm(double **v,int n){
	double k=0;
	for(int i=0;i<n;i++){
		k=k+v[i][0]*v[i][0];
	}
	assert(isnan(sqrt(k))==0);
	return sqrt(k);
}


//anti-simetric
double** skew(double **w0){
	//printV(w0,3);
	double **S=alloc(3,3);
	S[0][0]=0;
	S[0][1]=-w0[2][0];
	S[0][2]=w0[1][0];
	S[1][0]=w0[2][0];
	S[1][1]=0;
	S[1][2]=-w0[0][0];
	S[2][0]=-w0[1][0];
	S[2][1]=w0[0][0];
	S[2][2]=0;
	return S;
}

double** omega_gen(double **w0){
	
//omega_g0=[-skew(w0) w0;-w0' 0];
	int i,j;
	double **S;
	double **omega_g0=alloc(4,4);
	
	S=skew(w0);
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			if(i<3 && j<3){
				omega_g0[i][j]=-S[i][j];
				assert(isnan(omega_g0[i][j])==0);
			}
			else if(i<3 && j>=3){
				omega_g0[i][j]=w0[i][0];
				assert(isnan(omega_g0[i][j])==0);
			}
			else if(i>=3 && j<3){
				omega_g0[i][j]=-w0[j][0];
				assert(isnan(omega_g0[i][j])==0);							
			}
	}
	}
	omega_g0[3][3]=0.0;
	dalloc(S,3);
	return omega_g0;
}


double** H_gen(double **s,double **d){
//H=[-skew(s) d;-d' 0];

	int i,j;
	double **S;
	double **H=alloc(4,4);
	
	S=skew(s);
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			if(i<3 && j<3){
				H[i][j]=-S[i][j];
				assert(isnan(H[i][j])==0);
			}
			else if(i<3 && j>=3){
				H[i][j]=d[i][0];
				assert(isnan(H[i][j])==0);
			}
			else if(i>=3 && j<3){
				H[i][j]=-d[j][0];
				assert(isnan(H[i][j])==0);							
			}
	}
	}
	H[3][3]=0;
	dalloc(S,3);
	return H;
}

//matrix sum
double** addM(double **a,double **b,int row,int col,int op){
	double **r=alloc(row,col);
	
	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++){
			r[i][j]=a[i][j]+op*b[i][j];
			assert(isnan(r[i][j])==0);
		}
	}
	return r;
}

//factorial
double fat(int n){
	if(n==0){
		return 1;
	}else if(n==1){
		return 1;
	}else{
	return n*fat(n-1);
	}
}

//matrix power
double** powM(double **A,int row,int col, int n){
	double **B=alloc(row,col);
	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++){
			B[i][j]=A[i][j];
		}
	}
	double **prox;	
	for(int k=1;k<n;k++){
		prox = mm(B,row,col,A,row,col);
		for(int i=0;i<row;i++){
			for(int j=0;j<col;j++){
				B[i][j]=prox[i][j];
			}
		}
		dalloc(prox,row);
	}
	return B;
}

//order reduction
double** reduction(double **A,int n, int row,int col){
	if (n<=1){
		printf("Reduction impossible\n");
		return NULL;
	}
	if (n==2){
		return A;
	}
	int m=n-1,k=0,s=0;
	double **M=alloc(m,m);
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			if(i!=row && j!=col){
				if(s<m){
					M[k][s]=A[i][j];
					s++;
				}else{
					s=0;
					k++;
					M[k][s]=A[i][j];
					s++;
				}
			}
		
		}
		
	}
	//printM(M,m,m);
	return M;
	
}


//determinant
double det(double **A,int n){
	if(n==1)
		return A[0][0];
	if(n==2){
		return A[0][0]*A[1][1]-A[1][0]*A[0][1];
	}
	int m=n-1;
	double **M;
	double sum=0;
	for(int i=0;i<1;i++){
		for(int j=0;j<n;j++){
			M=reduction(A,n,i,j);
			sum+=pow(-1,i+j)*A[0][j]*det(M,n-1);
			dalloc(M,n-1);
		}
	}
	assert(isnan(sum)==0);
	return sum;
}

//transpose
double** transpose(double **a,int row,int col){
	int i,j;
	double **result=alloc(col,row);
	for(i=0;i<col;i++){
		for(j=0;j<row;j++){
			result[i][j]=a[j][i];
			assert(isnan(result[i][j])==0);
		}
	}
	return result;
}


double** PI(int a,int b,int dim){
	double **M=eye(dim);
	double **result=eye(dim);
	for(int i=0;i<dim;i++){
		result[a][i]=M[b][i];
		result[b][i]=M[a][i];
	}
	dalloc(M,dim);
	return result;
}

double detLU(double **A,int n){
	double pivo;
	double **P=eye(n),**U=alloc(n,n);
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			U[i][j]=A[i][j];
		}
	}
	int jpivo=0,pivo_line;
	double **temp,**temp1,**temp2,f;
	int flag=0;
	double s=0;
//-------------------------------------------------------	
	for(int r=0;r<n-1;r++){
		jpivo=r;
		pivo=U[jpivo][jpivo];
		
		if(fabs(pivo)<1e-5){
			//find the pivo
			for(int i=jpivo;i<n;i++){
				if (fabs(U[i][jpivo])>fabs(pivo)){
					pivo=U[i][jpivo];
					pivo_line=i;
					flag=1; 
				}	
			}
			
			if (pivo==0){
				printf("Singular Matrix\n");
				return 0;
			}
					
			if(flag==1){
				s++;
				//changing lines
				temp1=PI(jpivo,pivo_line,n);//actual permutation matrix
				temp2=mm(temp1,n,n,P,n,n);//total permutation matrix
				temp=mm(temp1,n,n,U,n,n);//actual matrix 
				for(int i=0;i<n;i++){
					for(int j=0;j<n;j++){
						U[i][j]=temp[i][j];
						P[i][j]=temp2[i][j];
					}
				}
				dalloc(temp,n);dalloc(temp1,n);dalloc(temp2,n);
			
			}
		
			flag=0;
		}
		
		//gaussian elimination
		for(int i=jpivo+1;i<n;i++){
			f=U[i][jpivo]/pivo;
			//L[i][jpivo]=f;
			for(int j=jpivo;j<n;j++){
				U[i][j]=U[i][j]-U[jpivo][j]*f;				
			}
		}	
	}
	
	double det=1;
	for(int i=0;i<n;i++){
		det*=U[i][i];
	}
	det=pow(-1,s)*det;
	//printf("determinant = %f\n",det);
	return det;
//------------------------------	
	

}


//inverse
double** inv(double **a,int n){
	if(det(a,n)==0){
		printf("Non-inversible matrix \n");
		return NULL;
	}
	if(n==2){
		double **C=alloc(n,n);
		double c=1/det(a,n);
		C[0][0]=a[1][1]*c;
		C[0][1]=-a[0][1]*c;
		C[1][0]=-a[1][0]*c;
		C[1][1]=a[0][0]*c;
		return C;
	}
	
	double **M;
	double **cof=alloc(n,n);
	
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			M=reduction(a,n,i,j);
			cof[i][j]=pow(-1,i+j)*detLU(M,n-1);
			dalloc(M,n-1);
		}
	}
	double **temp;
	temp=transpose(cof,n,n);
	double **r=mcm(1/detLU(a,n),temp,n,n);dalloc(temp,n);
	dalloc(cof,n);
	return r;
}


//mantissa and exponent
double log_2(double x){
	double f,e=0;
	while(true){
		f=x/pow(2,e);
		if(fabs(f)>=0.5 && fabs(f)<1){
			break;
		}
		else if(e>1023){
			e=1023;
			break;
		}
		else{
			e=e+1;
		}
	}
	return e;
}

double** prodq(double **qa,double **qb){
	double **q=alloc(4,4);
	q[0][0]=qb[3][0]; q[0][1]=qb[2][0]; q[0][2]=-qb[1][0]; q[0][3]=qb[0][0];
	q[1][0]=-qb[2][0]; q[1][1]=qb[3][0]; q[1][2]=qb[0][0]; q[1][3]=qb[1][0];
	q[2][0]=qb[1][0]; q[2][1]=-qb[0][0]; q[2][2]=qb[3][0]; q[2][3]=qb[2][0];
	q[3][0]=-qb[0][0]; q[3][1]=-qb[1][0]; q[3][2]=-qb[2][0]; q[3][3]=qb[3][0];
	//produit=[qb4 qb3 -qb2 qb1;
	//		-qb3 qb4 qb1 qb2 ;
	//		qb2 -qb1 qb4 qb3;
	//		-qb1 -qb2 -qb3 qb4]*[qa1; qa2;qa3;qa4];
	return mm(q,4,4,qa,4,1);
}

double** abs(double **A,int row,int col){
	double **a=alloc(row,col);
	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++){
			a[i][j]=fabs(A[i][j]);
		}
	}
	return a;
}
double trace(double **A,int dim){
	double sum=0;
	for(int i=0;i<dim;i++){
		for(int j=0;j<dim;j++){
			if(i==j){
				sum=sum+A[i][j];
			}
		}
	}
	assert(isnan(sum)==0);
	return sum;
}


// biggest row sum of the absolute terms of a matrix
double norminf(double **A,int n){
	//double *v=(double*) malloc(n*sizeof(double));
	double sum0=0,sum=0;
	for(int k=0;k<n;k++){
		sum0+=fabs(A[k][0]);
	}
	
	for(int i=1;i<n;i++){
		for(int j=0;j<n;j++){
			sum+=fabs(A[i][j]);
			//printf("sum %f\n",sum);
		}
		if(sum>sum0){
			sum0=sum;
			sum=0;
		}
	}
	assert(isnan(sum0)==0);
	return sum0;
}


double** sumM(double **A,double **B,int row, int col,double op){
	double **result=alloc(row,col);
	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++){
				result[i][j]=A[i][j]+op*B[i][j];
				assert(isnan(result[i][j])==0);			
			}		
		}
		return result;
}

//matrix exponential
double** expm(double **A, int n){
	double **I=eye(n);
	double **B=alloc(n,n);
	double **B2;
	double **x;
	double **y;
	double **r;
	double **temp,**temp1,**temp2;
	
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			B[i][j]=A[i][j];
		}
	}
	
	double N=(double) n;
	double trshift=trace(A,n)/N;
	
	if(trshift>0){
		temp=mcm(trshift,I,n,n);
		temp1=sumM(A,temp,n,n,-1);dalloc(temp,n);
	
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			B[i][j]=temp1[i][j];
		}
	}
	dalloc(temp1,n);	
	}
	
	double e=log_2(norminf(B,n));
	
	double s=0;
	if(e>s){
		s=e;
	}
	if(s>1023){
		s=1023;
	}
	
	temp=mcm(pow(2,-s),B,n,n);
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			B[i][j]=temp[i][j];
		}
	}
	dalloc(temp,n);
	

	double c[] = {5.0000000000000000e-1,1.1666666666666667e-1,1.6666666666666667e-2,1.6025641025641026e-3,1.0683760683760684e-4,4.8562548562548563e-6,1.3875013875013875e-7,1.9270852604185938e-9};
	B2=mm(B,n,n,B,n,n);
	
	temp=mcm(c[7],B2,n,n);
	temp1=mcm(c[5],I,n,n);
	temp2=sumM( temp, temp1,n,n,1); dalloc(temp1,n);dalloc(temp,n);
	temp=mm(temp2,n,n,B2,n,n);dalloc(temp2,n);
	temp1=mcm(c[3],I,n,n);
	temp2=sumM(temp,temp1,n,n,1);dalloc(temp1,n);dalloc(temp,n);
	temp=mm(temp2,n,n,B2,n,n);dalloc(temp2,n);
	temp1=mcm(c[1], I,n,n);
	temp2=sumM(temp, temp1,n,n,1);dalloc(temp1,n);dalloc(temp,n);
	temp=mm(temp2,n,n,B2,n,n);dalloc(temp2,n);
	x = sumM(temp, I,n,n,1); dalloc(temp,n);
	
	
	temp=mcm(c[6],B2,n,n);
	temp1=mcm(c[4],I,n,n);
	temp2=sumM( temp, temp1,n,n,1);dalloc(temp1,n);dalloc(temp,n);
	temp=mm(temp2,n,n,B2,n,n);dalloc(temp2,n);
	temp1=mcm(c[2],I,n,n);
	temp2=sumM(temp,temp1,n,n,1);dalloc(temp1,n);dalloc(temp,n);
	temp=mm(temp2,n,n,B2,n,n);
	temp1=mcm(c[0], I,n,n);
	temp2=sumM(temp, temp1,n,n,1);dalloc(temp1,n);dalloc(temp,n);
	y =mm(temp2,n,n, B,n,n);dalloc(temp2,n);
  
  
  	temp=sumM(x,y,n,n,-1);
  	temp1=inv(temp,n);
  	dalloc(temp,n);
  	temp=sumM(x,y,n,n,1);
  	r = mm(temp1,n,n,temp,n,n);dalloc(temp,n);dalloc(temp1,n);
	
	
	for(int k=0;k<int(s);k++){
		temp1=mm(r,n,n,r,n,n);
		for(int i=0;i<n;i++){
			for(int j=0;j<n;j++){
				r[i][j]=temp1[i][j];
			}
		}
		dalloc(temp1,n);
	}
	
	
	if (trshift > 0){
		temp=mcm(exp(trshift),r,n,n);
	    for(int i=0;i<n;i++){
			for(int j=0;j<n;j++){
				r[i][j]=temp[i][j];
			}
		}
		dalloc(temp,n);
	}
	
	
	dalloc(I,n);dalloc(B,n);dalloc(B2,n);dalloc(x,n);dalloc(y,n);
	return r;
}//--------------
//------------------------------------------------------------------------------------------------------------------

//enter with what column you want and how many lines
double** sep_col(double **A,int op_col,int row){
	double **r=alloc(row,1);
	for(int i=0;i<row;i++){
			r[i][0]=A[i][op_col];
			assert(isnan(r[i][0])==0);
	}
	return r;
}

//enter with what line you want and how many columns
double** sep_row(double **A,int op_row,int col){
	double **r=alloc(col,1);
	for(int i=0;i<col;i++){
			r[i][0]=A[op_row][i];
			assert(isnan(r[i][0])==0);
	}
	return r;
}


double dot(double** v1,double **v2,int dim){
	
	double sum=0;
	for(int i=0;i<dim;i++){
		sum+=v1[i][0]*v2[i][0];
	}
	assert(isnan(sum)==0);
	return sum;
}


double** rotation_matrix(double **e,double e0){
//A=(e0^2-e'*e)*eye(3)+2*e*e'-2*e0*skew(e);

double **A=alloc(3,3); //rotation matrix 
double **I=eye(3);
double **temp,**temp1,**temp2;
double **S=skew(e);
double **et=transpose(e,3,1);
temp=mm(e,3,1,et,1,3);
temp1=mcm(2,temp,3,3);dalloc(temp,3);
temp=mcm(e0*e0-dot(e,e,3),I,3,3);
temp2=sumM(temp,temp1,3,3,1);dalloc(temp,3);dalloc(temp1,3);
temp=mcm(-2*e0,S,3,3);
A=sumM(temp2,temp,3,3,1);dalloc(temp,3);dalloc(temp2,3);

dalloc(I,3);dalloc(S,3);dalloc(et,1);
return A;
}



double** E1_gen(double **xm_vect,double xm0){
	int i,j;
	//[skew(y_vect)+y0*eye(3);-y_vect']
	double **E1=alloc(4,3);
	double **v,**temp,**temp1;
	
	temp=skew(xm_vect);
	temp1=mcm(xm0,eye(3),3,3);
	v=sumM(temp,temp1,3,3,1);dalloc(temp,3);dalloc(temp1,3);
	for(int i=0;i<4;i++){
		for(int j=0;j<3;j++){
			if(i<3){
				E1[i][j]=v[i][j];
				assert(isnan(E1[i][j])==0);
			}else{
				E1[i][j]=-xm_vect[j][0];
				assert(isnan(E1[i][j])==0);
			}
		}
	}
	dalloc(v,3);
	return E1;
}

double** H_bar_gen(double** H, double** H2){
	double **H_bar=alloc(8,4);
	for(int i=0;i<8;i++){
		for(int j=0;j<4;j++){
			if(i<4){
				H_bar[i][j]=H[i][j];
				assert(isnan(H_bar[i][j])==0);
			}else{
				H_bar[i][j]=H2[(i-4)][j];
				assert(isnan(H_bar[i][j])==0);
			}
			
		}
	}
	return H_bar;
}

double** R_bar_gen(double** R1, double** R2){
	double **R_bar=alloc(8,8);
	//[R1 zeros(4,4);zeros(4,4) R2]; % covariance totale (magn�to+accelero)
	for(int i=0;i<8;i++){
		for(int j=0;j<8;j++){
			if(i<4 && j<4){
				R_bar[i][j]=R1[i][j];
				assert(isnan(R_bar[i][j])==0);
			}else if(i<4 && j>3){
				R_bar[i][j]=0;
				assert(isnan(R_bar[i][j])==0);
			}else if(i>3 && j<4){
				R_bar[i][j]=0;
				assert(isnan(R_bar[i][j])==0);
			}else if(i>3 && j>3){
				R_bar[i][j]=R2[(i-4)][j-4];
				assert(isnan(R_bar[i][j])==0);
			}
		}
	}
	return R_bar;
}

double** testGolden(int n,int test,int number_tests){
	
 //ler o arquivo
 
  FILE *myfile;
  double myvariable;
  double **golden=alloc(n,24),**golden_aux=alloc(n*number_tests,24);
  int i;
  int j;
  

  myfile=fopen("TestGolden.txt", "r");
  if(myfile==NULL){
	  printf("Impossible to open golden\n");
	  //exit(0);
	  
	  }

  for(i = 0; i < n*number_tests; i++){
    for (j = 0 ; j < 24; j++)
    {
    	fscanf(myfile,"%lf",&golden_aux[i][j]);
   		if(i>=n*test && i<n*test+n){
      		golden[i-n*test][j]=golden_aux[i][j];
  		}
      //printf("%f ",noise[i][j]);
    }  
    //printf("\n");
  }
  dalloc(golden_aux,n*number_tests);
  fclose(myfile);
  return golden;

}

double** get_gyros(int n, int iteration, int number_of_tests){

//ler o arquivo
 
  FILE *myfile;
  double **gyros=alloc(3,1);
  int i;
  int j;
  double aux;
  
  myfile=fopen("./sensors_data/gyros.txt", "r");
  
  for(i=0;i<n*number_of_tests;i++){
  	for(j=0;j<3;j++){		
  		if(i==iteration){
  			fscanf(myfile,"%lf",&gyros[j][0]);
		}else{
			fscanf(myfile,"%lf",&aux);
		}
	  }
  }
  
  fclose(myfile);
  return gyros;
	
}

double** get_mag(int n, int iteration, int number_of_tests){

//ler o arquivo
 
  FILE *myfile;
  double **gyros=alloc(3,1);
  int i;
  int j;
  double aux;
  
  myfile=fopen("./sensors_data/magnetometer.txt", "r");
  
  for(i=0;i<n*number_of_tests;i++){
  	for(j=0;j<3;j++){		
  		if(i==iteration){
  			fscanf(myfile,"%lf",&gyros[j][0]);
		}else{
			fscanf(myfile,"%lf",&aux);
		}
	  }
  }
  
  fclose(myfile);
  return gyros;
	
}

double** get_acc(int n, int iteration, int number_of_tests){

//ler o arquivo
 
  FILE *myfile;
  double **gyros=alloc(3,1);
  int i;
  int j;
  double aux;
  
  myfile=fopen("./sensors_data/accelerometer.txt", "r");
  
  for(i=0;i<n*number_of_tests;i++){
  	for(j=0;j<3;j++){		
  		if(i==iteration){
  			fscanf(myfile,"%lf",&gyros[j][0]);
		}else{
			fscanf(myfile,"%lf",&aux);
		}
	  }
  }
  
  fclose(myfile);
  return gyros;
	
}



double** noise_G(int n,int test,int number_tests){
	
 //ler o arquivo
 
  FILE *myfile;
  double myvariable;
  double **noise=alloc(3,n),**noise_aux=alloc(3*number_tests,n);
  int i;
  int j;
  

  myfile=fopen("noise_G.txt", "r");

  for(i = 0; i < 3*number_tests; i++){
    for (j = 0 ; j < n; j++)
    {
    	fscanf(myfile,"%lf",&noise_aux[i][j]);
   		if(i>=3*test && i<3*test+3){
      		noise[i-3*test][j]=noise_aux[i][j];
  		}
      //printf("%f ",noise[i][j]);
    }  
    //printf("\n");
  }
  dalloc(noise_aux,3*number_tests);
  fclose(myfile);
  return noise;

}


double** noise_acc(int n,int test,int number_tests){
	
 //ler o arquivo
  FILE *myfile;
  double myvariable;
  double **noise=alloc(3,n),**noise_aux=alloc(3*number_tests,n);
  int i;
  int j;

  myfile=fopen("noise_acc.txt", "r");

  for(i = 0; i < 3*number_tests; i++){
    for (j = 0 ; j < n; j++)
    {
    	fscanf(myfile,"%lf",&noise_aux[i][j]);
   		if(i>=3*test && i<3*test+3){
      		noise[i-3*test][j]=noise_aux[i][j];
  		}
      //printf("%f ",noise[i][j]);
    }
    //printf("\n");
  }
	dalloc(noise_aux,3*number_tests);
  
  fclose(myfile);
  return noise;

}


double** noise_mg(int n,int test,int number_tests){
	
 //ler o arquivo
  FILE *myfile;
  double myvariable;
  double **noise=alloc(3,n),**noise_aux=alloc(3*number_tests,n);
  int i;
  int j;

  myfile=fopen("noise_mg.txt", "r");

  for(i = test; i < 3+test; i++){
    for (j = 0 ; j < n; j++)
    {
    	fscanf(myfile,"%lf",&noise_aux[i][j]);
   		if(i>=3*test && i<3*test+3){
      		noise[i-3*test][j]=noise_aux[i][j];
  		}
      //printf("%f ",noise[i][j]);
    }
    //printf("\n");
  }
	dalloc(noise_aux,3*number_tests);
  fclose(myfile);
  return noise;

}



