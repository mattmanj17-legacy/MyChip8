#ifndef DEBBUGER_H_
#define DEBBUGER_H_
#include "VCPU.h"
#include <iostream>
using namespace std;
class Debugger{
private:
	VCPU* instance;
public:
	Debugger(VCPU* instance){
		this->instance=instance;
	}
	void debugPrintScreen(){
		for(int i=0;i<66;i++){
			cout<<"+";
		}
		cout<<endl;
		for(int j=0;j<32;j++){
			cout<<"+";
			for(int i=0;i<64;i++){
				if(instance->screen[i][j]==true)
					cout<<"0";
				else
					cout<<" ";
			}
			cout<<"+"<<endl;
		}
		for(int i=0;i<66;i++){
			cout<<"+";
		}
		cout<<endl;
	}
};
#endif