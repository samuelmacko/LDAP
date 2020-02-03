/*
 * ISA
 * 
 * Samuel Macko xmacko10
 * xmacko10@stud.fit.vutbr.cz
 */

#ifndef LDAP_TOOLS_H
#define LDAP_TOOLS_H

#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <cstdint>
#include <algorithm>

using namespace std;

class LDAP_tools{
    private:
        vector<string> cn_vec;
        vector<string> uid_vec;
        vector<string> mail_vec;
        int num_rows;
        int size_limit;
        bool negation;
        
        vector<string> get_result(string filter);
        vector<string> find_results(string type, string value);
        int length_value_length(uint8_t value);
        uint32_t length_value(string value);
        vector<uint8_t> get_length_vector(uint8_t value);
        vector<uint8_t> add_length(vector<uint8_t> arg_vector);
    public:
        LDAP_tools();
        bool validate_bind_request(string message);
        bool validate_search_request(string message);
        bool validate_unbind_request(string message);
        void load_file(string file_name);
        vector<uint8_t> form_search_result(string filter_result, uint8_t message_id);
        vector<string> search_request_parse(string message);
        uint8_t get_message_id(string message);
};

#endif /* LDAP_TOOLS_H */

