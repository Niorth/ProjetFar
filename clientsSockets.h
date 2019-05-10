struct ClientsSockets {
    int client1;
    int client2;
};

struct ClientInfo{
    char pseudo[20];
    int socket;
};

struct messageFrom{
    char msg[200];
    char pseudo[20];
};

struct salon{
    int nbClientsConnected;
    char desc[200]; //Description of the salon
    struct ClientInfo tabClients[10];
    int actif;

};

struct listeSalon{
    struct salon tabSalon[10];
    int nbSalons;
    int nbSalonActive;
    int nbClientsConnected;
};

struct connectToSalon{
    int idSalon;
    char desc[200];
};

struct clientSalon{
    struct ClientInfo client;
    int idSalon;
};