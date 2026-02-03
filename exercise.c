#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*

Under the same folder, file input.txt.
Each line contains a positive integer number and we guarantee the number is smaller than 1000.
You need to read the number in each line, add it by one, and write them line by line to another file called output.txt

NOTE: Please don't hardcode the results in your program because we change the content in input.txt when grading.

TIP: You can use sscanf to convert char array to int and sprintf to convert int to char array. 

*/
int main(){

    int fd;
    fd = open("input.txt", O_RDONLY);
    if(fd<0){
        perror("error");
        return 1;
    }

    int num[100] = {0}; int i = 0;
    char read_buf[100];
    ssize_t bytes_read; //check whether the file ends indicator
    while (bytes_read = read(fd, read_buf, 1) > 0) {

        //printf("str%s", read_buf);
        char ctemp = 0;
        sscanf(read_buf, "%c", &ctemp);

        if(ctemp != '\n'){
            
            //printf("nigga");
            num[i] = num[i]*10;
            num[i] = num[i]+ ctemp - 48;
        }else{
            num[i]++;
            //printf("integer %d get nigger\n", num[i]);
            i++;
        }
    }

    close(fd);//input+calculation finish

    for(int j = 0; j<i; j++){ //checking
        //printf("int%d\n", num[j]);
    }
    
    fd = open("output.txt", O_RDWR|O_CREAT);
    if(fd<0){
        perror("error");
        return 1;
    }

    char stemp[5];
    for(int j = 0; j<i; j++){ 
        sprintf(stemp, "%d\n", num[j]);
        write(fd, stemp, strlen(stemp));
    }
    close(fd);
    return 0;
}       