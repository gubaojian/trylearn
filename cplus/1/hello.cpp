#include <iostream>
#include <string>
#include <sstream>
using namespace std;
int main(){

  string buffer;
  float price = 0;
  int quantity = 0;

  cout << "Enter prices:\n";
  getline(cin, buffer);
  stringstream(buffer) >> price;
  cout << "Enter Quantity:\n";
  getline(cin, buffer);
  stringstream(buffer) >> quantity;
  cout << price << " * " << quantity << " = " << (price*quantity) << endl;





  return 0;
}
