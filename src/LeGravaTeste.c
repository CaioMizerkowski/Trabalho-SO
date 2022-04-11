#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_LEN 1000
#define STR_LEN 1024

char    msg [ARRAY_LEN][ARRAY_LEN];
int     ditados=0;

void LeDitado(const char *str)
{
    FILE *arq;
    ditados = 0;
    if ( (arq=fopen(str,"r")) == NULL ) { printf("\n Erro lendo arquivo ...\n\n");exit(0);}
    while (!feof(arq)) {

       fgets(msg[ditados],ARRAY_LEN,arq);
       // para debug
       printf("%d %s",ditados,msg[ditados]);
       ditados=(ditados+1)%ARRAY_LEN;
    }
    printf("\n\nCarregou %d ditados",ditados);

}

void GravaDitado(const char *str)
{
    FILE *arq;
    if ( (arq=fopen(str,"w")) == NULL ) { printf("\n Erro lendo arquivo ...\n\n");exit(0);}

    for(int i = 0; i < ditados; i++) {
        fputs(msg[i], arq);
        // para debug
        printf("%d %s",i,msg[i]);
    }
    printf("\n\nSalvou %d ditados",ditados);
}


int main(int argc, char **argv)
{
    char str[1000] = "Ditados33.txt"; 
    char str2[1000] = "Ditados33.txt"; 
        LeDitado(str);
        GravaDitado(str2);
    exit(0);
}