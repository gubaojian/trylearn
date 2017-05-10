#include <iostream>
struct Person{
  int age;
  const char*  name;
};

int main(int argc, char const *argv[]) {
   Person person = {20, "hello world"};

   Person persons[] = {{20, "hello world"}};

   Person* p = &person;

   std::cout << "person " << person.age << '\n';
   std::cout << "person " << person.name << '\n';

    std::cout << "person " << p->name << '\n';

    std::cout << persons[0].name << '\n';

    std::cout << sizeof(persons) << sizeof(Person) << '\n';


    
  /* code */
  return 0;
}
