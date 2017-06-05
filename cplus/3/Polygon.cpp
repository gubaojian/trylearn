#include <iostream>
using namespace std;

class Polygon{
    protected:
      int width, height;
    public:
      void setWidthHeight(int a, int b){
          width = a;
          height = b;
      }
      virtual int area (){
        return width*height;
      }
};
class Rectangle : public Polygon{
   public:
      int area(){
        return width*height;
      }
};

class Triangle:public Polygon{
    public:
     int area(){
        return width*height/2;
     }
};

int main(){
  Rectangle rect;
  Triangle trgl;
  rect.setWidthHeight(2, 2);
  trgl.setWidthHeight(2,2);
  std::cout << rect.area() << '\n';
  std::cout << trgl.area() << '\n';

  return 0;
}
