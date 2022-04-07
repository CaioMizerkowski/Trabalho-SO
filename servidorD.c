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
#include <semaphore.h>

#define PROTOPORT       5193            /* default protocol port number */
#define QLEN            20000           /* size of request queue        */
#define ARRAY_LEN 1000
#define STR_LEN 1024

int     visits=0;                       /* counts client connections    */
int     alteracoes=0;
char    msg [ARRAY_LEN][ARRAY_LEN];
int     ditados=0;
//sem_t m, m2;

void LeDitado(const char *str1)
{
    FILE *arq;
    ditados = 0;
    if ((arq=fopen(str1,"r")) == NULL) {
        printf("Erro lendo arquivo ...\n");
        exit(0);
        }
    while (!feof(arq)) {

       fgets(msg[ditados],ARRAY_LEN,arq);
       // para debug
       //printf("%d %\n",ditados,msg[ditados]);
       ditados=(ditados+1)%ARRAY_LEN;
    }
    printf("Carregou %d ditados\n",ditados);
    fclose(arq);

}

void GravaDitado(const char *str1)
{
    FILE *arq;
    int n = ditados;
    if ( (arq=fopen(str1,"w")) == NULL ) {
        printf("Erro lendo arquivo ...\n");
        exit(0);
        }

    for(int i = 0; i < ditados; i++) {
        fputs(msg[i], arq);
        n--;
        // para debug
        //printf("%d %s\n",i,msg[i]);
    }
    fclose(arq);
    printf("Salvou %d ditados\n",ditados-n);

}

char uppercase(char *input) {
    char i=0;
    for(i = 0; (i<strlen(input)) && (i<STR_LEN); i++) {
        input[i] = toupper(input[i]);
    }
    return i;
}

void remove_element(int index)
{
    int i;
    for(i = index; i < ARRAY_LEN - 1; i++){
        strcpy(msg[i], msg[i + 1]);
    }
}

void busca(int sd, char *input){
    int i;
    char str1[STR_LEN];
    printf("Buscando:%s\n", input);
    for(i = 0; i < ARRAY_LEN - 1; i++){
        //strcpy(str1, msg[i]);
        if(strstr(msg[i], input) != NULL){
            memset(str1, 0, STR_LEN);
            sprintf(str1,"Ditado %d: %s\n", i, msg[i]);
            send(sd,str1,strlen(str1),0);
        }
    }
}

int palavras_d(char *str1) {
    int c = 0, sep = 0, i;
    for(i = 0; i < STR_LEN - 1; i++){
        if (isspace(str1[i])) {
            sep = 1;
        } else {
            c += sep;
            sep = 0;
        }
    }
    return c;
}

void *atendeConexao( void *sd2 )
{
    int *temp=sd2;
    int sd=*temp;
    // fim do mutex para impedir a criação simultanea de threads
    //sem_post(&m2);

    char str1[STR_LEN], str2[STR_LEN-100], *endptr;
    int i=0, b=0, val=0, rc=0, del_values=0;
    printf("Aberto Socket: %i\n", sd);

    while (1) {
        memset(str1, 0, STR_LEN);
        //sem_wait(&m);
        visits++;
        sprintf(str1,"\nRequisição %d\n", visits);
        //sem_post(&m);

        rc = send(sd,str1,strlen(str1),MSG_NOSIGNAL);
        if (rc == -1){
            printf("Socket fechado\n");
            break;
        }
        memset(str1, 0, STR_LEN);
        recv(sd, str1, STR_LEN, 0);
        //printf("Comando recebido:%s\n",str1);

        uppercase(str1);

        if (!strncmp(str1,"GETR",4)) {
            memset(str1, 0, STR_LEN);
            //sem_wait(&m);
            memset(str1, 0, STR_LEN);
            sprintf(str1,"Ditado %d: %s\n", visits%ditados, msg[visits%ditados]);
            //sem_post(&m);
            send(sd,str1,strlen(str1),0);
        }
        else if (!strncmp(str1,"GETN",4)) {
            memset(str1, 0, STR_LEN);
            b=recv(sd, str1, STR_LEN, 0);
            str1[b]=0;
            val = strtol(str1, &endptr, 10);
            if (endptr==str1){
                memset(str1, 0, STR_LEN);
                sprintf(str1,"FALHA\n");
                send(sd,str1,strlen(str1),0);
                continue;
                }
            else{
                //sem_wait(&m);
                send(sd,msg[val],strlen(msg[val]),0);
                //sem_post(&m);
            }
        }
        else if (!strncmp(str1,"REPLACE",7)) {
            memset(str1, 0, STR_LEN);
            b=recv(sd, str1, STR_LEN, 0);
            str1[b]=0;
            val = strtol(str1, &endptr, 10);
            if (endptr==str1){
                memset(str1, 0, STR_LEN);
                sprintf(str1,"FALHA\n");
                send(sd,str1,strlen(str1),0);
                continue;
            }
            send(sd,str1,strlen(str1),0);
            b=recv(sd, str1, STR_LEN, 0);
            str1[b]=0;
            //sem_wait(&m);
            strcpy(msg[val],str1);
            alteracoes++;
            //sem_post(&m);

            memset(str1, 0, STR_LEN);
            sprintf(str1,"OK\n");
            send(sd,str1,strlen(str1),0);
            //printf("Novo ditado %d: %s\n",val,msg[val]);
            }
        else if (!strncmp(str1,"HELP",7)) {
            memset(str1, 0, STR_LEN); //Limpa o buffer
            sprintf(str1,"GETR\nGETN\nREPLACE\nVER\nDEL\nROTATE\nSEARCH\nPALAVRAS-D\nPALAVRAS-T\nALTERACOES\nGRAVA\nLE\nFIM\n");
            send(sd,str1,strlen(str1),0); //envia a mensagem
        }
        else if (!strncmp(str1,"DEL",3)) {
            memset(str1, 0, STR_LEN);
            b=recv(sd, str1, STR_LEN, 0);
            str1[b]=0;
            val = strtol(str1, &endptr, 10);
            //printf("%i", val);
            if (endptr==str1){
                memset(str1, 0, STR_LEN);
                sprintf(str1,"FALHA\n");
                send(sd,str1,strlen(str1),0);
                continue;
            }
            //sem_wait(&m);
            remove_element(val);
            alteracoes++;
            ditados--;
            del_values++;
            //sem_post(&m);
            memset(str1, 0, STR_LEN);
            sprintf(str1,"OK\n");
            send(sd,str1,strlen(str1),0);
        }
        else if (!strncmp(str1,"ROTATE",6)) {
            int val1, val2;

            memset(str1, 0, STR_LEN);
            b=recv(sd, str1, STR_LEN, 0);
            str1[b]=0;
            val1 = strtol(str1, &endptr, 10);
            if (endptr==str1){
                memset(str1, 0, STR_LEN);
                sprintf(str1,"FALHA\n");
                send(sd,str1,strlen(str1),0);
                continue;
            }

            memset(str1, 0, STR_LEN);
            b=recv(sd, str1, STR_LEN, 0);
            str1[b]=0;
            val2 = strtol(str1, &endptr, 10);
            if (endptr==str1){
                memset(str1, 0, STR_LEN);
                sprintf(str1,"FALHA\n");
                send(sd,str1,strlen(str1),0);
                continue;
            }

            //sem_wait(&m);
            strcpy(str1,msg[val1]);
            strcpy(msg[val1],msg[val2]);
            strcpy(msg[val2],str1);
            alteracoes++;
            //sem_post(&m);

            memset(str1, 0, STR_LEN);
            sprintf(str1,"OK\n");
            send(sd,str1,strlen(str1),0);
            //printf("Novo ditado %d: %s\n",val,msg[val]);
        }
        else if (!strncmp(str1,"SEARCH",6)) {
            memset(str1, 0, STR_LEN);
            recv(sd, str1, STR_LEN, 0); //str1 com o termo de busca
            //sem_wait(&m);
            busca(sd, str1);
            //sem_post(&m);
        }
        else if (!strncmp(str1,"PALAVRAS-D",10)) {
            int c;
            memset(str1, 0, STR_LEN);
            b=recv(sd, str1, STR_LEN, 0);
            str1[b]=0;
            val = strtol(str1, &endptr, 10);
            if (endptr==str1){
                memset(str1, 0, STR_LEN);
                sprintf(str1,"FALHA\n");
                send(sd,str1,strlen(str1),0);
                continue;
            }
            //sem_wait(&m);
            strcpy(str1, msg[val]);
            c = palavras_d(str1);
            //sem_post(&m);

            memset(str1, 0, STR_LEN);
            sprintf(str1, "%d\n", c);
            send(sd,str1,strlen(str1),0);
        }
        else if (!strncmp(str1,"PALAVRAS-T",10)) {
            int c=0, i;
            //sem_wait(&m);
            for(i = 0; i < ARRAY_LEN - 1; i++){
                memset(str1, 0, STR_LEN);
                strcpy(str1, msg[i]);
                c += palavras_d(str1);
            }
            //sem_post(&m);

            memset(str1, 0, STR_LEN);
            sprintf(str1, "%d\n", c);
            send(sd,str1,strlen(str1),0);
        }
        else if (!strncmp(str1,"ALTERACOES",10)) {
            memset(str1, 0, STR_LEN); //Limpa o buffer
            //sem_wait(&m);

            memset(str1, 0, STR_LEN);
            sprintf(str1,"%d\n", alteracoes);
            //sem_post(&m);
            send(sd,str1,strlen(str1),0); //envia a mensagem
        }
        else if (!strncmp(str1,"GRAVA",5)) {
            memset(str1, 0, STR_LEN); //Limpa o buffer
            memset(str2, 0, STR_LEN-100); //Limpa o buffer
            recv(sd, str2, STR_LEN-100, 0);
            //sem_wait(&m);
            GravaDitado(str2);
            //sem_post(&m);

            memset(str1, 0, STR_LEN);
            sprintf(str1,"Mensagem gravada em %s\n", str2);
            send(sd,str1,strlen(str1),0); //envia a mensagem
        }
        else if (!strncmp(str1,"LE",2)) {
            memset(str1, 0, STR_LEN); //Limpa o buffer
            memset(str2, 0, STR_LEN-100); //Limpa o buffer
            recv(sd, str2, STR_LEN-100, 0);
            //sem_wait(&m);
            LeDitado(str2);
            alteracoes = 0;
            //sem_post(&m);

            memset(str1, 0, STR_LEN);
            sprintf(str1,"Mensagem lida de %s\n", str2);
            send(sd,str1,STR_LEN,0); //envia a mensagem
        }
        else if (!strncmp(str1,"FIM",3)) {
            memset(str1, 0, STR_LEN);
            sprintf(str1,"Adeus\n");
            send(sd,str1,strlen(str1),0);
            break;
            }
        else if (!strncmp(str1,"VER",3)) {
            memset(str1, 0, STR_LEN);
            sprintf(str1,"Servidor de Ditados 2.0 Beta.\nTE355 2022 Primeiro Trabalho\n");
            send(sd,str1,strlen(str1),0);
           }
        else if (!strncmp(str1,"\n",1)) {
            memset(str1, 0, STR_LEN);
        }
        else{
            rc = send(sd,str1,strlen(str1),MSG_NOSIGNAL);
            if(rc == -1){
                printf("Não enviado:%s\n", str1);
                continue;
            }
            memset(str2, 0, STR_LEN-100);
            strcpy(str2, str1);
            memset(str1, 0, STR_LEN);
            sprintf(str1,"Erro de Protocolo, recebido '%s'\n", str2);
            printf("Erro de Protocolo, recebido '%s'\n", str2);
            send(sd,str1,strlen(str1),0);
           }

        memset(str1, 0, STR_LEN);
        sprintf(str1,"ACK");
        rc = send(sd,str1,strlen(str1),MSG_NOSIGNAL);
        if (rc == -1){
            printf("Socket fechado\n");
            break;
        }

    }

    printf("Fechando a conexão com %i deletes\n", del_values);
    close(sd);
    printf("Fechado Socket: %i\n", sd);
    fflush(stdout);
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

    //sem_init(&m, 0, 1);
    //sem_init(&m2, 0, 1);

    fflush(stdout);
    srandom(time(NULL)); /* inicializa a semente do gerador de números aleatórios */

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
    if (port > 0){                  /* test for illegal value       */
        sad.sin_port = htons((u_short)port);
    } else {                          /* print error message and exit */
        fprintf(stderr,"bad port number %s\n",argv[1]);
        exit(1);
    }

    LeDitado("Ditados.txt");

    /* Map TCP transport protocol name to protocol number */

    if ( ((ptrp = getprotobyname("tcp"))) == NULL) {
        fprintf(stderr, "cannot map \"tcp\" to protocol number\n");
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
        //sem_wait(&m2);
        alen = sizeof(cad);
        // inicio do mutex para impedir a criação simultanea de threads
        sd2=accept(sd, (struct sockaddr *)&cad, &alen);
        printf("Socket: %i\n", sd2);
        if ( sd2 < 0) {
            fprintf(stderr, "accept failed\n");
            exit(1);
        }
        //esta região é critica, até o depois do create
        printf("Servidor atendendo conexão %d\n", visits);
        pthread_create(&t, NULL,  atendeConexao, &sd2 );
    }
}

