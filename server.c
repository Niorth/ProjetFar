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

struct ClientsSockets {
    int client1;
    int client2;
};

void* fromClient1ToClient2(void *socketList){
    char msg[200];
    while(1){
        recv(((struct ClientsSockets*) socketList)->client1,&msg,sizeof(msg),0);
        send(((struct ClientsSockets*) socketList)->client2,&msg,strlen(msg)+1,0);
        if(strcmp(msg,"end\n") == 0){
            close(((struct ClientsSockets*) socketList)->client1);
            close(((struct ClientsSockets*) socketList)->client2);
            break;
        }
    }
    
}

void* fromClient2ToClient1(void *socketList){
    char msg[200];
    while(1){
        recv(((struct ClientsSockets*) socketList)->client2,&msg,sizeof(msg),0);
        send(((struct ClientsSockets*) socketList)->client1,&msg,strlen(msg)+1,0);
        if(strcmp(msg,"end\n") == 0){
            close(((struct ClientsSockets*) socketList)->client2);
            close(((struct ClientsSockets*) socketList)->client1);
            break;
        }
    }
    
}


int main(int argc, char const *argv[]){

	if(argc != 2) {
		perror("Invalid argument number : \n1 : Port");
		return -1;
	}

	char* port = argv[1];
    
    struct sockaddr_in addrClient;

    struct sockaddr_in addrServ;
    addrServ.sin_family=AF_INET;
    addrServ.sin_addr.s_addr = INADDR_ANY;
    addrServ.sin_port = htons((short) atoi(argv[1]));
    socklen_t lg = sizeof(struct sockaddr_in);

    
    int ID_CLIENT_1 = 1;
    int ID_CLIENT_2 = 2;
    

    int dSock = socket(PF_INET,SOCK_STREAM,0);

    if (dSock < 0){
        perror("Error while creating the server socket");
        exit(0);
    }

    int rBind = bind(dSock,(struct sockaddr *) &addrServ,sizeof(addrServ));

    if (rBind < 0){
            perror("Error while binding");
            exit(0);
        }

    int rListen = listen(dSock,10);

    if(rListen < 0){
        perror("Error while listen");
        exit(0);
    }

    printf("Server launched !\n");
    
    while(1){
        int socketClient1 = accept(dSock,(struct sockaddr*) &addrClient,&lg);
        if(socketClient1 < 0) {
		    perror("Error while accepting first connection");
		    exit(0);
	    }

        send(socketClient1,&ID_CLIENT_1,sizeof(int),0);

        int socketClient2 = accept(dSock,(struct sockaddr*) &addrClient,&lg);
        if(socketClient2 < 0) {
		    perror("Error while accepting second connection");
		    exit(0);
	    }

        send(socketClient2,&ID_CLIENT_2,sizeof(int),0);

        pthread_t PTh_1To2;
        pthread_t PTh_2To1;


        struct ClientsSockets *socketList = (struct ClientsSockets*) malloc(sizeof(struct ClientsSockets));
        socketList->client1 = socketClient1;
        socketList->client2 = socketClient2;

        

        pthread_create(&PTh_1To2,NULL, fromClient1ToClient2,(void*) socketList);
        pthread_create(&PTh_2To1,NULL, fromClient2ToClient1,(void*) socketList);
        
        
    } 
    
    close(dSock);
    return 0;
}
