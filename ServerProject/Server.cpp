#include "ServerNetwork.h"

int main() {
    ServerNetwork server;
    if (server.Start(8080)) {
        server.AcceptClients();
    }
    return 0;
}
