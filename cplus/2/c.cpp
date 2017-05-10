#include <iostream>
#include <string>

using namespace std;

class Person{
private:
  int age;
  int month;
  string name;
  static int count;

public:
  Person(string n){name = n; count++;}
  string get_name(){return name;};
  void set_name(string n){name = n;};
  void say(){
      std::cout << "name " << name  << " age " << age << " count " << count << '\n';
  };
  void set_age(int);
  Person operator +(const Person&);
};

int Person::count = 0;

void Person::set_age(int a){
   age = a;
};

Person Person::operator+(const Person& p){
     std::cout << this->name << '\n';
     Person plus("plus");
     plus.set_age(age + p.age);
     plus.set_name(name + " plus " + p.name);
     return plus;
}



int main(int argc, char const *argv[]) {

   Person person("谷宝剑");
   person.set_age(100);
   person.say();
   Person person2 {"谷宝剑2"};

   person2.say();

   Person* person3 = &person2;
   person3->say();

   Person plus =  person + person2;

   plus.say();

  /* code */
  return 0;
}
