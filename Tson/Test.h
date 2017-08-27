//
// Created by furture on 2017/8/5.
//

#ifndef HELLO_TEST_H
#define HELLO_TEST_H

int is_little(){
    int i = 1;
    char *p = (char *)&i;
    if(*p == 1)
        return 1;
    else
        return 0;
}

#endif //HELLO_TEST_H
