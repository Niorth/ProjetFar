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
//Tab clients for 10 clients
struct ClientInfo tabClients[10]; 
int nbClientsConnected = 0;

//function receives message from dsock and forward it to the others
void* listenAndForward(void *ClientToListen){
    char pseudo[20];
    
    int mySocket = ((struct ClientInfo*)ClientToListen)->socket;
    strcpy(pseudo,((struct ClientInfo*)ClientToListen)->pseudo);
    printf("Check %d %s",mySocket,pseudo);
    char msg[200];
    while(1){
        int rcv = recv(mySocket,&msg,sizeof(msg),0);
        if(rcv == 0){
            break;
        }
        
        struct messageFrom msgF;
        strcpy(msgF.msg,msg);
        strcpy(msgF.pseudo,pseudo);

        for(int i = 0; i < nbClientsConnected; i++){
            if (tabClients[i].socket != mySocket){
                send(tabClients[i].socket,&msgF,sizeof(msgF),0);
            }
        } 

        if (strcmp(msg,"end\n") == 0){
            nbClientsConnected = 0;
        }
    }
}



int main(int argc, char const *argv[]){

	if(argc != 2) {
		perror("Invalid argument number : \n1 : Port");
		return -1;
	}

    //port number
	char* port = argv[1];
    //struct uses for client connexion
    struct sockaddr_in addrClient;
    //struct uses for server connexion
    struct sockaddr_in addrServ;
    addrServ.sin_family=AF_INET;
    addrServ.sin_addr.s_addr = INADDR_ANY;
    addrServ.sin_port = htons((short) atoi(argv[1]));
    socklen_t lg = sizeof(struct sockaddr_in);

    //Client IDs
    int ID_CLIENT_1 = 1;
    int ID_CLIENT_2 = 2;
    
    //Open Connexion
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
        
        if(nbClientsConnected <10){
            int socketClient1 = accept(dSock,(struct sockaddr*) &addrClient,&lg);
            printf("socketclient %d\n", socketClient1);
            if(socketClient1 < 0) {
		        perror("Error while accepting first connection");
		        exit(0);
	        }
        
        

            char clientPseudo[20];
            recv(socketClient1,&clientPseudo,sizeof(clientPseudo),0);

            struct ClientInfo client;
            strcpy(client.pseudo,clientPseudo);
            client.socket = socketClient1;
            tabClients[nbClientsConnected] = client;
            nbClientsConnected = nbClientsConnected + 1; 

            
        
        
            pthread_t PTh_ListenClient;
        
            //creating pthread
            pthread_create(&PTh_ListenClient,NULL, listenAndForward,(void*) &client);
        }
        
    } 
    
    close(dSock);
    return 0;
}
