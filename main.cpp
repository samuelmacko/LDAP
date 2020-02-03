/*
 * ISA
 * 
 * Samuel Macko xmacko10
 * xmacko10@stud.fit.vutbr.cz
 */

#include "main.h"

using namespace std;

/*
 * rutina ktora spracuje prichodzi signal
 * 
 * @signal : signal ktory vyvolal rutinu
 */
void signal_handler(int signal){
    cerr << "\nerror: caught signal:" << signal << endl;
    exit(1); 
}

int main(int argc, char** argv){
    struct sigaction signal_handler_st;
    //naplnenie strukturi pre spracovavanie signalov
    //z casti pomoc zo stack owerflow
    signal_handler_st.sa_handler = signal_handler;
    sigemptyset(&signal_handler_st.sa_mask);
    signal_handler_st.sa_flags = 0;
    sigaction(SIGINT, &signal_handler_st, NULL);
    //koniec
    
    Parameters params;
    //zpracovanie parametrov
    params.handle_parameters(argc, argv);
    
    int socket_client, pid;
    //vytvorenie hlavneho soketu an ktorom server prijima poziadavky
    int socket_main = Network::create_socket(params.get_port());
    socklen_t client_info_length;
    struct sockaddr_in client_info;

    //vytvorenie fronty prichodzich poziadavok
    listen(socket_main, 10);

    while(1){
        client_info_length = sizeof(client_info);
        try{
            //vytvorenie soketu pomocou ktoreho server komunikuje s klientom
            if((socket_client = accept(socket_main, (struct sockaddr *) &client_info, &client_info_length)) < 0){
                throw "error: accept failure";
            }
            //vytvorenie noveho procesu
            if((pid = fork()) < 0){
                throw "error: can't create new thread";
            }
            //proces potomka komunikuje s klientom
            if(pid == 0){
                close(socket_main);
                vector<uint8_t> bind_response {0x30, 0x0c, 0x02, 0x01, 0x01, 0x61, 0x07, 0x0a, 0x01, 0x00, 0x04, 0x00, 0x04, 0x00};
                vector<uint8_t> search_res_done_good {0x30, 0x0c, 0x02, 0x01, 0x02, 0x65, 0x07, 0x0a, 0x01, 0x00, 0x04, 0x00, 0x04, 0x00};
                
                LDAP_tools ldap_obj;
                string received_message;
                vector<string> result_vector;
                vector<uint8_t> message;
                int message_id;
                
                while(1){
                    received_message = Network::read_message(socket_client);
                    message_id = ldap_obj.get_message_id(received_message);
                    
                    if(ldap_obj.validate_bind_request(received_message)){
                        //ak server obdrzi BindRequest tak odpovie s BindResponse
                        bind_response[4] = message_id;
                        Network::send_message(socket_client, bind_response);
                    } else if(ldap_obj.validate_unbind_request(received_message)){
                        //ak server obdrzi UnbindRequest tak odpoji tohto klienta
                        close(socket_client);
                        exit(0);
                    } else if(ldap_obj.validate_search_request(received_message)){
                        //ak server obdrzi SearchRequest tak spracuje spravu a odosle odpovede vo forme sprav SearchResEntry
                        ldap_obj.load_file(params.get_database_file_name());
                        result_vector = ldap_obj.search_request_parse(received_message);
                        for(vector<string>::iterator i = result_vector.begin(); i < result_vector.end(); ++i){
                            message = ldap_obj.form_search_result(*i, message_id);
                            Network::send_message(socket_client, message);
                        }
                        //ked sa dokonci odosielanie odpovedi SearchResEntry tak sa odosle SearchResDone
                        search_res_done_good[4] = message_id;
                        Network::send_message(socket_client, search_res_done_good);
                    } else {
                        cerr << "error: wrong message" << endl;
                    }
                }
            } else {
                close(socket_client);
            }
        } catch(const char* error_message){
            cerr << error_message << endl;
            exit(1);
        }
    }
    return 0;
}