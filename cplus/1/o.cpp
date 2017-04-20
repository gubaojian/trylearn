#include <iostream>
template <class T>
T sum(T a, T b){
  T result;
  result = a + b;
  return result;
}

template<class T, int N>
T multi(T num){
  return num*N;
}


int main(int argc, char* args[]){

  std::cout << sum<double>(4.0, 2) << '\n';

  std::cout << multi<int, 10>(31) << '\n';


  return 0;
}
