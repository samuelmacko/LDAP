/*
 * ISA
 * 
 * Samuel Macko xmacko10
 * xmacko10@stud.fit.vutbr.cz
 */

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <string>
#include <getopt.h>
#include <iostream>

using namespace std;

class Parameters{
    private:
        int port;
        string database_file_name;
    public:
        Parameters();
        int get_port();
        string get_database_file_name();
        void handle_parameters(int argc, char** argv);
};


#endif /* PARAMETERS_H */

