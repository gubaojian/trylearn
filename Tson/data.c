//
// Created by furture on 2017/8/26.
//

#include <stdio.h>
#include <stdlib.h>

char* readJSONFile(char const* fileName){

    FILE* fp;
    fp = fopen(fileName, "r");
    if(!fp){
        printf("error open %s", fileName);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    printf("read %ld   %ld \n", length, ftell(fp));
    char* buffer = malloc(sizeof(char)*length);
    while(length > 0){
        int read = fread(buffer, sizeof(char), length, fp);
        length -=read;
    }
    fclose(fp);
    printf("read end %ld \n", length);
    return buffer;

}