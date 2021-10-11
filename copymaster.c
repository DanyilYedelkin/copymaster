#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "options.h"

#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>


void FatalError(char c, const char* msg, int exit_status);
void PrintCopymasterOptions(struct CopymasterOptions* cpm_options);

void fastCopy(struct CopymasterOptions cpm_options, char flag);
void slowCopy(struct CopymasterOptions cmp_options);
void openInfile(int *file, struct CopymasterOptions cpm_options, char flag);
void openOutfile(int *file, struct CopymasterOptions cpm_options, char flag);


int main(int argc, char* argv[])
{
    struct CopymasterOptions cpm_options = ParseCopymasterOptions(argc, argv);

    //-------------------------------------------------------------------
    // Kontrola hodnot prepinacov
    //-------------------------------------------------------------------

    // Vypis hodnot prepinacov odstrante z finalnej verzie
    
    PrintCopymasterOptions(&cpm_options);

    if(argc == 3){
        fastCopy(cpm_options, ' ');
    }
    if(cpm_options.fast){
        fastCopy(cpm_options, 'f');
    }
    if(cpm_options.slow){
        slowCopy(cpm_options);
    }
    
    //-------------------------------------------------------------------
    // Osetrenie prepinacov pred kopirovanim
    //-------------------------------------------------------------------
    
    if (cpm_options.fast && cpm_options.slow) {
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }

    
    // TODO Nezabudnut dalsie kontroly kombinacii prepinacov ...
    
    //-------------------------------------------------------------------
    // Kopirovanie suborov
    //-------------------------------------------------------------------
    
    // TODO Implementovat kopirovanie suborov
    
    // cpm_options.infile
    // cpm_options.outfile
    
    //-------------------------------------------------------------------
    // Vypis adresara
    //-------------------------------------------------------------------
    
    if (cpm_options.directory) {
        // TODO Implementovat vypis adresara
    }


        
    //-------------------------------------------------------------------
    // Osetrenie prepinacov po kopirovani
    //-------------------------------------------------------------------
    
    // TODO Implementovat osetrenie prepinacov po kopirovani
    
    return 0;
}

void openInfile(int *file, struct CopymasterOptions cpm_options, char flag){
    /**file = open(cpm_options.infile, O_RDONLY) == -1;

    if(*file == -1){
        FatalError(flag, "infile", 21);
    }*/
    if((*file = open(cpm_options.infile, O_RDONLY)) == -1){
        FatalError(flag, "infile", 21);
    }
}

void openOutfile(int *file, struct CopymasterOptions cpm_options, char flag){
    /**file = open(cpm_options.outfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);

    if(*file == -1){
        FatalError(flag, "infile", 21);
    }*/
    if((*file = open(cpm_options.outfile, O_WRONLY | O_CREAT | O_TRUNC, 0777)) == -1){
        FatalError(flag, "outfile", 21);
    }
}

void fastCopy(struct CopymasterOptions cpm_options, char flag){
    long lengthBuffer;
    int infile;
    int outfile;
    int words;

    openInfile(&infile, cpm_options, flag);
    openOutfile(&outfile, cpm_options, flag);
    
    lengthBuffer = lseek(infile, 0, SEEK_END);
    char buffer[lengthBuffer];

    lseek(infile, 0, SEEK_SET);
    if((words = read(infile, &buffer, lengthBuffer)) > 0){
        write(outfile, &buffer, words);
    }

    close(infile);
    close(outfile);
}
void slowCopy(struct CopymasterOptions cpm_options){
    char buffer[2];
    int infile;
    int outfile;
    int temp;

    openInfile(&infile, cpm_options, 's');
    openOutfile(&outfile, cpm_options, 's');

    while((temp = read(infile, &buffer, 1)) > 0){
        write(outfile, &buffer, temp);
    }

    close(infile);
    close(outfile);
}


void FatalError(char c, const char* msg, int exit_status)
{
    fprintf(stderr, "%c:%d\n", c, errno); 
    fprintf(stderr, "%c:%s\n", c, strerror(errno));
    fprintf(stderr, "%c:%s\n", c, msg);
    exit(exit_status);
}

void PrintCopymasterOptions(struct CopymasterOptions* cpm_options)
{
    if (cpm_options == 0)
        return;
    
    printf("infile:        %s\n", cpm_options->infile);
    printf("outfile:       %s\n", cpm_options->outfile);
    
    printf("fast:          %d\n", cpm_options->fast);
    printf("slow:          %d\n", cpm_options->slow);
    printf("create:        %d\n", cpm_options->create);
    printf("create_mode:   %o\n", (unsigned int)cpm_options->create_mode);
    printf("overwrite:     %d\n", cpm_options->overwrite);
    printf("append:        %d\n", cpm_options->append);
    printf("lseek:         %d\n", cpm_options->lseek);
    
    printf("lseek_options.x:    %d\n", cpm_options->lseek_options.x);
    printf("lseek_options.pos1: %ld\n", cpm_options->lseek_options.pos1);
    printf("lseek_options.pos2: %ld\n", cpm_options->lseek_options.pos2);
    printf("lseek_options.num:  %lu\n", cpm_options->lseek_options.num);
    
    printf("directory:     %d\n", cpm_options->directory);
    printf("delete_opt:    %d\n", cpm_options->delete_opt);
    printf("chmod:         %d\n", cpm_options->chmod);
    printf("chmod_mode:    %o\n", (unsigned int)cpm_options->chmod_mode);
    printf("inode:         %d\n", cpm_options->inode);
    printf("inode_number:  %lu\n", cpm_options->inode_number);
    
    printf("umask:\t%d\n", cpm_options->umask);
    for(unsigned int i=0; i<kUMASK_OPTIONS_MAX_SZ; ++i) {
        if (cpm_options->umask_options[i][0] == 0) {
            // dosli sme na koniec zoznamu nastaveni umask
            break;
        }
        printf("umask_options[%u]: %s\n", i, cpm_options->umask_options[i]);
    }
    
    printf("link:          %d\n", cpm_options->link);
    printf("truncate:      %d\n", cpm_options->truncate);
    printf("truncate_size: %ld\n", cpm_options->truncate_size);
    printf("sparse:        %d\n", cpm_options->sparse);
}
