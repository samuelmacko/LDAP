/*
 * ISA
 * 
 * Samuel Macko xmacko10
 * xmacko10@stud.fit.vutbr.cz
 */

#include "Parameters.h"

using namespace std;

//vrati hodnotu triednej premennej @port
int Parameters::get_port(){
    return this->port;
}

//vrati hodnotu triednej premennej @database_file_name
string Parameters::get_database_file_name(){
    return this->database_file_name;
}

Parameters::Parameters(void){
    this->port = 0;
    this->database_file_name = "";
}

/*
 * naplni triedne premenne a kontroluje validitu vstupnych parametrov
 * ak nieje zadany parameter -p tak nastavi @port na hodnotu 389
 */
void Parameters::handle_parameters(int argc, char** argv){
    int parameter, port_number;
    this->port = 389;
    try{
        if(argc > 5 or argc < 1){
            throw "error: wrong parameters";
        }
        while((parameter = getopt(argc, argv, "p:f:h")) != -1){
            switch(parameter){
                case 'h':{
                    cout << "Zjednodušený LDAP server\n";
                    cout << "\n";
                    cout << "Použitie: ./myldap {-p <port>} -f <súbor>\n";
                    cout << "./myldap -h\n";
                    cout << "\t-p : nepovinný parameter, umožňuje špecifikovať port na ktorom bude server počúvať, ak nieje zadaný tak server počúva na porte 389\n";
                    cout << "\t-f : cesta k textovému súboru vo formáte csv\n";
                    cout << "\t-h : zobrazí nápovedu použitia programu\n";
                    cout << "\n";
                    cout << "Príklad použitia:\n";
                    cout << "\t./myldap -p 45678 -f test.cvt\n";
                    cout << "\t./myldap -f test.cvt\n";
                    exit(0);
                }
                case 'p':{
                    if(((port_number = strtol(optarg, NULL, 0)) != 0) and (port_number > 0) and (port_number <= 65535)){
                        this->port = port_number;
                    } else {
                        throw "error: wrong port";
                    }
                    break;
                }
                case 'f':{
                    this->database_file_name = optarg;
                    break;
                }
                default:
                    throw "error: wrong parameters";
            }
        }
        if(this->database_file_name == ""){
            throw "error: missing database file";
        }
    } catch(const char* error_message){
        cerr << error_message << endl;
        exit(1);
    }
}