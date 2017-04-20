#include <iostream>
#include <string>

using namespace std;

void plusa(int a, int b){
  a += 2;
  b +=2;
}

void plusRef(int& a, int& b){
   a+=2;
   b+=2;
}


string inline concat(const string& a, const string& b)
{
  return a+b;
}


int main(int argc, char const *argv[]){
    int a=3, b=5;
    std::cout << a << "  " << b << '\n';
    plusa(a, b);
    std::cout << a << "  " << b << "\n";
    plusRef(a, b);
    std::cout << a << "  " << b << "\n";
    string result = concat("aaa", "bbb");
    std::cout << " hello world " << result  << '\n';
}
