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