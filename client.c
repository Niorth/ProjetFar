#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include "clientsSockets.h"


//function send message to the server until the client writes end
void* sendToServer(void* socket){
    char msg[200];
    int sock = (int) socket;
    printf("You can write your messages \n");
    while(1){
        fgets(msg,200,stdin); 
        send(sock,&msg,strlen(msg)+1,0);
        if(strcmp(msg,"end\n") == 0){
            break;
        }
    }
    close(sock);
    printf("Disconected ! \n");
    exit(0);
}
//function receive message from the server until the client receives end
void* receiveFromServer(void * socket){
    
    struct messageFrom msgF;
    int sock = (int) socket;
    while(1){
        int rcv = recv(sock,&msgF,sizeof(msgF),0);
        if(strcmp(msgF.msg,"end\n") == 0){
            break;
        }
        *strchr(msgF.pseudo,'\n') = '\0';
        printf("%s said: %s",msgF.pseudo,msgF.msg);
    }
    close(sock);
    printf("Disconected ! \n");
    exit(0);
}



int main(int argc, char const *argv[]) {
	if(argc != 3) {
		perror("invalid argument number : \n1 : Ip\n2 : Port");
		return -1;
	}

    //IP address and port number
	char* ip;
    ip = argv[1];
	char* port; 
    port = argv[2];
    
    //struct uses for server connexion
    struct sockaddr_in addrServ;
    addrServ.sin_family=AF_INET;
    inet_pton(AF_INET, ip, &(addrServ.sin_addr));
    addrServ.sin_port = htons((short) atoi(port));
    socklen_t igA = sizeof(struct sockaddr_in);

    

    //Open Connexion
    int dSock = socket(PF_INET,SOCK_STREAM,0);
    int connexion = connect(dSock, (struct sockaddr*)&addrServ, igA);
    if (connexion < 0){
        perror("Connexion Error : ");
        exit(0);
    }

    printf("Connected ! \n");
    
    char MyPseudo[20];
    printf("Enter your pseudo: \n");
    fgets(MyPseudo,20,stdin); 

    //Send pseudo to the server
    send(dSock,&MyPseudo,strlen(MyPseudo),0);
    //pthread uses for send and receive
    pthread_t PTh_send;
    pthread_t PTh_receive;
    //creating pthread
    pthread_create(&PTh_send,NULL, sendToServer,(void*) dSock);
    pthread_create(&PTh_receive,NULL, receiveFromServer,(void*) dSock);
    //wait until the threads are finished to close the client
    pthread_join(PTh_send, NULL);
    pthread_join(PTh_receive, NULL);

    return 0;
}
