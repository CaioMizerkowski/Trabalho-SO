#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

   int i, j, k, max;
   int vet[100];

   if (argc!=2) {
      printf ("\nOps %d. Errou o comando: \n      %s <numero maximo de interacoes> \n\n", argc,argv[0]);
      exit (0);
   }
   max = atoi(argv[1]);

   for (i=0;i<max;i++)
      for (j=0;j<max;j++)
          for (k=0;k<max;k++)
              vet [ (i + j + k) % 100] = i*j*k;

}
