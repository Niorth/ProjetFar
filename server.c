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

//Salons with max 10 salon
//nb client max by salon is handle in the salon struct
struct listeSalon Salons;

//function receives message from dsock and forward it to the others
void *listenAndForward(void *ClientToListen)
{
    char pseudo[20];
    int mySalon = ((struct clientSalon *)ClientToListen)->idSalon;
    int mySocket = ((struct clientSalon *)ClientToListen)->client.socket;
    strcpy(pseudo, ((struct clientSalon *)ClientToListen)->client.pseudo);
    printf("Check %d %s", mySocket, pseudo);
    char msg[200];
    while (1)
    {
        int rcv = recv(mySocket, &msg, sizeof(msg), 0);
        if (rcv == 0)
        {
            break;
        }

        struct messageFrom msgF;
        strcpy(msgF.msg, msg);
        strcpy(msgF.pseudo, pseudo);

        for (int i = 0; i < Salons.tabSalon[mySalon].nbClientsConnected; i++)
        {
            if (Salons.tabSalon[mySalon].tabClients[i].socket != mySocket)
            {
                send(Salons.tabSalon[mySalon].tabClients[i].socket, &msgF, sizeof(msgF), 0);
            }
        }

        if (strcmp(msg, "end\n") == 0)
        {
            Salons.nbClientsConnected = Salons.nbClientsConnected - Salons.tabSalon[mySalon].nbClientsConnected; //we disconect all users of the salon when one ask to disconect
            Salons.tabSalon[mySalon].nbClientsConnected = 0;
            Salons.tabSalon[mySalon].actif = 0;
            Salons.nbSalonActive = Salons.nbSalonActive - 1;
        }
    }
}

int main(int argc, char const *argv[])
{

    Salons.nbSalons = 0;
    Salons.nbClientsConnected = 0;
    Salons.nbSalonActive = 0;

    struct salon saloninit;
    strcpy(saloninit.desc, "");
    saloninit.actif = 0;
    saloninit.nbClientsConnected = 0;

    struct ClientInfo clientinit;
    strcpy(clientinit.pseudo, "");
    clientinit.socket = 0;

    for (int j = 0; j < 10; j++)
    {
        saloninit.tabClients[j] = clientinit;
    }

    for (int i = 0; i < 10; i++)
    {
        Salons.tabSalon[i] = saloninit;
    }

    if (argc != 2)
    {
        perror("Invalid argument number : \n1 : Port");
        return -1;
    }

    //port number
    char *port = argv[1];
    //struct uses for client connexion
    struct sockaddr_in addrClient;
    //struct uses for server connexion
    struct sockaddr_in addrServ;
    addrServ.sin_family = AF_INET;
    addrServ.sin_addr.s_addr = INADDR_ANY;
    addrServ.sin_port = htons((short)atoi(argv[1]));
    socklen_t lg = sizeof(struct sockaddr_in);

    //Client IDs
    int ID_CLIENT_1 = 1;
    int ID_CLIENT_2 = 2;

    //Open Connexion
    int dSock = socket(PF_INET, SOCK_STREAM, 0);

    if (dSock < 0)
    {
        perror("Error while creating the server socket");
        exit(0);
    }

    int rBind = bind(dSock, (struct sockaddr *)&addrServ, sizeof(addrServ));

    if (rBind < 0)
    {
        perror("Error while binding");
        exit(0);
    }

    int rListen = listen(dSock, 10);

    if (rListen < 0)
    {
        perror("Error while listen");
        exit(0);
    }

    printf("Server launched !\n");

    while (1)
    {

        if (Salons.nbClientsConnected < 100)
        { //100 because 10 salons of 10 clients
            int socketClient1 = accept(dSock, (struct sockaddr *)&addrClient, &lg);
            printf("socketclient %d\n", socketClient1);
            if (socketClient1 < 0)
            {
                perror("Error while accepting first connection");
                exit(0);
            }

            //Receive client's pseudo
            char clientPseudo[20];
            recv(socketClient1, &clientPseudo, sizeof(clientPseudo), 0);

            //Store it in a client struct
            struct ClientInfo client;
            strcpy(client.pseudo, clientPseudo);
            client.socket = socketClient1;

            printf("pseudo: %s \n", clientPseudo);

            //Send list of salon availabe to the client
            int sendList = send(socketClient1, &Salons, sizeof(Salons), 0);
            if (sendList < 0)
            {
                perror("error: ");
            }
            else
            {
                printf("ok send \n");
            }
            //receive salon id from client to connect him to his salon
            struct connectToSalon salonToConnect;
            recv(socketClient1, &salonToConnect, sizeof(salonToConnect), 0);

            //connect client to his salon
            Salons.nbClientsConnected++;
            printf("nb client connected %d\n", Salons.nbClientsConnected);
            Salons.tabSalon[salonToConnect.idSalon].tabClients[Salons.tabSalon[salonToConnect.idSalon].nbClientsConnected] = client;
            Salons.tabSalon[salonToConnect.idSalon].nbClientsConnected += 1;
            printf("nb client connected salon %d\n", Salons.tabSalon[salonToConnect.idSalon].nbClientsConnected);

            printf("desc %s \n", salonToConnect.desc);
            if (strcmp(salonToConnect.desc, "") != 0)
            { //if we have a desc that mean we create a new salon
                if (Salons.nbSalons < 10)
                {
                    Salons.nbSalons++; //if we don't have 10 salons that mean we created a new salon
                    strcpy(Salons.tabSalon[salonToConnect.idSalon].desc, salonToConnect.desc);
                }
                Salons.nbSalonActive++;
                strcpy(Salons.tabSalon[salonToConnect.idSalon].desc, salonToConnect.desc);
                Salons.tabSalon[salonToConnect.idSalon].actif = 1;
            }

            printf(" desc salon 0 %s \n", Salons.tabSalon[0].desc);

            printf("Les clients du salon \n");
            for (int i = 0; i < Salons.tabSalon[0].nbClientsConnected; i++){
                printf("utilisateur : %s \n",Salons.tabSalon[0].tabClients[i].pseudo);
            }

            //create a struct to store client and his salon;
            struct clientSalon CS;
            CS.client = client;
            CS.idSalon = salonToConnect.idSalon;
            pthread_t PTh_ListenClient;

            //creating pthread
            pthread_create(&PTh_ListenClient, NULL, listenAndForward, (void *)&CS);
        }
    }

    close(dSock);
    return 0;
}
