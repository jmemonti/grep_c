//
// Created by ice on 10/11/23.
//

#include <stdio.h>
#include <sys/stat.h>//information about the archive
#include <stdlib.h>
#include <dirent.h>
#include <string.h> //string compare
#include <ctype.h> // upper lower for string
#include <unistd.h>//obtain actual path
//


#define LINE_MAX 500
#define WORD_MAX 101 //+1 for the /0
#define ARCHIVE_MAX 500

void help(){
		printf("Modo de empleo: migrep [OPCIÓN... PATRONES [FICHEROS]...]");
}

char* upper_word(char *word){
        //vars
        int longitud=strlen(word);
        
        for (int j=0; j<longitud; j++) {
                word[j]= toupper(word[j]);
        }
        return word;
}

char* lower_word(char *word){
        //vars
        int longitud=strlen(word);

        for (int j=0; j<longitud; j++) {
                word[j]= tolower(word[j]);
        }
        return word;
}



void read_file(char *pattern, const char *fich, int i, int w, int r){
        FILE * file;
        char word_line[WORD_MAX];
        char line[LINE_MAX];
        int longitud;
        int word_index;


        file=fopen(fich, "rb");//rb -> to read regular and binary files

        if (!file) {
                fprintf(stderr, "Error to open file");
                exit(EXIT_FAILURE);
        }
       

        //read lines
        while (fgets(line, sizeof(line), file) ) {
                        
                //read word line
                longitud=strlen(line);
                word_index=0;


                for (int j=0; j<longitud;j++) {
                        if (line[j] == ' ' || line[j] == '\0') {
                                word_line[word_index]='\0';
                                if (word_index > 0) {
                                                
                                        if ( w  && !i) {

                                                if ( strcmp(pattern, word_line) ==0 ) { 
                                                        if (r) {        
                                                                printf("%s: %s", fich, line);    
                                                        }else {
                                                                printf("%s", line);
                                                        }
                                                        break;
                                                }
                                        }else if ( w && i ) {
                                                        
                                                if (strcmp(upper_word(pattern), word_line) == 0 || strcmp(lower_word(pattern), word_line) ==0 ) {
                                                        if (r) {
                                                                printf("%s: %s", fich, line);
                                                        }else {
                                                                printf("%s", line);
                                                        }
                                                        break;
                                                }
                                                                
                                        }
                                        word_index=0;
                                }
                        }else{
                                word_line[word_index] = line[j];
                                word_index++;
                        }
                }

                if ( !w && !i ) {

                        //search pattern in line with strstr 
                        if ( strstr(line, pattern) != NULL ) {
                                if (r) {
                                        printf("%s: %s", fich, line);
                                }else {
                                        printf("%s", line);
                                }
                        }
                }


                if (!w && i) { 
                        if (strstr(line, upper_word(pattern)) != NULL || strstr(line, lower_word(pattern)) !=0 ) {
                                if (r) {
                                        printf("%s: %s", fich, line);
                                }else {
                                        printf("%s", line);
                                }
                        }
                }
        }

        fclose(file);
}
void recursive(char *pattern, const char *fich, int i, int w, int r){
        DIR *d;
        struct dirent *node; //navigate directory and read directory name
        struct stat st;
        char path[ARCHIVE_MAX];
        

        
        if ( (d=opendir(fich)) == NULL ) {
                fprintf(stderr, "%s", "Error to open directory");
                exit(EXIT_FAILURE);
        }
        
        //recursive
        while ( (node=readdir(d)) != NULL ) {
                //obtain the path fusion the parent directory and child directory
                snprintf(path, sizeof(path), "%s/%s", fich, node->d_name);

                //analize the new path
                if ( stat(path, &st) != 0 ) {
                        fprintf(stderr, "%s", "Error to obtain information of the arhcive");
                        exit(EXIT_FAILURE);
                }

                //if is a directory repeat the process
                if (S_ISDIR(st.st_mode)) {
                        //ignore actual and parent directory or hidden directory
                        if ( strcmp(node->d_name, ".") !=0 && strcmp(node->d_name, "..") !=0 && node->d_name[0] != '.' ) {
                                recursive(pattern, path, i, w, r);
                        }
                }else if(S_ISREG(st.st_mode)){
                        read_file(pattern, path, i, w, r); 
                }
        }
        closedir(d);
}

void read_fich(char *pattern, const char *fich, int i, int w, int r){
       //vars
        struct stat st;

        //obtain information of the archive by save in stat variable
        if ( stat(fich, &st) != 0 ) {
                fprintf(stderr, "Error not exist file or directory");
                exit(EXIT_FAILURE);
        }

        //is a file 
        if ( S_ISREG(st.st_mode) ) {
                read_file(pattern, fich, i, w, r);
                exit(0);

        } else if( S_ISDIR(st.st_mode)){//is a directory 
                if ( r ) {
                        //recursive
                        recursive(pattern, fich, i, w, r);
                }else{
                        printf("%s is a directory", fich);
                        exit(0);    
                }
                
        }
}


int main (int argc, char*argv[]){
        //vars
        int nop, i, w, r;
        char *word[WORD_MAX], *fich[ARCHIVE_MAX], actual_path[ARCHIVE_MAX];
        
        nop=i=w=r=0;
        
        //recorrer parámetros 
        for (int j=1; j<argc; j++) {
                        
                if ( argv[j][0]  == '-') {
                        switch ( argv[j][1] )  {
                                case 'r':
                                        r=1;
                                        break;
                                case 'i':
                                        i=1;
                                        break;
                                case 'w':
                                        w=1;
                                        break;
                                default:
                                        help();
                                        exit(0);
                                        break;
                        }
                        nop++;
                }
        }
        
        //basic rule for sintax
        if ( argc-1<=0 || argc-1 == nop || argc-1 == 1) {
                help();
                return 0;
        }

        *word=argv[nop+1];// ./nombre_programa don´t important because there is in position 0
        
        //options and pattern only and recursive
        if (argc-1 == nop+1 && r) {
                //obtain actual directory
                if( getcwd(actual_path, sizeof(actual_path)) != NULL){
                        read_fich(*word, actual_path, i, w, r);
                }
        }else{        
                //ficheros
                for (int j=nop+2; j< argc; j++) {
                        *fich=argv[j];
                        read_fich(*word, *fich, i, w, r);
                }
        }
        return 0;
}
