#include <iostream>

using namespace std;

int add(int a, int b){
  return a + b;
}

int main(int argc, char const *argv[]) {

  int  a = 25;

  int* p = &a;

  cout << p << " " << a << endl;
  cout << *p++ << " " << a << endl;


  int myarray[20];
  int* mypointer;
  for(int i=0; i<20; i++){
    myarray[i] = i;
  }

  for(int n : myarray){
    cout << n << endl;
  }

  int *cur = myarray;
  int *end = myarray + 20;
  while (cur != end) {
     ++*cur;
     cout << "cur " << (*cur) << endl;
     cur++;
  }


  for(int n : myarray){
    cout << " count " << n << endl;
  }

  char e = 'c';
  char *f;
  char ** g;
  f = &e;
  g = &f;

  cout << e  <<  " f " << f  << " g " << g << endl;

  int (*funcp)(int,int) = add;

  cout << funcp(10, 13) << endl;
  cout << funcp(20, 13) << endl;


  /* code */
  return 0;
}
