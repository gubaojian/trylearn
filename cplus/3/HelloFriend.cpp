#include <iostream>


class Rectangle {
private:
   int width, height;
 public:
   Rectangle(){}
   Rectangle(int x, int y): width(x), height(y){}
   int area(){return width*height;}
   friend Rectangle duplicateRect(const Rectangle&);
};

Rectangle duplicateRect(const Rectangle& param){
    Rectangle res;
    res.width = param.width*2;
    res.height = param.height*2;
    return res;
}

int main(){
    Rectangle foo;
    Rectangle bar (3, 4);
    foo = duplicateRect(bar);

    std::cout << foo.area() << "\n";
    return 0;
}
