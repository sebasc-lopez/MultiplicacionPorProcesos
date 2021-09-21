#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

double **MatA;
double **MatB;
double **MatC;
int row;
int NumProcesos;

void llenarMatriz(int col, int row)
{
  int i, j;
  for (i = 0; i < row; i++)
  {
    for (j = 0; j < col; j++)
    {
      MatA[i][j] = rand() % 9;
    }
  }

  for (i = 0; i < row; i++)
  {
    for (j = 0; j < col; j++)
    {
      MatB[i][j] = rand() % 9;
    }
  }
}

int mulmat(int id_arg)
{
  int i, j, k;
  long id = (long)id_arg;
  int filasxHilo = row / NumProcesos;
  int inicio = id * filasxHilo;
  int final;
  if (id + 1 == NumProcesos)
  {
    final = row;
  }
  else
  {
    final = (id + 1) * filasxHilo;
  }
  for (i = inicio; i < final; i++)
  {
    for (j = 0; j < row; j++)
    {
      for (k = 0; k < row; k++)
      {
        MatC[i][j] += MatA[i][k] * MatB[k][j];
      }
    }
  }
  return 0;
}

void print_result(int col, int row)
{
  int i, j;

  printf("Matriz A : \n");
  for (i = 0; i < row; i++)
  {
    for (j = 0; j < col; j++)
    {
      printf("%.0lf ", MatA[i][j]);
    }
    printf("\n");
  }

  printf("Matriz B : \n");
  for (i = 0; i < row; i++)
  {
    for (j = 0; j < col; j++)
    {
      printf("%.0lf ", MatB[i][j]);
    }
    printf("\n");
  }

  printf("Matriz C : \n");
  for (i = 0; i < row; i++)
  {
    for (j = 0; j < col; j++)
    {
      printf("%.0lf ", MatC[i][j]);
    }
    printf("\n");
  }
}

int main(int argc, char *argv[])
{

  srand(time(NULL));
  row = strtol(argv[1], NULL, 10); // Number of elements for the array
  NumProcesos = strtol(argv[2], NULL, 10);
  MatA = mmap(NULL, row * sizeof(double),
              PROT_READ | PROT_WRITE,
              MAP_SHARED | MAP_ANONYMOUS,
              0, 0);

  MatB = mmap(NULL, row * sizeof(double),
              PROT_READ | PROT_WRITE,
              MAP_SHARED | MAP_ANONYMOUS,
              0, 0);

  MatC = mmap(NULL, row * sizeof(double),
              PROT_READ | PROT_WRITE,
              MAP_SHARED | MAP_ANONYMOUS,
              0, 0);

  if (MatC == MAP_FAILED || MatA == MAP_FAILED || MatB == MAP_FAILED)
  {
    printf("Mapping Failed\n");
    return 1;
  }

  int i, j;

  for (i = 0; i < row; i++)
  {
    MatA[i] = mmap(NULL, row * sizeof(double),
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS,
                   0, 0);
  }

  for (i = 0; i < row; i++)
  {
    MatB[i] = mmap(NULL, row * sizeof(double),
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS,
                   0, 0);
  }

  for (i = 0; i < row; i++)
  {
    MatC[i] = mmap(NULL, row * sizeof(double),
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS,
                   0, 0);
  }

  llenarMatriz(row, row);

  pid_t pid;
  int status;

  struct timeval inicio;
  struct timeval fin;
  double milisegundosFinal;
  long segundos, milisegundos;

  gettimeofday(&inicio, 0);

  for (i = 0; i < NumProcesos; i++)
  {
    pid = fork();
    if (pid > 0)
    {
      continue;
    }
    else if (pid == 0)
    {
      exit(mulmat(i));
    }
  }

  for (i = 0; i < NumProcesos; i++)
  {
    wait(&status);
  }

  gettimeofday(&fin, 0);

  segundos = fin.tv_sec - inicio.tv_sec;
  milisegundos = fin.tv_usec - inicio.tv_usec;
  milisegundosFinal = ((segundos)*1000 + milisegundos / 1000.0);

  printf("Tiempo de ejecucion:\t");
  printf("%.16g milisegundos\n", milisegundosFinal);


  FILE *archivo;
  archivo = fopen("documento.txt", "a");
  fprintf(archivo, "%d\n", row);
  fprintf(archivo, "%.16g\n", milisegundosFinal);

  //print_result(row, row);
  return 0;
}