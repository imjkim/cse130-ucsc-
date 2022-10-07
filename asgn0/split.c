#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <err.h>

/*

char** split(char* file, const char delim){
    //check if it is binary or ascii file first
    int fd = open(file, O_RDONLY);
    if(fd == -1){
        pringf("Unable to open file");
    }
    char c;
    read(fd, &c, 1);
}
*/

int main(int argc, char* argv[]){
    int error = 0;
    if(argc < 3){
        warnx("%s\nusage: %s%s\n", "not enough arguments", argv[0], " <files> <delimiter>");
        error = 1;
    } 
    else {
        char* delim = argv[argc-1];
        if(strlen(delim) != 1){
            warnx("%s\nusage: %s%s\n", "only single-character delimiters allowed", argv[0]," <files> <delimiter>");
            error = 1;
        }
        char value = delim[0];
        int mode = 0;
        // argc for split f1 ab should output 3
        for(int i = 1; i < (argc - 1); i++){
            char* tmp = argv[i];
            char tmpval = tmp[0];
            if((tmpval == 45) &&(strlen(tmp) == 1)){
                mode = 1;
            } 
            else if(!((mode == 1)&& (i == argc-2))&&(access(argv[i], F_OK) == 0)){          // confrim file is valid
                int j = 0;
                char ch = 0;
                int fd = open(argv[i], O_RDONLY);
                int countbuff=0;
                char *buff = (char*)malloc(sizeof(char)*2048);
                int count = read(fd, buff, 2048);
                // printf("\n value is %c and asci is %d", value, value);
                while(count !=0){
                    for (int i =0; i < count; i++){
                        ch = buff[i];
                        if((ch >= 0) && (ch< 128)){
                        //printf("\n\n ch is %c and asci is %d", ch, ch);
                            if(ch == value){
                                buff[i] = '\n';
                            // write(STDOUT_FILENO, "\n",1);
                            //printf("\n");
                            } 
                        }
                    }
                    write(STDOUT_FILENO, buff,count);
                    count = read(fd, buff, 2048); 
                }
                free(buff);
            } else{
                warnx("%s: %s\n",argv[i], "No such file or directory");
                error =1;
            }
        }
        if(mode == 1){
            
            for(;;){
                char *buf = (char*)malloc(sizeof(char)*4096);
                int ch1 = read(STDIN_FILENO, buf, 4096);
                int len = ch1;
                if(ch1 == 0){
                    len = 4096;
                }
                for(int i = 0; i < len; i++){
                    char ch = buf[i];
                     if((ch >= 0) && (ch< 128)){
                        //printf("\n\n ch is %c and asci is %d", ch, ch);
                        if(ch == value){
                            printf("\n");
                        } 
                        else {
                            printf("%c", ch);
                        }
                    } 
                    else {
                        printf("%d", ch);
                    }
                }
                free(buf);
            }
            printf("\n");
        }
    }
    return error;
}
