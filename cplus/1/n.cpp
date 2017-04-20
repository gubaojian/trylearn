#include <iostream>
using namespace std;
namespace config{
    int x = 30;
}


int main ()
{
  int y;
  cout << y << '\n';

 using namespace config;

 cout << x << '\n';

  std::cout << "config " << config::x << '\n';
  return 0;
}
