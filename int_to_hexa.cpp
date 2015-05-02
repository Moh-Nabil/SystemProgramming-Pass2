#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;


//Testung function
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

int main(){
	while(1){
		int n, digit; cin >> n >> digit;
		cout << int_to_hexa(n) << " " << int_to_hexa(n,digit) << endl;
	}
}
