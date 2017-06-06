#include <iostream>
#include <exception>
using namespace std;

class Base{
   virtual void dummy(){}
};

class SubBase : public Base{
    int a;
};

int main(){
    try{
      Base* subB = new SubBase;
      Base*  b = new Base;
      SubBase* subBase;
      subBase = reinterpret_cast<SubBase*>(subB);
      if(subBase == 0){
        std::cout << "convert subBase failed" << '\n';
      }
     cout<< typeid(*subB).name() << "\n";
     cout<< typeid(*b).name() << "\n";

      subBase = reinterpret_cast<SubBase*>(b);
      if(subBase == 0){
        std::cout << "convert Base failed" << '\n';
      }


    }catch(exception& e){
       std::cout << "exception " << e.what() << '\n';
    }
}
