#include <iostream>
using namespace std;
int main(int argc, char const *argv[]) {

   int n= 20;
  label:
      cout << n << endl;
      n--;
    if(n > 0){
       goto  label;
    }

  std::cout << "endle" << '\n';


  std::cout << "string euqals: " << (strcmp("hello", "hello")) << '\n';

  if(strcmp("hello", "hello") == 0){
      printf("hello equals hello\n");
  }


  return 0;
}
