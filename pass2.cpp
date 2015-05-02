#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <fstream>
#include <algorithm>
#include <limits.h>
#include <vector>
#include <locale>
#include <set>
#include<math.h>
#include <string>
using namespace std;

////						**************TABLES****************
//*****************SYMTABLE
map<string, int> SYMTAB;
//*****************OPTABLE
struct op_line{
	string mnemonic, opcode;
	int format, no_args;
	char input_type, category;
	bool directive_flag, good;


	// This is function can be used for debugging and testing purposes
	void print(struct op_line line){
		cout << line.mnemonic << "\t"
			 << line.input_type << "\t"
			 << line.format << "\t"
			 << line.opcode << "\t"
			 << line.no_args << "\t"
			 << line.directive_flag << "\t"
			 << line.category << "\t"
			 << line.good << endl;
	}
};
struct OP_TABLE{
	map<string, struct op_line> m;

	// Used to get operations from the op_table
	struct op_line get(string str){
		for(int i= 0; i< (int)str.size(); i++)
			str[i]= toupper(str[i]);
		if(m.count(str) == 0){
			struct op_line dumi;
			dumi.good= 0;
			return dumi;
		}
		else{
			return m[str];
		}
	}

	// Used to add the op_file to the map and start working normally.
	void create(){
		ifstream file;
		file.open("op_file.txt");
		string check;
		while(file >> check){
			struct op_line line;
			line.mnemonic= check;
			file >> line.input_type;
			file >> line.format;
			file >> line.opcode;
			file >> line.no_args;
			file >> line.directive_flag;
			file >> line.category;
			line.good= 1;
			m[check]= line;
//			line.print(line);
		}
		file.close();
	}

// This is function can be used for debugging and testing purposes
	void print(struct op_line line){
		line.print(line);
	}
};
//   mnemonic    input_type    format     opcode    no_args     directive_flag     category
//
//
//   input_type:    	"m"  memory or address.
//                  	"r"  register.
//                  	"n"  no input, ex: RSUB.
//                  	"d"  directive input.
//
//   format:        	3/4  format  ------->   5
//
//   opcode:            FF   is given to directives as we don't have any codes for them.
//
//   directive_flag:    1    if directive mnemonic
//                      0    if not directive
//							**************VALID****************
struct statement {
	string label;
	string mnemonic;
	vector<string> operand;
	string comment;
	string line;
	int error;
	int address;
	bool n, i, x, b, p, e;
	bool is_comment;
};
vector<statement> statements;
string errors[] = {"Illegal Format.", 				//0
				   "Illegal start statement.", 		//1
				   "Illegal end statement.", 		//2
				   "Illegal label.",				//3
				   "Illegal mnemonic.",				//4
				   "Illegal operand(s).",       	//5
				   "Illegal Addressing Mode.",  	//6
				   "odd length for hex string.",		//7
				   "duplicate label definition.", 		//8
				   "Out of Range",						//9
				   "Operand doesn't exist"};				//10
				

bool caseIn_str_comp(string s1 , string s2)
{
    if(s1.size() != s2.size())
        return false ;

    for(int i = 0 ; i<(int)s1.size() ; i++)
    {
        if(tolower(s1[i])!=tolower(s2[i]))
            return false ;
    }
    return true ;


}
bool isNum(string s)
{
    char const* temp = s.c_str() ;

    for(int i = 0 ; i <(int)s.size() ; i++)
    {
        if(!isdigit(temp[i]))
            return false;
    }
    return true ;
}
int find_statement(string s) {
	int n = statements.size();
	if (s.compare("start") == 0) {
		for (int i = 0; i < n; i++)
			if (!statements[i].is_comment)
				return i;
	} else {
		for (int i = n - 1; i >= 0; i--)
			if (!statements[i].is_comment)
				return i;
	}
	return -1;
}
bool is_valid_symbol(string s){
	if (!isalpha(s[0]) && s[0] != '$')
		return false;
	for (int i = 1; i < (int)s.size(); i++)
		if (!isalnum(s[i]) && s[i] != '$')
			return false;
	return true;
}
bool is_valid_label(statement &s) {
	if(s.label.size()!=0 && !is_valid_symbol(s.label)){
		s.error=3;
		return false;
	}
	return true;
}
void insert_SYMTAB(statement &s){
	if(SYMTAB.count(s.label))
		 s.error = 8;
	else
		SYMTAB[s.label]= 0;;
}
struct OP_TABLE op_table;
////						****************addressing and printing*****************
//from hexa to integer and vice versa
int hexa_to_int (string s){
    int res = 0;
    for(int i=0;i<(int)s.size();i++){
        int now;
        if(s[i]=='A' || s[i]=='a')
            now = 10;
        else if(s[i]=='B' || s[i]=='b')
            now=11;
        else if(s[i]=='C' || s[i]=='c')
            now=12;
        else if(s[i]=='D' || s[i]=='d')
            now=13;
        else if(s[i]=='E' || s[i]=='e')
            now=14;
        else if(s[i]=='F' || s[i]=='f')
            now=15;
        else
            now = s[i]-'0';
        res*=16;
        res+=now;
    }
    return res;
}
string int_to_hexa(int n){
    vector<char> temp ;
    while(n>0){
        int now = n%16;
        if(now<10)
            temp.push_back(now+'0');
        else
            temp.push_back((now%10)+'A');
        n/=16;
    }
    while(temp.size()<6){
        temp.push_back('0');
    }
    string res;
    for(int i=temp.size()-1;i>=0;i--){
        res+=temp[i];
    }
    return res;
}
// assigning addresses
void assignAddress(OP_TABLE table){
    int ind = 0,start=0;
    while(statements[ind].is_comment)
        ind++;
    if(statements[ind].mnemonic.compare("start")==0 && statements[ind].error==-1){
        start = hexa_to_int(statements[ind].operand[0]);
        statements[ind].address = start;
    }
    int curr = start;
    op_line line ;
    for(int i=ind+1;i<(int)statements.size();i++){
        if(statements[i].is_comment)
            continue;
        else if(statements[i].error!=-1){
            statements[i].address=curr;
            continue;
        }
        line = table.get(statements[i].mnemonic);
        statements[i].address=curr;
        /////
        if(SYMTAB.count(statements[i].label)==1 && SYMTAB[statements[i].label]==0){
			SYMTAB[statements[i].label]= curr;
		}
        //////
        if(line.directive_flag==false){
            if(line.format==5 && statements[i].e==true)
                curr+=4;
            else if(line.format==5)
                curr+=3;
            else
                curr+=line.format;
        }
        else{
            if(statements[i].mnemonic.compare("resb")==0){
                int operand = atoi(statements[i].operand[0].c_str());
                curr+=operand;
            }
            else if(statements[i].mnemonic.compare("resw")==0){
                int operand = atoi(statements[i].operand[0].c_str());
                curr+=operand*3;
            }
            else if(statements[i].mnemonic.compare("word")==0){
                int operands = statements[i].operand.size();
                curr+=operands*3;
            }
            else if(statements[i].mnemonic.compare("byte")==0){
                string operand = statements[i].operand[0];
                if(operand[0]=='x' || operand[0]=='X'){
                    curr+=(operand.length()-3)/2;
                }
                else if(operand[0]=='c' || operand[0]=='C'){
                    curr+=operand.length()-3;
                }
            }
        }
    }
}
// output file

void output(){
    ofstream file ;
    file.open("output.txt");
    for(int i=0;i<(int)statements.size();i++){
        int digits = log10(i+1)+1;
        file<<i+1;
        for(int j=0;j<8-digits;j++)
            file<<" ";
        if(statements[i].is_comment)
            file<<"             "<<statements[i].line<<endl;
        else{
            file<<int_to_hexa(statements[i].address)<<"       "<<statements[i].line<<endl;
            if(statements[i].error!=-1){
                file<<"                     ****"<<errors[statements[i].error]<<endl;
            }
        }
    }
    file<<"\n\n"<<"**********************SYMBOL TABLE**************************"<<endl<<endl;
    file <<"\t\t\t" <<"Symbol" << "\t\t" << "Address"<< endl;

    map <string, int> :: iterator it;
		for(it= SYMTAB.begin(); it!= SYMTAB.end(); it++){
			pair<string, int> temp;
			temp.first= it->first;
			temp.second= it->second;
			file <<"\t\t\t" <<temp.first << "\t\t" << int_to_hexa(temp.second )<< endl;
    }
    file.close();
}
///////////////////////fathallah's notes //////////////////////////////
/*
    -starting address is 0000 by default , unless  start statement is correct and has another address.

    -in case of error jump the line without increasing address.

    -in case of resW , address +=operand*3
     in case of resB , address+=operand

    -in case of word , address+=3;
     there may be a list of words separated by commas , address+=3*operands.size();

    -in case of Byte , X'sth' must has even length , address+=len/2
                       C'sth' add+=len

    - shokran :D :D

*/
////						**************VStart****************
void validate_start_operand(statement &s) {
	if(s.error!=-1)
		return;
	if(s.operand.size()!=1 || s.operand[0].size()>4){
		s.error = 5;
		return;
	}
	for(int i=0; i<(int)s.operand[0].size(); i++)
		if(!isxdigit((char)s.operand[0][i])){
			s.error = 5;
			return;
		}
}
void validate_start() {
	int idx = find_statement((string)"start");
	if (idx == -1) { // program without start statement !
		statements[0].error = 1;
		return;
	}
	if (statements[idx].error != -1)
		return;
	if (statements[idx].label.size()>0 && !is_valid_label(statements[idx]))
		return;
	if (statements[idx].mnemonic.compare("start") != 0) {
		statements[idx].error = 1;
		return;
	}
	validate_start_operand(statements[idx]);
}
////						****************VEnd*****************
void validate_end_operand(statement &s) {
	if (s.operand.size()==0 || (s.operand.size()==1 && is_valid_symbol(s.operand[0])) )
		return;
    s.error = 5;
}
void validate_end() {
	int idx = find_statement((string)"end");
	if (idx == -1) { // program without end statement !
		statements[statements.size()-1].error = 2;
		return;
	}
	if (statements[idx].error != -1)
		return;
	if (statements[idx].mnemonic.compare("end") != 0) {
		statements[idx].error = 2;
		return;
	}
	if (statements[idx].label.size() != 0){
			  statements[idx].error = 3;
			  return ;
	    }
	validate_end_operand(statements[idx]);
}
////						******************VBody*****************

bool is_dec_num(string s){
	for(int i=0; i<(int)s.size(); i++)
		if(!isdigit(s[i]))
			return false;
	return true;
}
bool is_reg(string s){
	return (s.size()==1 && (s[0]=='a' || s[0]=='x' || s[0]=='l' || s[0]=='b' || s[0]=='s' || s[0]=='t'));
}
void validate_operand_catC(statement &s){
	if(s.n && s.i && !s.x && !s.b && !s.p && !s.e && s.operand.size()==2 && is_reg(s.operand[0]) && is_reg(s.operand[1]))
		return;
	s.error = 5;
}
void validate_operand_catB(statement &s){
	if(s.n && s.i && !s.x && !s.b && !s.p && !s.e && s.operand.size()==1 && is_reg(s.operand[0]))
		return;
	s.error = 5;
}
void validate_operand_catE(statement &s){
	if(s.n && s.i && !s.x && !s.b && !s.p && !s.e && s.operand.size()==0)
		return;
	s.error = 5;
}
void validate_operand_catF(statement &s){
	if(s.n && s.i && !s.x && !s.b && !s.p && !s.e && s.operand.size()==1 && is_dec_num(s.operand[0]) && s.operand[0].size()<=4 )
		return;
	s.error = 5;
}
void validate_operand_catH(statement &s){
	if(s.n && s.i && !s.x && !s.b && !s.p && !s.e){
		bool valid = true;
		for(int i=0; i<(int)s.operand.size(); i++)
			if(is_dec_num(s.operand[0])==false || s.operand[0].size()>4){
				valid = false;
				break;
			}
		if(valid)
			return;
	}
	s.error = 5;
}
void validate_catA(statement &s)
{
    if((!s.n && !s.i)||((!s.n||!s.i)&&s.x))
    {
        s.error = 6;
        return;
    }
     if(s.operand.size() == 1 && (is_valid_symbol(s.operand[0]) || isNum(s.operand[0]) || (s.operand[0].compare("*")==0)))
    	 return;

     s.error = 5;
}
void validate_catD(statement &s )
{
    if(s.operand.size()==2 && is_reg(s.operand[0]) && isNum(s.operand[1])
         && s.n && s.i && !s.e)
        return;

    s.error = 5 ;
}
void validate_catG(statement &s)
{
     if(s.operand.size()!=1 || !s.n || !s.i || s.x || s.e)
     {
         s.error = 5 ;
         return;
     }

     string str = s.operand[0] ;
     if(str[1]!='\'' || str[str.size()-1]!='\'')
     {
         s.error = 5 ;
         return;

     }
     else if(tolower(str[0])=='c')
     {
         for(int i = 2 ; i< (int)str.size()-1 ; i++)
         {
             if(str[i]=='\'')
             {
                 s.error = 5 ;
                 return;
             }

         }

     }
     else if(tolower(str[0])=='x')
     {   std::locale loc;

         for(int i = 2 ; i< (int)str.size()-1 ; i++)
         {
             if(!std::isxdigit(str[i],loc))
             {
                 s.error = 5 ;
                 return ;
             }

         }
         if((str.size()-3)%2 != 0)
         {
            s.error = 7;
            return ;
         }

     }

}


bool is_valid_mnemonic(statement &s) {
	if(!op_table.get(s.mnemonic).good){
		s.error = 4;
		return false;
	}
	return true;
}
void validate_operands(statement &s){//
	//category of mnemonic
	switch(op_table.get(s.mnemonic).category){
	case 'A':
		validate_catA(s);
		break;
	case 'B':
		validate_operand_catB(s);
		break;
	case 'C':
		validate_operand_catC(s);
		break;
	case 'D':
		validate_catD(s);
		break;
	case 'E':
		validate_operand_catE(s);
		break;
	case 'F':
		validate_operand_catF(s);
		break;
	case 'G':
		validate_catG(s);
		break;
	case 'H':
		validate_operand_catH(s);
		break;
	default:
		break;
	}
}

void validate_body() { //
	int st = find_statement((string)"start");
	int e = find_statement((string)"end");
	for(int i=st+1; i<e; i++){
		statement &s = statements[i];
		if(s.is_comment || s.error != -1) continue;
		if(!is_valid_label(s)) continue;
		if(s.label.size()!=0) insert_SYMTAB(s);
		if(s.error != -1) continue;
		if(!is_valid_mnemonic(s)) continue;
		validate_operands(s);
	}
}
////						****************Validate*****************
void validate() {
	validate_start();
	validate_body();
	validate_end();
}
////						****************Parsing*****************
statement temporary;string tempOperand;
bool check_label(string label){
    if(label.find_first_not_of(' ') == std::string::npos ){
        temporary.label = "";
    }
    else if(label.find_first_of(' ') ==std::string::npos){
        temporary.label = label;
    }
    else if(label.at(0)==' '){
        return false;
    }
    else{
        int first_space = label.find_first_of(' ');
        int last_char = label.find_last_not_of(' ');
        if(first_space<last_char){
            return false;
        }
        else{
           temporary.label = label.substr(0,first_space);
        }
    }
    return true;
}
bool check_mnemonic(string mnemonic){
    if(mnemonic.at(0)==' ' ){
        return false;
    }
    else if(mnemonic.find_first_of(' ') ==std::string::npos){
        temporary.mnemonic =mnemonic;
    }
    else{
        int first_space = mnemonic.find_first_of(' ');
        int last_char = mnemonic.find_last_not_of("\n ");
        if(first_space<last_char){
            return false;
        }
        else{
            temporary.mnemonic = mnemonic.substr(0,first_space);
        }
    }
    return true;
}
bool check_operand(){
    if(isspace(tempOperand.at(0)) ){
        return false;
    }
    else if(tempOperand.find_first_of(' ') !=std::string::npos){
        int first_space = tempOperand.find_first_of(' ');
        int last_char = tempOperand.find_last_not_of(' ');
        if(first_space<last_char && tempOperand.find_first_of("c'") ==std::string::npos){
            return false;
        }
        else if(tempOperand.find_first_of("c'") ==std::string::npos)
            tempOperand = tempOperand.substr(0,first_space);
        else{
            int last_com = tempOperand.find_last_of("'");
            last_char = tempOperand.find_last_not_of(' ');
            if(last_com<last_char)
                return false;
            else{
                tempOperand = tempOperand.substr(0,last_com+1);
            }

        }
    }
    if(!isalpha(tempOperand.at(0)) && !isdigit(tempOperand.at(0)) ){     //handle n,i
        if(tempOperand.at(0) == '@'){
            temporary.i=false;
            tempOperand = tempOperand.substr(1,tempOperand.length()-1);
        }
        else if(tempOperand.at(0) == '#'){
            temporary.n=false;
            tempOperand = tempOperand.substr(1,tempOperand.length()-1);
        }

    }
    return true;
}
bool parse_operand(){
    //handle x
    if(tempOperand.length() > 1 && tempOperand.at(tempOperand.length()-2) == ',' &&
       tempOperand.at(tempOperand.length()-1) == 'x' &&
       temporary.mnemonic.at(temporary.mnemonic.length()-1)!='r' && temporary.mnemonic !="rmo" ){
            temporary.x = true;
            tempOperand.resize(tempOperand.length()-2);

    }
    if(tempOperand.find_first_not_of(' ')==std::string::npos ){}
    else if(tempOperand.find_first_of(',') ==std::string::npos){
        temporary.operand.push_back(tempOperand);
    }
    else {
        int start = 0;
        int i;
        for(i=0;i<(int)tempOperand.length();i++){
            if(tempOperand.at(i) == ','){
                temporary.operand.push_back(tempOperand.substr(start,i-start));
                start = i+1;
            }
        }
        temporary.operand.push_back(tempOperand.substr(start,i-start));
    }
    return true;
}
void read_file(char file[]){

    string line;
    char ext[] = ".txt";
    ifstream myfile (strcat(file, ext));
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            temporary.line=line;temporary.error=-1;temporary.is_comment=false;
            temporary.x=false;temporary.e=false;temporary.n=true;temporary.i=true;
            temporary.operand.clear();
            if(line.empty()|| line.find_first_not_of(' ') == std::string::npos ){
                continue;
            }
            if(line.at(0)=='.'){
                temporary.is_comment=true;
                statements.push_back(temporary);
                continue;
            }
            std::transform(line.begin(), line.end(), line.begin(), ::tolower);
            if(line.length()<9 || ( line.at(8)!=' ' && line.at(8)!='+' )||line.find_first_of('\t') != std::string::npos ||
               (line.length()>18 && (line.at(15)!=' ' || line.at(16)!=' ') ) ||line.length()<10 ){
                temporary.error=0;
                statements.push_back(temporary);
                continue;
            }

            if(line.at(8) == '+') temporary.e=true;
            string label = line.substr(0,8);
            string mnemonic = line.substr(9,6);
            if(line.length() > 17) tempOperand = line.substr(17,18);
            else tempOperand=" ";
            if(!check_mnemonic(mnemonic) ||!check_label(label) ){
                temporary.error=0;
                statements.push_back(temporary);
                continue;
            }
            if   (tempOperand.length()==1 && tempOperand.at(0)=='*') {
                temporary.operand.push_back(tempOperand);
            }

            else if(tempOperand.find_first_not_of(' ') !=std::string::npos ){
                if(!check_operand() || !parse_operand()){
                    temporary.error=0;
                    statements.push_back(temporary);
                    continue;
                }
            }
            if(line.length() > 36){
                temporary.comment = line.substr(35,31);
            }
            statements.push_back(temporary);
        }
    }
}
////
////						****************Main*****************

map <string, int> registers;

void load_registers(){
	ifstream file_r;
	file_r.open("registers.txt");
	string r;
	if(file_r.is_open()){
		while(file_r >> r){
			int num; file_r >> num;
			registers[r]= num;
		}
	}
}

//New function, with new paramter for number of trailling zeroes.
string int_to_hexa(int n, int digit){
	string res= "";
	while(n>0){
		int curr= n%16;
		if(curr<10)
			res+= curr+'0';
		else
			res+= curr+'A'-10;
		n/= 16;
	}
	
	while((int)res.size() < digit)
		res+= '0';
	reverse(res.begin(), res.end());
	return res;
}

int bin_to_int(string bin){
	int ret= 0;
	reverse(bin.begin(), bin.end());
	for(int i= 0; i< (int)bin.size(); i++){
		ret+= (bin[i]-'0') * pow(2, i);
	}
	
	return ret;
}

struct op_code{
	statement s;
	string opcode;
};

vector <op_code> op_codes;

char bin_to_hexa(string bin){
	if(bin == "0000")
		return '0';
	else if(bin == "0001")
		return '1';
	else if(bin == "0010")
		return '2';
	else if(bin == "0011")
		return '3';
	else if(bin == "0100")
		return '4';
	else if(bin == "0101")
		return '5';
	else if(bin == "0110")
		return '6';
	else if(bin == "0111")
		return '7';
	else if(bin == "1000")
		return '8';
	else if(bin == "1001")
		return '9';
	else if(bin == "1010")
		return 'A';
	else if(bin == "1011")
		return 'B';
	else if(bin == "1100")
		return 'C';
	else if(bin == "1101")
		return 'D';
	else if(bin == "1110")
		return 'E';
	else if(bin == "1111")
		return 'F';
	return 'H';
}

string neg_int_to_hexa(int n, int digit){
	string ret= "";
	for(int i= 0; i< digit; i++){
		string curr= "";
		for(int j= 4*i; j< 4*i+4; j++){ 
			curr += ((n&(1<<j))!=0) + '0';
		}
		reverse(curr.begin(), curr.end());
		ret+= bin_to_hexa(curr);
	}
	reverse(ret.begin(), ret.end());
	
	return ret;
}

void generate_op_code(){
	
	
	for(int i= 0; i< (int)statements.size(); i++){
		if(statements[i].is_comment)
			continue;
		op_code temp;
		temp.s= statements[i];
		op_line curr_mnemonic= op_table.get(temp.s.mnemonic);
		vector<string> operands= statements[i].operand;
		if(curr_mnemonic.directive_flag || statements[i].is_comment){
			if(curr_mnemonic.mnemonic == "WORD"){
				int operand = atoi(operands[0].c_str());
				temp.opcode= int_to_hexa(operand,6);
			}
			else if(curr_mnemonic.mnemonic == "BYTE"){
				temp.opcode= "";
				for(int j= 2; j< (int)operands[0].size()-1; j++){
					if(operands[0][0] =='x')
						temp.opcode+= toupper(operands[0][j]);
					else if(operands[0][0] =='c'){
						cout << (int)operands[0][j] << " " << operands[0][j] << endl;
						temp.opcode+= int_to_hexa((int)operands[0][j],2);
					}
				}
			}
			//if(curr_mnemonic.mnemonic=="resb" || curr_mnemonic.mnemonic=="resw")
		}
		else{
			temp.opcode= "";
			if(curr_mnemonic.format == 2){
				temp.opcode+= curr_mnemonic.opcode;
				temp.opcode+= int_to_hexa(registers[operands[0]], 1);
				if(curr_mnemonic.mnemonic=="CLEAR" || curr_mnemonic.mnemonic=="TIXR")
					temp.opcode+= '0';
				else
					temp.opcode+= int_to_hexa(registers[operands[1]], 1);
			}
			else{
				bool imed_digit= false;
				temp.opcode+= curr_mnemonic.opcode;
				string ni,xbpe;
				ni= statements[i].n+'0';
				ni+= statements[i].i+'0';
				
				xbpe= statements[i].x+'0';
				xbpe+= statements[i].b+'0';
				xbpe+= statements[i].p+'0';
				xbpe+= statements[i].e+'0';
				
				if(ni == "11"){
					xbpe[2]= !statements[i].e+'0';
				}
				else if(ni != "00"){
					if(isalpha(operands[0][0]))
						xbpe[2]= !statements[i].e+'0';
					else
						imed_digit= true;
				}
				
				string sec= "";
				sec+= temp.opcode[1];
				int second_digit= hexa_to_int(sec);
				
				second_digit+= bin_to_int(ni);
				sec= int_to_hexa(second_digit, 1);
				temp.opcode[1]= sec[0];
				
				int third_digit= bin_to_int(xbpe);
				temp.opcode+= int_to_hexa(third_digit, 1);
				
				int disp= 0;
				
				if(statements[i].e){
					if(curr_mnemonic.no_args > 0){
						disp= SYMTAB[operands[0]];
						if(imed_digit)
							disp= atoi(operands[0].c_str());
					}
					temp.opcode+= int_to_hexa(disp, 5);
				}
				else{
					if(curr_mnemonic.no_args > 0){
						disp= SYMTAB[operands[0]]-statements[i+1].address;
						if(imed_digit)
							disp= atoi(operands[0].c_str());
					}
					if(disp<0)
						temp.opcode+= neg_int_to_hexa(disp, 3);
					else
						temp.opcode+= int_to_hexa(disp, 3);
				}
				
				if(curr_mnemonic.mnemonic == "RSUB")
					temp.opcode= "4F0000";
			}
		}
		op_codes.push_back(temp);
	}
}


int main() {
	op_table.create();
	load_registers();
	while(true){
		string file;
		cin >> file;
		cin >> file;
		char f[222];
		strcpy(f, file.substr(1,file.size()-2).c_str());

		statements.clear();
		SYMTAB.clear();
		read_file(f);
		validate();
		assignAddress(op_table);
		output();
		generate_op_code();
		for(int i= 0; i< (int)op_codes.size(); i++)
			cout << op_codes[i].opcode << " " << op_codes[i].s.mnemonic << endl;
	}
}

