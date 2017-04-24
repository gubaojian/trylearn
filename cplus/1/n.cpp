#include <iostream>
using namespace std;
namespace config{
    int x = 30;
}

namespace bar{
  const double pi = 3.1416;
  double pi_value(){
     return pi;
  }
}


int main ()
{
  int y;
  cout << y << '\n';

 using namespace config;

 cout << x << '\n';

  std::cout << "config " << config::x << '\n';

  std::cout << "config pi value " << bar::pi_value() << '\n';
  return 0;
}
