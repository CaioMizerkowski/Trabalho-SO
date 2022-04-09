/* client.c - code for example client program that uses TCP */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <semaphore.h>
#include <stdbool.h>
#define PROTOPORT       5193            /* default protocol port number */
#define N_THREADS       2
#define QLEN            2000               /* size of request queue        */
#define ARRAY_LEN 1000
#define STR_LEN 1024

int     visits=0;                       /* counts client connections    */
int     alteracoes=0;
char    msg [ARRAY_LEN][ARRAY_LEN];
int     ditados=0;
sem_t m;
bool lock = true;
extern  int             errno;
char    localhost[] =   "localhost";    /* default host name            */
/*------------------------------------------------------------------------
 * Program:   client
 *
 * Purpose:   allocate a socket, connect to a server, and print all output
 *
 * Syntax:    client [ host [port] ]
 *
 *               host  - name of a computer on which server is executing
 *               port  - protocol port number server is using
 *
 * Note:      Both arguments are optional.  If no host name is specified,
 *            the client uses "localhost"; if no protocol port is
 *            specified, the client uses the default given by PROTOPORT.
 *
 *------------------------------------------------------------------------
 */

void *recebeDados( void *sd2 ){
    int *temp=sd2;
    int sd=*temp;
    int n=1;
    char buf[1024];

    while (n > 0) {
        memset(buf, 0, sizeof(buf));
        n = recv(sd, buf, sizeof(buf), 0);

        if(lock && ){
            sem_post(&m);
        }else{
            printf("%s",buf);
        }

        if (!strncmp(buf,"Adeus",5)) {
            printf("Parando de receber dados\n");
            sem_post(&m);
            break;
        }
    }
}

void *enviaDados( void *sd2 ){
    int *temp=sd2;
    int sd=*temp;
    int n;
    char buf[1024];

    while (1){
        memset(buf, 0, sizeof(buf));
        scanf("%s", buf);
        send(sd, buf, sizeof(buf), 0);

       if(!strncmp(buf,"FIM",3)){
            break;
        }
    }
}

void *delEnviaDados( void *sd2 ){
    int *temp=sd2;
    int sd=*temp;
    int n;
    char buf[1024];

    int contador=0;
    while (contador<100){
        sem_wait(&m);
        memset(buf, 0, sizeof(buf));
        strcpy(buf, "DEL\n");
        send(sd, buf, sizeof(buf), 0);

        memset(buf, 0, sizeof(buf));
        strcpy(buf, "1\n");
        send(sd, buf, sizeof(buf), 0);

        contador++;
    }

    sleep(1);
    memset(buf, 0, sizeof(buf));
    strcpy(buf ,"FIM");
    send(sd, buf, sizeof(buf), 0);
}

void *rotateEnviaDados( void *sd2 ){
    int *temp=sd2;
    int sd=*temp;
    int n;
    char buf[1024];

    int contador=0;
    while (contador<100){
        sem_wait(&m);
        memset(buf, 0, sizeof(buf));
        strcpy(buf, "ROTATE\n");
        send(sd, buf, sizeof(buf), 0);

        memset(buf, 0, sizeof(buf));
        strcpy(buf, "1\n");
        send(sd, buf, sizeof(buf), 0);

        memset(buf, 0, sizeof(buf));
        strcpy(buf, "10\n");
        send(sd, buf, sizeof(buf), 0);

        contador++;
    }
    sleep(1);
    memset(buf, 0, sizeof(buf));
    strcpy(buf ,"FIM");
    send(sd, buf, sizeof(buf), 0);
}

void *replaceEnviaDados( void *sd2 ){
    int *temp=sd2;
    int sd=*temp;
    int n;
    char buf[1024];

    int contador=0;
    while (contador<100){
        sem_wait(&m);
        memset(buf, 0, sizeof(buf));
        strcpy(buf, "REPLACE\n");
        send(sd, buf, sizeof(buf), 0);

        memset(buf, 0, sizeof(buf));
        strcpy(buf, "10\n");
        send(sd, buf, sizeof(buf), 0);

        memset(buf, 0, sizeof(buf));
        strcpy(buf, "stringAleatóriaAqui\n");
        send(sd, buf, sizeof(buf), 0);

        contador++;
    }
    sleep(1);
    memset(buf, 0, sizeof(buf));
    strcpy(buf ,"FIM");
    send(sd, buf, sizeof(buf), 0);
}

int main(int argc, char **argv)
{
    struct  hostent  *ptrh;  /* pointer to a host table entry       */
    struct  protoent *ptrp;  /* pointer to a protocol table entry   */
    struct  sockaddr_in sad; /* structure to hold an IP address     */
    int     sd;              /* socket descriptor                   */
    int     port;            /* protocol port number                */
    char    *host;           /* pointer to host name                */
    int     n;               /* number of characters read           */
    char    buf[1024];       /* buffer for data from the server     */
    pthread_t t[N_THREADS];
    int tipo_client = 0;

#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(0x0101, &wsaData);
#endif
    printf("#OLA\n");

    memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET;         /* set family to Internet     */
    /* Check command-line argument for protocol port and extract    */
    /* port number if one is specified.  Otherwise, use the default */
    /* port value given by constant PROTOPORT                       */
    if (argc > 2) {                 /* if protocol port specified   */
        port = atoi(argv[2]);   /* convert to binary            */
    } else {
        port = PROTOPORT;       /* use default port number      */
    }
    if (port > 0)                   /* test for legal value         */
        sad.sin_port = htons((u_short)port);
    else {                          /* print error message and exit */
        fprintf(stderr,"bad port number %s\n",argv[2]);
        exit(1);
    }
    /* Check host argument and assign host name. */
    if (argc > 1) {
        host = argv[1];         /* if host argument specified   */
    } else {
        host = localhost;
    }

    /* Convert host name to equivalent IP address and copy to sad. */
    ptrh = gethostbyname(host);
    if ( ((char *)ptrh) == NULL ) {
        fprintf(stderr,"invalid host: %s\n", host);
        exit(1);
    }
    memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);
    /* Map TCP transport protocol name to protocol number. */
    if ( ((long long)(ptrp = getprotobyname("tcp"))) == 0) {
        fprintf(stderr, "cannot map \"tcp\" to protocol number\n");
        exit(1);
    }
    /* Create a socket. */
    sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd < 0) {
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }
    /* Connect the socket to the specified server. */
    if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr,"connect failed\n");
        exit(1);
    }

    sem_init(&m, 0, 1);
    printf("#ESTOU PRONTO\n");

    printf("Iniciando primeira thread\n");
    pthread_create(&t[0], NULL, recebeDados, &sd );

    /* Check função do cliente */
    if (argc > 3) {
        tipo_client = atoi(argv[3]);
    }

    printf("Iniciando segunda thread\n");
    if(tipo_client==0){
        printf("Tipo padrão\n");
        lock = false;
        pthread_create(&t[1], NULL,  enviaDados, &sd );
    }else if(tipo_client==1){
        printf("Tipo delete\n");
        pthread_create(&t[1], NULL,  delEnviaDados, &sd );
    }else if (tipo_client==2){
        printf("Tipo rotate\n");
        pthread_create(&t[1], NULL,  rotateEnviaDados, &sd );
    }else if (tipo_client==3){
        printf("Tipo replace\n");
        pthread_create(&t[1], NULL,  replaceEnviaDados, &sd );
    }else{
        printf("Tipo padrão\n");
        lock = false;
        pthread_create(&t[1], NULL,  enviaDados, &sd );
    }

    pthread_join(t[1], NULL);
    printf("#Enviou tudo\n");
    pthread_join(t[0], NULL);
    printf("#Recebeu tudo\n");

    fflush(NULL);
    /* Close the socket. */
    closesocket(sd);

    printf("#Terminando programa\n");
    /* Terminate the client program gracefully. */
    exit(0);
}
