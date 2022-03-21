// Universidade Federal do Paraná
// TE355 - Sistemas Operacionais Embarcados
// Base para o Trabalho 1 - 2022
// Prof. Pedroso

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

#define PROTOPORT       5193            /* default protocol port number */
#define QLEN            6               /* size of request queue        */

int     visits      =   0;              /* counts client connections    */
char    msg [1000][1000];
int     ditados=0;

        
void LeDitado()
{
    FILE *arq;
 
    if ( (arq=fopen("Ditados.txt","r")) == NULL ) { printf("\n Erro lendo arquivo ...\n\n");exit(0);}
    while (!feof(arq)) {
       
       fgets(msg[ditados],1000,arq); 
       // para debug
       // printf("%d %s",ditados,msg[ditados]);
       ditados=(ditados+1)%1000;
    }
    printf("\n\nCarregou %d ditados",ditados);   
     
}

char uppercase(char *input) {
    char i=0;
    for(i = 0; (i<strlen(input)) && (i<1000); i++) {
        input[i] = toupper(input[i]);
    }
    return i;
}

void *atendeConexao( void *sd2 )
{
    int *temp=sd2;
    int sd=*temp;
    char str[1024], *endptr;
        
    int i=0, b, val;

        while (1) {
        fflush(stdout);
        visits++;
        sprintf(str,"\nRequisição %d \n", visits);
        send(sd,str,strlen(str),0);
                     
        memset(str, 0, sizeof(str));
        b=recv(sd,str,sizeof(str),0);
                str[b]=0;
                printf("\nComando recebido:%s",str);
     
        uppercase(str);
        
        if (!strncmp(str,"GETR",4)) {             
            memset(str, 0, sizeof(str));
            sprintf(str,"\nDitado %d: %s ", visits%ditados, msg[visits%ditados]);
            send(sd,str,sizeof(str),0);
        }
        else if (!strncmp(str,"GETN",4)) {
            memset(str, 0, sizeof(str));
            b=recv(sd,str,sizeof(str),0);
            str[b]=0;
            val = strtol(str, &endptr, 10);
            if (endptr==str){
                sprintf(str,"\nFALHA");
                continue;
                }
            else{ 
                send(sd,msg[val],strlen(msg[val]),0);
            }  
        }
        else if (!strncmp(str,"REPLACE",7)) {             
            memset(str, 0, sizeof(str));
            b=recv(sd,str,sizeof(str),0);
                    str[b]=0;
                    val = strtol(str, &endptr, 10);
                    if (endptr==str)  {sprintf(str,"\nFALHA");continue;}
                    else sprintf(str,"\nOK");
            send(sd,str,strlen(str),0);
            b=recv(sd,str,sizeof(str),0);
                    str[b]=0;
                    strcpy(msg[val],str);
                    sprintf(str,"\nOK");
            send(sd,str,strlen(str),0);  
                    printf("\nNovo ditado %d: %s",val,msg[val]);
                }
        else if (!strncmp(str,"FIM",3)) {             
            memset(str, 0, sizeof(str));
            sprintf(str,"\nAté Logo");
            send(sd,str,strlen(str),0);
                    break;
                }
        else if (!strncmp(str,"VER",3)) {             
            memset(str, 0, sizeof(str));
            sprintf(str,"\nServidor de Ditados 2.0 Beta.\nTE355 2022 Primeiro Trabalho");
            send(sd,str,strlen(str),0);
           }
        else if (!strncmp(str,"\n",1)) {             
            memset(str, 0, sizeof(str));
            sprintf(str,"\n");
        }
        else{
            memset(str, 0, sizeof(str));
            sprintf(str,"\nErro de Protocolo");
            send(sd,str,strlen(str),0);
           }
       }
       close(sd);
}

int main(int argc, char **argv)
{
    struct  protoent *ptrp;  /* pointer to a protocol table entry   */
    struct  sockaddr_in sad; /* structure to hold server's address  */
    struct  sockaddr_in cad; /* structure to hold client's address  */
    int     sd, sd2;         /* socket descriptors                  */
    int     port;            /* protocol port number                */
    int     alen;            /* length of address                   */
    pthread_t t;
    
    fflush(stdout);srandom(time(NULL)); /* inicializa a semente do gerador de números aleatórios */
    
    memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET;         /* set family to Internet     */
    sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address   */
    
    /* Check command-line argument for protocol port and extract    */
    /* port number if one is specified.  Otherwise, use the default */
    /* port value given by constant PROTOPORT                       */
    
    if (argc > 1) {                 /* if argument specified        */
        port = atoi(argv[1]);   /* convert argument to binary   */
    } else {
        port = PROTOPORT;       /* use default port number      */
    }
    if (port > 0)                   /* test for illegal value       */
        sad.sin_port = htons((u_short)port);
    else {                          /* print error message and exit */
        fprintf(stderr,"bad port number %s\n",argv[1]);
        exit(1);
    }

        LeDitado();	
  
    /* Map TCP transport protocol name to protocol number */
    
    if ( ((ptrp = getprotobyname("tcp"))) == NULL) {
        fprintf(stderr, "cannot map \"tcp\" to protocol number");
        exit(1);
    }
    
    /* Create a socket */
    
    sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd < 0) {
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }
    
    /* Bind a local address to the socket */
    
    if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr,"bind failed\n");
        exit(1);
    }
    
    /* Specify size of request queue */
    
    if (listen(sd, QLEN) < 0) {
        fprintf(stderr,"listen failed\n");
        exit(1);
    }
    
    /* Main server loop - accept and handle requests */
    
    while (1) {
        alen = sizeof(cad);
        if ( (sd2=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
            fprintf(stderr, "accept failed\n");
            exit(1);
        }
        printf ("\nServidor atendendo conexão %d", visits);
        pthread_create(&t, NULL,  atendeConexao, &sd2 );
    }
}

