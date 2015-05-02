#include <iostream>

using namespace std;
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
					else if(operands[0][0] =='c')
						temp.opcode+= operands[0][j];
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
				temp.opcode+= curr_mnemonic.opcode;
				string ni,xbpe;
				ni= statements[i].n+'0';
				ni+= statements[i].i+'0';
				
				xbpe= statements[i].x+'0';
				xbpe+= statements[i].b+'0';
				xbpe+= statements[i].p+'0';
				xbpe+= statements[i].e+'0';
				
				if(curr_mnemonic.mnemonic!="RSUB" && temp.s.e==0)
					xbpe[2]='1';
				
				string sec= "";
				sec+= temp.opcode[1];
				int second_digit= hexa_to_int(sec);
				if(curr_mnemonic.mnemonic == "STL"){
					cout << ni+xbpe << endl;
					cout << sec << endl;
				}
				second_digit+= bin_to_int(ni);
				sec= int_to_hexa(second_digit, 1);
				temp.opcode[1]= sec[0];
				
				int third_digit= bin_to_int(xbpe);
				temp.opcode+= int_to_hexa(third_digit, 1);
				
				int disp= 0;
				
				if(statements[i].e){
					if(curr_mnemonic.no_args > 0){
						disp= SYMTAB[operands[0]];
					}
					temp.opcode+= int_to_hexa(disp, 5);
				}
				else{
					if(curr_mnemonic.no_args > 0){
						disp= SYMTAB[operands[0]]-statements[i+1].address;
					}
					temp.opcode+= int_to_hexa(disp, 3);
				}
			}
		}
		op_codes.push_back(temp);
	}
}

int main(){
	
}
