#include <iostream>
#include <algorithm>
#include <math.h>
using namespace std;

int bin_to_int(string bin){
	int ret= 0;
	reverse(bin.begin(), bin.end());
	for(int i= 0; i< (int)bin.size(); i++){
		ret+= (bin[i]-'0') * pow(2, i);
	}
	
	return ret;
}

int main(){
	while(1){
		string n; cin >> n;
		cout << bin_to_int(n) << endl;
	}
}
