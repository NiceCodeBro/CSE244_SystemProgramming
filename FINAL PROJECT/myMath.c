/*
REFERANCES
https://github.com/chris-wood/llvm-optimized/blob/master/applications/qr_decomposition.c
http://www.programming-techniques.com/2011/09/numerical-methods-inverse-of-nxn-matrix.html
https://www.youtube.com/watch?v=ivSdbpFan2k
http://www.math.umd.edu/~mariakc/teaching-2/householder.c
http://cacs.usc.edu/education/phys516/src/TB/svdcmp.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 

#define SIGN(a,b) ((b) > 0.0 ? fabs(a) : - fabs(a))

static double maxarg1,maxarg2;
#define FMAX(a,b) (maxarg1 = (a),maxarg2 = (b),(maxarg1) > (maxarg2) ? (maxarg1) : (maxarg2))

static int iminarg1,iminarg2;
#define IMIN(a,b) (iminarg1 = (a),iminarg2 = (b),(iminarg1 < (iminarg2) ? (iminarg1) : iminarg2))

static double sqrarg;
#define SQR(a) ((sqrarg = (a)) == 0.0 ? 0.0 : sqrarg * sqrarg)

typedef struct {
	int m, n;
	double ** v;
} mat_t, *mat;
typedef struct {
	int m, n;
	double v[40][40];
} matt;
mat matrix_new(int m, int n)
{
	int i;
	mat x = malloc(sizeof(mat_t));
	x->v = malloc(sizeof(double) * m);
	x->v[0] = calloc(sizeof(double), m * n);
	for (i = 0; i < m; i++)
		x->v[i] = x->v[0] + n * i;
	x->m = m;
	x->n = n;
	return x;
}
 
void matrix_delete(mat m)
{
	free(m->v[0]);
	free(m->v);
	free(m);
}

void matrix_transpose(mat m){
	double temp[40][40];
	int i,j;
	for ( i = 0; i < m->m; ++i)
	{
		for ( j = 0; j < m->n; ++j)
		{
			temp[j][i]=m->v[i][j];

		}
	}
	for ( i = 0; i < m->m; ++i)
	{
		for ( j = 0; j < m->n; ++j)
		{
			m->v[i][j]=temp[i][j];
		}
	}
}
mat matrix_copy(double a[][40], int m, int n)
{
	int i,j;
	mat x = matrix_new(m, n);
	for (i = 0; i < m; i++)
		for (j = 0; j < n; j++)
			x->v[i][j] = a[i][j];
	return x;
}
 
mat matrix_mul(mat x, mat y)
{
	int i,j,k;
	if (x->n != y->m) return 0;
	mat r = matrix_new(x->m, y->n);
	for (i = 0; i < x->m; i++)
		for (j = 0; j < y->n; j++)
			for (k = 0; k < x->n; k++)
				r->v[i][j] += x->v[i][k] * y->v[k][j];
	return r;
}
 
mat matrix_minor(mat x, int d)
{
	int i,j;
	mat m = matrix_new(x->m, x->n);
	for ( i = 0; i < d; i++)
		m->v[i][i] = 1;
	for (i = d; i < x->m; i++)
		for (j = d; j < x->n; j++)
			m->v[i][j] = x->v[i][j];
	return m;
}
 
/* c = a + b * s */
double *vmadd(double a[], double b[], double s, double c[], int n)
{
	int i;
	for (i = 0; i < n; i++)
		c[i] = a[i] + s * b[i];
	return c;
}
 
/* m = I - v v^T */
mat vmul(double v[], int n)
{
	int i,j;
	mat x = matrix_new(n, n);
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			x->v[i][j] = -2 *  v[i] * v[j];
	for (i = 0; i < n; i++)
		x->v[i][i] += 1;
 
	return x;
}
 
/* ||x|| */
double vnorm(double x[], int n)
{
	int i;
	double sum = 0;
	for (i = 0; i < n; i++) sum += x[i] * x[i];
	return sqrt(sum);
}
 
/* y = x / d */
double* vdiv(double x[], double d, double y[], int n)
{
	int i;
	for (i = 0; i < n; i++) y[i] = x[i] / d;
	return y;
}
 
/* take c-th column of m, put in v */
double* mcol(mat m, double *v, int c)
{
	int i ;
	for (i = 0; i < m->m; i++)
		v[i] = m->v[i][c];
	return v;
}
 
void matrix_show(mat m)
{
	int i ,j;
	for( i = 0; i < m->m; i++) {
		for (j = 0; j < m->n; j++) {
			printf(" %8.3f", m->v[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}
 
void householder(mat m, mat *R, mat *Q)
{
	int i,k;	
	mat q[m->m];
	mat z = m, z1;
	for ( k = 0; k < m->n && k < m->m - 1; k++) {
		double e[m->m], x[m->m], a;
		z1 = matrix_minor(z, k);
		if (z != m) matrix_delete(z);
		z = z1;
 
		mcol(z, x, k);
		a = vnorm(x, m->m);
		if (m->v[k][k] > 0) a = -a;
 
		for ( i = 0; i < m->m; i++)
			e[i] = (i == k) ? 1 : 0;
 
		vmadd(x, e, a, e, m->m);
		vdiv(e, vnorm(e, m->m), e, m->m);
		q[k] = vmul(e, m->m);
		z1 = matrix_mul(q[k], z);
		if (z != m) matrix_delete(z);
		z = z1;
	}
	matrix_delete(z);
	*Q = q[0];
	*R = matrix_mul(q[0], m);
	for (i = 1; i < m->n && i < m->m - 1; i++) {
		z1 = matrix_mul(q[i], *Q);
		if (i > 1) matrix_delete(*Q);
		*Q = z1;
		matrix_delete(q[i]);
	}
	matrix_delete(q[0]);
	z = matrix_mul(*Q, m);
	matrix_delete(*R);
	*R = z;
	matrix_transpose(*Q);
}

void giveTranspose(matt *main,matt* tran)
{
	int i,j;
	
    for(i=0; i<main -> m; ++i)
        for(j=0; j<main -> n; ++j)
        {
            tran->v[j][i] = main->v[i][j];
        }
      
    tran->m = main -> n;
    tran->n = main -> m;
        
}

void matrisCpy(mat m, matt *mt)
{
	int i,j;
	for(i = 0; i < m->m; ++i)
	{
		for(j = 0; j < m->n; ++j)
		{
			mt->v[i][j] = m->v[i][j]; 
		}
	}
	
	mt->m = m->m;
	mt->n = m->n;
	
}
void matrixMultiplication2(double m1[40][40], double m2[40][40], int r,int c, int r2, int c2, matt *result)
{
	int i,j,k;
	int temp = 0;

    for(i=0; i<r; ++i)
        for(j=0; j<c2; ++j)
            for(k=0; k<c; ++k)
            {
                result->v[i][j]+=m1[i][k]*m2[k][j];
            }
            
    result-> m = r;
    result-> n = c2;

}


//http://www.programming-techniques.com/2011/09/numerical-methods-inverse-of-nxn-matrix.html
void inverseOfNxNmatrix(double m1[40][40], int n)
{

    double ratio,a;
    int i, j, k;

    for(i = 0; i < n; i++){
        for(j = n; j < 2*n; j++){
            if(i==(j-n))
                m1[i][j] = 1.0;
            else
                m1[i][j] = 0.0;
        }
    }
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            if(i!=j){
                ratio = m1[j][i]/m1[i][i];
                for(k = 0; k < 2*n; k++){
                    m1[j][k] -= ratio * m1[i][k];
                }
            }
        }
    }
    for(i = 0; i < n; i++){
        a = m1[i][i];
        for(j = 0; j < 2*n; j++){

            m1[i][j] /= a;
        }
    }
    
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
			m1[i][j] = m1[i][j+n];
			m1[i][j+n] = 0;
        }
    }
    
}

void matrisTranspose(double mainMatrix[40][40], double transposeMatrix[40][40], int r,int c)
{
	int i,j;
	
    for(i=0; i<r; ++i)
        for(j=0; j<c; ++j)
        {
            transposeMatrix[j][i] = mainMatrix[i][j];
        }
    
}


// calculates sqrt( a^2 + b^2 ) with decent precision
double pythag(double a, double b) {
  double absa,absb;

  absa = fabs(a);
  absb = fabs(b);

  if(absa > absb)
    return(absa * sqrt(1.0 + SQR(absb/absa)));
  else
    return(absb == 0.0 ? 0.0 : absb * sqrt(1.0 + SQR(absa / absb)));
}

/*
  Modified from Numerical Recipes in C
  Given a matrix a[nRows][nCols], svdcmp() computes its singular value 
  decomposition, A = U * W * Vt.  A is replaced by U when svdcmp 
  returns.  The diagonal matrix W is output as a vector w[nCols].
  V (not V transpose) is output as the matrix V[nCols][nCols].
*/
int svdcmp(double a[40][40], int nRows, int nCols, double w[40], double v[40][40]) {
  int flag,i,its,j,jj,k,l,nm;
  double anorm,c,f,g,h,s,scale,x,y,z,*rv1;

  rv1 = malloc(sizeof(double)*nCols);
  if(rv1 == NULL) {
  	
  	return(-1);
  }

  g = scale = anorm = 0.0;
  for(i=0;i<nCols;i++) {
    l = i+1;
    rv1[i] = scale*g;
    g = s = scale = 0.0;
    if(i < nRows) {
      for(k=i;k<nRows;k++) scale += fabs(a[k][i]);
      if(scale) {
	for(k=i;k<nRows;k++) {
	  a[k][i] /= scale;
	  s += a[k][i] * a[k][i];
	}
	f = a[i][i];
	g = -SIGN(sqrt(s),f);
	h = f * g - s;
	a[i][i] = f - g;
	for(j=l;j<nCols;j++) {
	  for(s=0.0,k=i;k<nRows;k++) s += a[k][i] * a[k][j];
	  f = s / h;
	  for(k=i;k<nRows;k++) a[k][j] += f * a[k][i];
	}
	for(k=i;k<nRows;k++) a[k][i] *= scale;
      }
    }
    w[i] = scale * g;
    g = s = scale = 0.0;
    if(i < nRows && i != nCols-1) {
      for(k=l;k<nCols;k++) scale += fabs(a[i][k]);
      if(scale)  {
	for(k=l;k<nCols;k++) {
	  a[i][k] /= scale;
	  s += a[i][k] * a[i][k];
	}
	f = a[i][l];
	g = - SIGN(sqrt(s),f);
	h = f * g - s;
	a[i][l] = f - g;
	for(k=l;k<nCols;k++) rv1[k] = a[i][k] / h;
	for(j=l;j<nRows;j++) {
	  for(s=0.0,k=l;k<nCols;k++) s += a[j][k] * a[i][k];
	  for(k=l;k<nCols;k++) a[j][k] += s * rv1[k];
	}
	for(k=l;k<nCols;k++) a[i][k] *= scale;
      }
    }
    anorm = FMAX(anorm, (fabs(w[i]) + fabs(rv1[i])));


    fflush(stdout);
  }

  for(i=nCols-1;i>=0;i--) {
    if(i < nCols-1) {
      if(g) {
	for(j=l;j<nCols;j++)
	  v[j][i] = (a[i][j] / a[i][l]) / g;
	for(j=l;j<nCols;j++) {
	  for(s=0.0,k=l;k<nCols;k++) s += a[i][k] * v[k][j];
	  for(k=l;k<nCols;k++) v[k][j] += s * v[k][i];
	}
      }
      for(j=l;j<nCols;j++) v[i][j] = v[j][i] = 0.0;
    }
    v[i][i] = 1.0;
    g = rv1[i];
    l = i;

    fflush(stdout);
  }

  for(i=IMIN(nRows,nCols) - 1;i >= 0;i--) {
    l = i + 1;
    g = w[i];
    for(j=l;j<nCols;j++) a[i][j] = 0.0;
    if(g) {
      g = 1.0 / g;
      for(j=l;j<nCols;j++) {
	for(s=0.0,k=l;k<nRows;k++) s += a[k][i] * a[k][j];
	f = (s / a[i][i]) * g;
	for(k=i;k<nRows;k++) a[k][j] += f * a[k][i];
      }
      for(j=i;j<nRows;j++) a[j][i] *= g;
    }
    else
      for(j=i;j<nRows;j++) a[j][i] = 0.0;
    ++a[i][i];

    fflush(stdout);
  }

  for(k=nCols-1;k>=0;k--) {
    for(its=0;its<30;its++) {
      flag = 1;
      for(l=k;l>=0;l--) {
	nm = l-1;
	if((fabs(rv1[l]) + anorm) == anorm) {
	  flag =  0;
	  break;
	}
	if((fabs(w[nm]) + anorm) == anorm) break;
      }
      if(flag) {
	c = 0.0;
	s = 1.0;
	for(i=l;i<=k;i++) {
	  f = s * rv1[i];
	  rv1[i] = c * rv1[i];
	  if((fabs(f) + anorm) == anorm) break;
	  g = w[i];
	  h = pythag(f,g);
	  w[i] = h;
	  h = 1.0 / h;
	  c = g * h;
	  s = -f * h;
	  for(j=0;j<nRows;j++) {
	    y = a[j][nm];
	    z = a[j][i];
	    a[j][nm] = y * c + z * s;
	    a[j][i] = z * c - y * s;
	  }
	}
      }
      z = w[k];
      if(l == k) {
	if(z < 0.0) {
	  w[k] = -z;
	  for(j=0;j<nCols;j++) v[j][k] = -v[j][k];
	}
	break;
      }
      if(its == 29) printf("no convergence in 30 svdcmp iterations\n");
      x = w[l];
      nm = k-1;
      y = w[nm];
      g = rv1[nm];
      h = rv1[k];
      f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
      g = pythag(f,1.0);
      f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g,f))) - h)) / x;
      c = s = 1.0;
      for(j=l;j<=nm;j++) {
	i = j+1;
	g = rv1[i];
	y = w[i];
	h = s * g;
	g = c * g;
	z = pythag(f,h);
	rv1[j] = z;
	c = f/z;
	s = h/z;
	f = x * c + g * s;
	g = g * c - x * s;
	h = y * s;
	y *= c;
	for(jj=0;jj<nCols;jj++) {
	  x = v[jj][j];
	  z = v[jj][i];
	  v[jj][j] = x * c + z * s;
	  v[jj][i] = z * c - x * s;
	}
	z = pythag(f,h);
	w[j] = z;
	if(z) {
	  z = 1.0 / z;
	  c = f * z;
	  s = h * z;
	}
	f = c * g + s * y;
	x = c * y - s * g;
	for(jj=0;jj < nRows;jj++) {
	  y = a[jj][j];
	  z = a[jj][i];
	  a[jj][j] = y * c + z * s;
	  a[jj][i] = z * c - y * s;
	}
      }
      rv1[l] = 0.0;
      rv1[k] = f;
      w[k] = x;
    }

    fflush(stdout);
  }

  
  free(rv1);
  
  return(0);
}


void matrixMultiplication(double a[40][40],double b[40][40],int r,int c,int r2,int c2,matt *res){

    int i,j,k;
    if(c!=r2)     	return; 
    
    for(i=0; i<r; ++i)
        for(j=0; j<c2; ++j)
        {
            res->v[i][j] = 0;
        }
     

  
    for(i=0; i<r; ++i)
        for(j=0; j<c2; ++j)
            for(k=0; k<c; ++k)
            {
                res->v[i][j]+=a[i][k]*b[k][j];
            }
    
    res->m = r;
    res->n = c2;
}


void matrisTranspose2(double arr[40][40],int r,int c){
	double tempMatris[40][40];
	int i,j;
	for ( i = 0; i < r; ++i)
		for ( j = 0; j < c; ++j)
			tempMatris[j][i]=arr[i][j];
	
	for ( i = 0; i < r; ++i)
		for ( j = 0; j < c; ++j)
			arr[i][j]=tempMatris[i][j];
		
	
}
void vectorToMatrisW(double res[40][40], double w[40], int r, int c){

	int i,j,index = 0;
	for (i = 0; i < r; ++i)
		for ( j = 0; j < c; ++j)
			res[i][j]=0;
	
	for (i = 0; i < c; ++index, ++i)
		res[index][i]=w[i];

}



void arrCpy(double arrA[40][40], double arrB[40][40], int r, int c)
{
	int i,j;
	for(i = 0; i < r; ++i)
		for(j = 0; j < c; ++j)
			arrA[i][j] = arrB[i][j];
}
void arrCpy2(double arrA[40][40], double arrB[40], int r, int c)
{
	int i,j;
	for(i = 0; i < r; ++i)
		for(j = 0; j < c; ++j)
			arrA[i][j] = arrB[i];
}



