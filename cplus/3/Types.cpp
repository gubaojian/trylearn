#include <iostream>
using namespace std;

class A{

};

class B{
public:
  B(const A& x){cout << "constructor" <<"\n";}
  B& operator= (const A& x){
    cout << "operator=" <<"\n";
    return *this;
  }

  operator A(){
    cout << "operatorA" <<"\n";
    return A();
  }

};

int main(){

  A foo;
  B bar = foo;
  bar = foo;

  foo = bar;

  cout << &foo << &bar << "\n";


  return 0;
}
