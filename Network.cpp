/*
 * ISA
 * 
 * Samuel Macko xmacko10
 * xmacko10@stud.fit.vutbr.cz
 */

#include "Network.h"

using namespace std;

/*
 * vytvori deskriptor soketu pomocou ktoreho bude server komunikovat a pripoji ho na zadany port
 * 
 * @port : port na ktori sa bindne soket
 * @socket_fd : deskriptor vytvoreneho soketu
 * @server_info : struktura obsahujuca udaje o serveri potrebne pre bindnutie soketu na port
 */
int Network::create_socket(int port){
    int socket_fd;
    struct sockaddr_in server_info;
    
    try{
        if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            throw "error: can't create socket";
        }
        memset(&server_info, 0, sizeof(server_info));
        server_info.sin_family = AF_INET;
        server_info.sin_addr.s_addr = htonl(INADDR_ANY);
        server_info.sin_port = htons(port);
        if(bind(socket_fd, (struct sockaddr *) &server_info, sizeof(server_info)) < 0){
            throw "error: can't bind to port";
        }
    } catch(const char* error_message){
        cerr << error_message << endl;
        exit(1);
    }
    
    return socket_fd;
}

/*
 * vrati retazec obsahujuci spravu na sokete
 * 
 * @socket : soket z ktoreho sa cita
 * @buffer : pole do ktoreho sa ukladaju znaky zo soketu
 * @received_characters : pocet znakov ktore sa precitali zo soketu
 * @received_message : precitana sprava
 */
string Network::read_message(int socket){
    char buffer[BUFFER_SIZE];
    ssize_t received_characters;
    string received_message = "";
    while(1){
        memset(buffer, 0, BUFFER_SIZE);
        if((received_characters = recv(socket, buffer, BUFFER_SIZE, 0)) <= 0){
            break;
        } else {
            received_message.append(buffer, received_characters);                    
            if(received_characters < BUFFER_SIZE)
                break;
        }
    }
    return received_message;
}

/*
 * zapise zadanu spravu na soket
 * 
 * @socket : deskriptor soketu an ktory sa zapisuje
 * @message : vektor obsahujuci spravu ktora sa zapise na soket
 */
void Network::send_message(int socket, vector<uint8_t> message){
    try{
        if(write(socket, &message[0], message.size()) == -1){  
            throw "error: can't send message";
        }
    } catch(const char* error_message){
        cerr << error_message << endl;
        exit(1);
    }
}