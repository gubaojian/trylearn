#include <iostream>
#include <array>

using namespace std;

int main(){

  int* foo = new (nothrow) int[10];

  std::cout << foo[0] << std::endl;

   delete[] foo;

   int bar[] = {10, 20, 30};
   std::cout << bar[1] << std::endl;



  array<int,3> arrs = {13, 25, 16};

  for(int i=0; i<arrs.size(); i++){
       ++arrs[i];
  }

  for(int arr : arrs){
     cout << arr << endl;
  }


  char loves[] = "hello";


  std::cout << (loves[5] == '\0');

  char question1[] = "What is your name? ";
  string question2 = "Where do you live? ";
  char buffer[40];
  string strBuffer;
  cout << question1;
  cin >> buffer;
  cout << question2;
  cin >> strBuffer;
  cout << "Hello, World" << buffer;
  cout << " from " << strBuffer << "!\n";


  char myntcs[] = "some text";
  string mystring = myntcs;

  cout << mystring;
  cout << mystring.c_str();

  for(char ch : mystring){
    std::cout << ch << '\n';
  }

  return 0;
}
