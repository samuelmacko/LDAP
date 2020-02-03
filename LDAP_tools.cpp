/*
 * ISA
 * 
 * Samuel Macko xmacko10
 * xmacko10@stud.fit.vutbr.cz
 */

#include "LDAP_tools.h"

using namespace std;

LDAP_tools::LDAP_tools(void){
    this->size_limit = 0;
    this->num_rows = 0;
    this->negation = false;
}

//vrati ID spravy @message_id zadanej spravy @message
uint8_t LDAP_tools::get_message_id(string message){
    int shift = this->length_value_length(message[1]);
    uint8_t message_id = message[4 + shift];
    return message_id;
}

/*
 * vrati true ak je zadana sprava validny SearchRequest
 * 
 * @message : sprava ktoru kontrolujeme
 * @main_shift : pocet znakov o ktory je posunuta sprava do prava koli znaku vyjadrujucemu dlzku celej spravy, 
 * posun vznika koli tomu ze dlzka bloku spravy moze byt zapisana na viacerich znakoch
 * @sr_shift : pocet znakov o ktory je posunuta sprava do prava koli znaku vyjadrujucemu dlzku spravy SearchRequest
 */
bool LDAP_tools::validate_search_request(string message){
    int main_shift = this->length_value_length(message[1]);
    int sr_shift = this->length_value_length(message[6 + main_shift]);
    sr_shift += main_shift;
    int base_object_length = message[8 + sr_shift];
    if(message[0] == 0x30 and
        message[2 + main_shift] == 0x02 and
        message[3 + main_shift] == 0x01 and
        message[5 + main_shift] == 0x63 and
        message[7 + sr_shift] == 0x04 and
        message[9 + base_object_length + sr_shift] == 0x0a and
        message[10 + base_object_length + sr_shift] == 0x01 and
        message[15 + base_object_length + sr_shift] == 0x02 and
        message[16 + base_object_length + sr_shift] == 0x01
            ){
        return true;
    } else {
        return false;
    }
}

//vrati true ak je zadana sprava @message validny BindRequest
bool LDAP_tools::validate_bind_request(string message){
    int name_length = message[11];
    uint8_t authetication_mode = message[12 + name_length];
    if(message[0] == 0x30 and
        message[2] == 0x02 and
        message[3] == 0x01 and
        message[5] == 0x60 and
        message[7] == 0x02 and
        message[8] == 0x01 and
        message[10] == 0x04 and
        authetication_mode == 0x80
            ){
        return true;
    } else {
        return false;
    }
}

//vrati true ak je zadana sprava @message validny UnbindRequest
bool LDAP_tools::validate_unbind_request(string message){
    if(message[0] == 0x30 and
        message[2] == 0x02 and
        message[3] == 0x01 and
        message[5] == 0x42 and
        message[6] == 0x00
            ){
        return true;
    } else {
        return false;
    }
}
/*
 * nacita obsah suboru do vektorov @cv_vec, @cn_uid a @cn_mail
 * 
 * @file_name : nazov suboru z ktoreho nacitame data
 * @line : jeden riadok zdrojoveho suboru
 * @correct_line_format : regex na kontrolu ci je riadok ciarkami rozdeleny na 3 casti
 * @file : deskriptor suboru
 * @comma_position : poloha ciarky "," v riadku suboru
 */
void LDAP_tools::load_file(string file_name){
    string line;
    regex correct_line_format(".+,.+,.+");
    ifstream file;
    size_t comma_position;
    file.open(file_name.c_str(), ios::out);
    try{
        if(file.is_open()){
            while(getline(file, line)){
                if(regex_match(line, correct_line_format)){
                    this->num_rows++;
                    
                    comma_position = line.find(",");
                    this->cn_vec.push_back(line.substr(0, comma_position));
                    line.erase(0, comma_position + 1);

                    comma_position = line.find(",");
                    this->uid_vec.push_back(line.substr(0, comma_position));
                    line.erase(0, comma_position + 1);

                    this->mail_vec.push_back(line.substr(0));
                }
            }
            file.close();
        } else {
            throw "error: can't open file";
        }
    } catch(const char* error_message){
        cerr << error_message << endl;
    }
}

/*
 * vrati vektor obsahujuci vsetky riadky databazy ktore sa odoslu klientovi
 * 
 * @message : sprava SearchRequest
 * @shift : pocet znakov o ktory sa posuva zvysok spravy do prava, posun vznika koli tomu ze
 * dlzka bloku spravy moze byt zapisana na viacerich znakoch
 * @base_object_length : dlzka mena bazoveho objektu, uklada sa koli posunu znakov do prava
 * @result_vector : vektor obsahujuci vyhovujuce riadky databazy
 */
vector<string> LDAP_tools::search_request_parse(string message){
    int shift = this->length_value_length(message[1]);
    shift += this->length_value_length(message[6 + shift]);
    int base_object_length = message[8 + shift];
    vector<string> result_vector;
    if(message[17 + base_object_length + shift] != 0x00){
        //ak je v sprave zadana nenulova hodnota SizeLimit tak ju ulozime
        this->size_limit = message[17 + base_object_length + shift];
    }
    
    result_vector = this->get_result(message.substr(24 + base_object_length + shift * 2));
    
    if(this->size_limit != 0){
        //ak je zadana hodnota SizeLimit tak z vysledneho vektoru @result_vector vymazeme vsetky polia
        //ktore presiahuju tuto hodnotu
        result_vector.erase(result_vector.begin() + this->size_limit, result_vector.end());
    }
    return result_vector;
}

/*
 * vrati pocet extra znakov na ktorych je zapisana dlzka bloku spravy
 * 
 * @value : hodnota 1. znaku udavajuceho dlzku
 * @length : pocet extra znakov
 */
int LDAP_tools::length_value_length(uint8_t value){
    int length = 0;
    if(value >= 0x81){
        length = value - 0x80;
    }
    return length;
}

/*
 * vrati hodnotu ulozenu na znakoch ktore reprezentuju dlzku bloku spravy
 * 
 * @value : znaky reprezentujuce dlzku
 * @length : hodnota dlzky ktore tieto znaky reprezentuju
 */
uint32_t LDAP_tools::length_value(string value){
    uint32_t length = 0;
    if(value.length() == 1){
        length = value[0];
    } else {
        for(unsigned int i = 1; i < value.length(); i++){
            length += value[i];
            if(i != (value.length() - 1)){
                length = length << 8;
            }
        }
    }
    return length;
}

/*
 * vrati vektor obsahujuci vsetky polozky suboru ktore vyhovuju zadanemu filtru @filter
 * 
 * formuje regularny vyraz pomocou ktoreho sa prehladavaju zaznamy suboru, filter moze byt zlozeny z
 * viacerich podfiltrov takze sa metoda moze volat rekurzivne
 * 
 * @filter : klientom zadany filter podla ktoreho vyberame vyhovujuce zaznami suboru
 * @shift : posun znakov spravy do prava koli znakom obsahujucim dlzku
 * @mode : hodnota ktora udava ci sa jedna o filter And, Or, EqualityMatch, Substring alebo Not
 * @result_vector : vektor obsahujuci zaznamy suboru ktore vyhovuju danemu filtru
 */
vector<string> LDAP_tools::get_result(string filter){
    int shift = this->length_value_length(filter[1]);
    unsigned char mode = filter[0];
    vector<string> result_vector;
    try{
        switch(mode){
            case 0xa3:{
                /*
                 * EqualityMatch
                 * 
                 * @type : typ (cn/commonname/uid/userid/mail)
                 * @type_length : dlzka retazca @type
                 * @temp_shift : pomocna premenna na ukladanie ciastocnej hodnoty celkoveho posunu @shift
                 * @value : hodnota typu
                 * @value_length : dlzka retazkca @value
                 */
                int type_length = filter[3 + shift];
                string type = filter.substr(4 + shift, type_length);
                int temp_shift = this->length_value_length(filter[5 + type_length + shift]);
                unsigned char value_length = this->length_value(filter.substr(5 + type_length + shift, temp_shift + 1));
                shift += temp_shift;
                string value = filter.substr(6 + type_length + shift, value_length);
                result_vector = this->find_results(type, value);
                return result_vector;
                break;
            }
            case 0xa4:{
                /*
                 * Substring
                 * 
                 * @temp_shift : pomocna premenna na ukladanie ciastocnej hodnoty celkoveho posunu @shift
                 * @substring_shift : posun spravy iba vramci Substring casti filtru
                 * @type : typ (cn/commonname/uid/userid/mail)
                 * @type_length : dlzka retazca @type
                 * @substring_type : typ substringu (initial/any/final)
                 * @substring_length : dlazka substringu
                 * @total_value : uplny regularny vyraz popisujuci filter
                 * @partial_value : cast regularneho vyrazu
                 * @partial_value_length : dlzka retazca @partial_value
                 * @substring_pos : pozicia vramci celeho filtru, pouziva sa pri oddelovani 
                 * ciastkovych regularnych vyrazov
                 */
                int substring_shift;
                int temp_shift;
                int substring_pos = 0;
                int type_length = filter[3 + shift];
                string type = filter.substr(4 + shift, type_length);
                temp_shift = this->length_value_length(filter[5 + type_length + shift]);
                uint8_t substring_length = this->length_value(filter.substr(5 + type_length + shift, temp_shift + 1));
                shift += temp_shift;
                uint8_t substring_type;
                uint8_t partial_value_length;
                string partial_value = "";
                string total_value = "";
                while(substring_pos < substring_length){
                    //hlavny regularny vyraz sa moze skladat z viacerych ciastkovych, preto sa v cykle
                    //cely prechadza az kym sa ukazatel pozicie @substring_pos nerovna dlzke substringu
                    substring_type = filter[6 + type_length + substring_pos + shift];
                    substring_shift = this->length_value_length(filter[7 + type_length + substring_pos + shift]);
                    partial_value_length = this->length_value(filter.substr(7 + type_length + substring_pos + shift, substring_shift + 1));
                    partial_value = filter.substr(8 + type_length + substring_pos + shift + substring_shift, partial_value_length);
                    switch(substring_type){
                        case 0x80:{
                            //initial
                            partial_value += ".*";
                            break;
                        }
                        case 0x81:{
                            //any
                            partial_value = ".*" + partial_value + ".*";
                            break;
                        }
                        case 0x82:{
                            //final
                            partial_value = ".*" + partial_value;
                            break;
                        }
                        default:
                            throw "error: wrong substring type";
                            break;
                    }
                    substring_pos += 2 + partial_value_length + substring_shift;
                    total_value += partial_value;
                }
                result_vector = this->find_results(type, total_value);
                return result_vector;
                break;
            }
            case 0xa0:{
                /*
                 * And
                 * 
                 * @length : dlzka celeho filtru
                 * @operand_a : vektor obsahujuci zaznami vyhovujuce subfiltru 1. operandu
                 * @operand_b : vektor obsahujuci zaznami vyhovujuce subfiltru 2. operandu
                 * @operand_a_length : dlzka 1. operandu
                 * @operand_b_length : dlzka 2. operandu
                 * @operand_a_shift : posun vramci 1. operandu
                 * @match : iterator nad vektorom, sluzi na prechadzanie vektoru @operand_a
                 */
                uint8_t length = this->length_value(filter.substr(1, shift + 1));
                vector<string> operand_a;
                vector<string> operand_b;
                int operand_a_shift = this->length_value_length(filter[3 + shift]);
                uint8_t operand_a_length = this->length_value(filter.substr(3 + shift, operand_a_shift + 1));
                uint8_t operand_b_length = length - (operand_a_length + 2);
                operand_a = this->get_result(filter.substr(2 + shift, operand_a_length + 2));
                operand_b = this->get_result(filter.substr(4 + shift + operand_a_length, operand_b_length));
                vector<string>::iterator match;
                for(vector<string>::iterator i = operand_a.begin(); i < operand_a.end(); ++i){
                    //do vektoru obsahujuceho vyhovujuce zaznami @result_vector vklada iba tie zaznami
                    //ktore su sucastne v @operand_a a v @operand_b
                    match = find(operand_b.begin(), operand_b.end(), *i);
                    if(match != operand_b.end()){
                        result_vector.push_back(*match);
                    }
                }
                //zoradenie vysledkov a zmazanie duplikatov
                sort(result_vector.begin(), result_vector.end());
                result_vector.erase(unique(result_vector.begin(), result_vector.end()), result_vector.end());
                return result_vector;
                break;
            }
            case 0xa1:{
                /*
                 * Or
                 * 
                 * @length : dlzka celeho filtru
                 * @operand_a : vektor obsahujuci zaznami vyhovujuce subfiltru 1. operandu
                 * @operand_b : vektor obsahujuci zaznami vyhovujuce subfiltru 2. operandu
                 * @operand_a_length : dlzka 1. operandu
                 * @operand_b_length : dlzka 2. operandu
                 * @operand_a_shift : posun vramci 1. operandu
                 */
                uint8_t length = this->length_value(filter.substr(1, shift + 1));
                vector<string> operand_a;
                vector<string> operand_b;
                int operand_a_shift = this->length_value_length(filter[3 + shift]);
                uint8_t operand_a_length = this->length_value(filter.substr(3 + shift, operand_a_shift + 1));
                uint8_t operand_b_length = length - (operand_a_length + 2);
                operand_a = this->get_result(filter.substr(2 + shift, operand_a_length + 2));
                operand_b = this->get_result(filter.substr(4 + shift + operand_a_length, operand_b_length));
                
                //@result_vector obsahuje oba vektory @operand_a a @operand_b
                result_vector.insert(result_vector.end(), operand_a.begin(), operand_a.end());
                result_vector.insert(result_vector.end(), operand_b.begin(), operand_b.end());
                
                //zoradenie vysledkov a zmazanie duplikatov
                sort(result_vector.begin(), result_vector.end());
                result_vector.erase(unique(result_vector.begin(), result_vector.end()), result_vector.end());
                return result_vector;
                break;
            }
            case 0xa2:{
                /*
                 * Not
                 */
                this->negation = true;
                result_vector = LDAP_tools::get_result(filter.substr(2 + shift));
                this->negation = false;
                return result_vector;
                break;
            }
            case 0x87:{
                /*
                 * NoFilter
                 * 
                 * ak nieje zadany ziaden filter tak sa vyhladava podla regularneho vyrazu
                 * ktoremu vyhovuju vsetky znaky
                 */
                result_vector = this->find_results("cn", ".*");
                return result_vector;
                break;
            }
            default:
                throw "error: wrong mode";
        }
    } catch(const char* error_message){
        cerr << error_message << endl;
        vector<string> result_vector;
        return result_vector;
    }
}

/*
 * vrati vektor vsetkych zaznamov vyhovujucich zadanemu regularnemu vyrazu pre dany typ
 * 
 * @type : oznacuje v ktorom z vektorov hladame:
 *      cn, commonname : @cn_vec
 *      uid, userid : @uid_vec
 *      mail : @mail_vec
 * @value : regularny vyraz
 * @matched_line : retazec obsahujuci cn, mail a uid v tvare: cn;mail;uid
 * @result_vector : vektor obsahujuci vysledky
 */
vector<string> LDAP_tools::find_results(string type, string value){
    vector<string> result_vector;
    string matched_line = "";
    
    regex rgx(value, regex_constants::ECMAScript | regex_constants::icase);
    transform(type.begin(), type.end(), type.begin(), ::tolower);
    if((type == "cn") or (type == "commonname")){
        for(int i = 0; i < this->num_rows; i++){
            if((regex_match(this->cn_vec[i], rgx) and !this->negation) or
                (!regex_match(this->cn_vec[i], rgx) and this->negation)){
                matched_line += this->cn_vec[i] + ";" + this->mail_vec[i] + ";" + this->uid_vec[i];
                result_vector.push_back(matched_line);
                matched_line = "";
            }
        }
    } else if((type == "uid") or (type == "userid")){
        for(int i = 0; i < this->num_rows; i++){
            if((regex_match(this->uid_vec[i], rgx) and !this->negation) or
                (!regex_match(this->uid_vec[i], rgx) and this->negation)){
                matched_line += this->cn_vec[i] + ";" + this->mail_vec[i] + ";" + this->uid_vec[i];
                result_vector.push_back(matched_line);
                matched_line = "";
            }
        }
    } else if(type == "mail"){
        for(int i = 0; i < this->num_rows; i++){
            if((regex_match(this->mail_vec[i], rgx) and !this->negation) or
                (!regex_match(this->mail_vec[i], rgx) and this->negation)){
                matched_line += this->cn_vec[i] + ";" + this->mail_vec[i] + ";" + this->uid_vec[i];
                result_vector.push_back(matched_line);
                matched_line = "";
            }
        }
    } else {
        cerr << "error: wrong type" << endl;
    }
    return result_vector;
}

/*
 * pred zadany vektor vlozi znaky vyjadrujuce dlzku zadaneho vektoru
 * 
 * @arg_vector : zadany vektor
 * @arg_vector_length : dlzka zadaneho vektoru
 */
vector<uint8_t> LDAP_tools::add_length(vector<uint8_t> arg_vector){
    int arg_vector_length = arg_vector.size();
    if(arg_vector_length < 0x81){
        arg_vector.insert(arg_vector.begin(), arg_vector_length);
    } else {
        arg_vector.insert(arg_vector.begin(), arg_vector_length);
        arg_vector.insert(arg_vector.begin(), 0x81);
    }
    return arg_vector;
}

/*
 * vrati vektor obsahujuci spravy SearchReqEntry
 * 
 * @message_id : ID spravy SearchRequest na ktoru sa tvori odpoved
 * @filter_result : retazec vo formate cn;mail;uid z ktoreho sa vytvori SearchResEntry
 * @semicolon_position : pozicia bodkociarky ";"
 * @object_name : bazovy objekt v tvare "dn=bazovy objekt"
 * @mail_part : cast odpovede obsahujuca zaobalenu hodnotu mail
 * @cn_part : cast odpovede obsahujuca zaobalenu hodnotu sn
 * @message : vysledna sprava
 */
vector<uint8_t> LDAP_tools::form_search_result(string filter_result, uint8_t message_id){
    size_t semicolon_position = filter_result.find(";");
    string value = filter_result.substr(0, semicolon_position);
    filter_result.erase(0, semicolon_position + 1);
    semicolon_position = filter_result.find(";");
    string mail_value = filter_result.substr(0, semicolon_position);
    filter_result.erase(0, semicolon_position + 1);
    
    string object_name = "dn=" + filter_result;
    
    vector<uint8_t> mail_part;
    vector<uint8_t> cn_part;
    vector<uint8_t> message;
    
    copy(mail_value.begin(), mail_value.end(), back_inserter(mail_part));
    mail_part = this->add_length(mail_part);
    mail_part.insert(mail_part.begin(), 0x04);
    mail_part = this->add_length(mail_part);
    mail_part.insert(mail_part.begin(), 0x31);
    mail_part.insert(mail_part.begin(), 'l');
    mail_part.insert(mail_part.begin(), 'i');
    mail_part.insert(mail_part.begin(), 'a');
    mail_part.insert(mail_part.begin(), 'm');
    mail_part.insert(mail_part.begin(), 0x04);
    mail_part.insert(mail_part.begin(), 0x04);
    mail_part = this->add_length(mail_part);
    mail_part.insert(mail_part.begin(), 0x30);
    
    copy(value.begin(), value.end(), back_inserter(cn_part));
    cn_part = this->add_length(cn_part);
    cn_part.insert(cn_part.begin(), 0x04);
    cn_part = this->add_length(cn_part);
    cn_part.insert(cn_part.begin(), 0x31);
    cn_part.insert(cn_part.begin(), 'n');
    cn_part.insert(cn_part.begin(), 's');
    cn_part.insert(cn_part.begin(), 0x02);
    cn_part.insert(cn_part.begin(), 0x04);
    cn_part = this->add_length(cn_part);
    cn_part.insert(cn_part.begin(), 0x30);
    
    copy(cn_part.begin(), cn_part.end(), back_inserter(message));
    copy(mail_part.begin(), mail_part.end(), back_inserter(message));
    message = this->add_length(message);
    message.insert(message.begin(), 0x30);
    
//    message.insert(message.begin(), object_name[10]);
//    message.insert(message.begin(), object_name[9]);
//    message.insert(message.begin(), object_name[8]);
//    message.insert(message.begin(), object_name[7]);
//    message.insert(message.begin(), object_name[6]);
//    message.insert(message.begin(), object_name[5]);
//    message.insert(message.begin(), object_name[4]);
//    message.insert(message.begin(), object_name[3]);
//    message.insert(message.begin(), object_name[2]);
//    message.insert(message.begin(), object_name[1]);
//    message.insert(message.begin(), object_name[0]);
    for(int i = 10; i >= 0; i--){
        message.insert(message.begin(), object_name[i]);
    }
    message.insert(message.begin(), 0x0b);
    message.insert(message.begin(), 0x04);
    message = this->add_length(message);
    message.insert(message.begin(), 0x64);
    message.insert(message.begin(), message_id);    
    message.insert(message.begin(), 0x01);
    message.insert(message.begin(), 0x02);
    message = this->add_length(message);
    message.insert(message.begin(), 0x30);
    
    return message;
}