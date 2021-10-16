#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "options.h"

#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <pwd.h>
#include <grp.h>


void FatalError(char c, const char* msg, int exit_status);
void PrintCopymasterOptions(struct CopymasterOptions* cpm_options);

void fastCopy(struct CopymasterOptions cpm_options, char flag);
void slowCopy(struct CopymasterOptions cmp_options);
void openInfile(int *file, struct CopymasterOptions cpm_options, char flag);
void openOutfile(int *file, struct CopymasterOptions cpm_options, char flag);
void createFile(struct CopymasterOptions cpm_options);
void overwriteFile(struct CopymasterOptions cpm_options);
void appendFile(struct CopymasterOptions cpm_options);
void lseekFile(struct CopymasterOptions cpm_options);
void deleteOptFile(struct CopymasterOptions cpm_options);
void chmodFile(struct CopymasterOptions cpm_options);
void truncateFile(struct CopymasterOptions cpm_options);
void inodeFile(struct CopymasterOptions cpm_options);
void linkFile(struct CopymasterOptions cpm_options);
void directoryFile(struct CopymasterOptions cpm_options);

int regularFile(const char *path);
bool checkOpen(int infile, int outfile);
int writeln(int file);

void checkOptions(int optionsAmount, int givenOptions[], int legalOptions[]);

int main(int argc, char* argv[])
{
    struct CopymasterOptions cpm_options = ParseCopymasterOptions(argc, argv);

    //-------------------------------------------------------------------
    // Kontrola hodnot prepinacovs

    //int options = 14;
    //int legalOptions[options];
    //int givenOptions[options];

    //checkOptions(options, givenOptions, legalOptions);
    //-------------------------------------------------------------------

    // Vypis hodnot prepinacov odstrante z finalnej verzie
    
    PrintCopymasterOptions(&cpm_options);

    // Bez akéhokoľvek prepínača 
    if(argc == 3){
        fastCopy(cpm_options, ' ');
    }
    // -f (--fast)
    if(cpm_options.fast){
        fastCopy(cpm_options, 'f');
    }
    // -s (--slow)
    if(cpm_options.slow){
        slowCopy(cpm_options);
    }
    // -c 777 (--create 777)
    if(cpm_options.create){
        createFile(cpm_options);
    }
    // -o (--overwrite)
    if(cpm_options.overwrite){
        overwriteFile(cpm_options);
    }
    // -a (--append)
    if(cpm_options.append){
        appendFile(cpm_options);
    }
    // -l (--lseek) (-l x,pos1,pos2,num (--lseek x,pos1,pos2,num))
    if(cpm_options.lseek){
        lseekFile(cpm_options);
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
    
    // -D (--directory)
    if (cpm_options.directory) {
        // TODO Implementovat vypis adresara
        directoryFile(cpm_options);
    }

    // -d (--delete)
    if(cpm_options.delete_opt){
        deleteOptFile(cpm_options);
    }
    // -m 0777 (--chmod 0777)
    if(cpm_options.chmod){
        chmodFile(cpm_options);
    }

    // -t size (--truncate size)
    if(cpm_options.truncate){
        truncateFile(cpm_options);
    }
    // -i number (--inode number)
    if(cpm_options.inode){
        inodeFile(cpm_options);
    }
    // -K (--link)
    if(cpm_options.link){
        linkFile(cpm_options);
    }
    // -u UTR,UTR,.... (--umask UTR,UTR,...)
    if(cpm_options.umask){
        //int infile = open(cpm_options.infile, O_RDONLY);
        //int outfile = open(cpm_options.outfile, O_WRONLY | O_CREAT);
        //mode_t newRights = umask(777);

        //umask(newRights);
    }

        
    //-------------------------------------------------------------------
    // Osetrenie prepinacov po kopirovani
    //-------------------------------------------------------------------
    
    // TODO Implementovat osetrenie prepinacov po kopirovani
    
    return 0;
}

void directoryFile(struct CopymasterOptions cpm_options){
    struct stat statDirectory;
    struct dirent *dirStruct;
    char info[10];
    char buffer[200];

    if(stat(cpm_options.infile, &statDirectory) < 0){
        // no such file o directory
        FatalError('D', "VSTUPNY SUBOR NIE JE ADRESAR", 28);
    }
    if(!S_ISDIR(statDirectory.st_mode)){
        FatalError('D', "VSTUPNY SUBOR NIE JE ADRESAR", 28);
    }

    DIR* directory = opendir(cpm_options.infile);

    int outfile = open(cpm_options.outfile, O_WRONLY | O_CREAT, 0777);
    if(outfile == -1){
        FatalError('D', "VYSTUPNY SUBOR - CHYBA", 28);
    }

    if(directory){
        //printf("Directory is opened, let's check it :D\n");

        struct stat statFiles;

        while((dirStruct = readdir(directory)) != NULL){
            if(strcmp(dirStruct->d_name, ".") && strcmp(dirStruct->d_name, "..")){
                char filePath[100];
                buffer[0] = 0;
                filePath[0] = 0;
                strcat(filePath, cpm_options.infile);
                strcat(filePath, "/");
                strcat(filePath, dirStruct->d_name);

                if(stat(filePath, &statFiles) < 0){
                    perror("stat()"); 
                }

                if(dirStruct->d_type == DT_DIR){
                    strcat(buffer, "d");
                } else{
                    strcat(buffer, "-");
                }

                // rules for owner 
                statFiles.st_mode & S_IRUSR ? strcat(buffer, "r") : strcat(buffer, "-");
                statFiles.st_mode & S_IWUSR ? strcat(buffer, "w") : strcat(buffer, "-");
                statFiles.st_mode & S_IXUSR ? strcat(buffer, "x") : strcat(buffer, "-");
                // rules for groupe
                statFiles.st_mode & S_IRGRP ? strcat(buffer, "r") : strcat(buffer, "-");
                statFiles.st_mode & S_IWGRP ? strcat(buffer, "w") : strcat(buffer, "-");
                statFiles.st_mode & S_IXGRP ? strcat(buffer, "x") : strcat(buffer, "-");
                // rules for others
                statFiles.st_mode & S_IROTH ? strcat(buffer, "r") : strcat(buffer, "-");
                statFiles.st_mode & S_IWOTH ? strcat(buffer, "w") : strcat(buffer, "-");
                statFiles.st_mode & S_IXOTH ? strcat(buffer, "x") : strcat(buffer, "-");

                // info of the links
                strcat(buffer, " ");
                sprintf(info, "%ld", (long)statFiles.st_nlink);
                strcat(buffer, info);
                info[0] = 0;
                strcat(buffer, " ");

                // info of the id owner + id group
                sprintf(info, "%d", (int)statFiles.st_uid);
                strcat(buffer, info);
                info[0] = 0;
                strcat(buffer, " ");
                sprintf(info, "%d", (int)statFiles.st_gid);
                strcat(buffer, info);
                info[0] = 0;
                strcat(buffer, " ");

                // info of the size files
                sprintf(info, "%d", (int)statFiles.st_size);
                strcat(buffer, info);
                info[0] = 0;
                strcat(buffer, "\t");

                // info of the date
                struct tm date = *(gmtime(&statFiles.st_mtime));

                sprintf(info, "%d", (int)date.tm_mday);
                strcat(buffer, info);
                info[0] = 0;
                strcat(buffer, "-");
                sprintf(info, "%d", (int)date.tm_mon + 1);
                strcat(buffer, info);
                info[0] = 0;
                strcat(buffer, "-");
                sprintf(info, "%d", (int)date.tm_year + 1900);
                strcat(buffer, info);
                info[0] = 0;
                strcat(buffer, " ");

                // info of the file's name
                strcat(buffer, dirStruct->d_name);


                printf("%s\n", buffer);
                strcat(buffer, "\0");
                int size;
                for(int i = 0; buffer[i] != '\0'; i++){
                    size = i;
                }
                size += 1;
                write(outfile, buffer, size);
                if((dirStruct) != NULL){
                    writeln(outfile);
                }

                buffer[0] = 0;
                filePath[0] = 0;
                size = 0;
            }
        }
        int sizeOutfile = lseek(outfile, 0L, SEEK_END);
        truncate(cpm_options.outfile, sizeOutfile - 2);
        close(outfile);
        closedir(directory);
    }
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
    if((*file = open(cpm_options.outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1){
        FatalError(flag, "outfile", 21);
    }
}


void createFile(struct CopymasterOptions cpm_options){
    char buffer[3];
    int nemberWord;
    int infile;
    int outfile;

    openInfile(&infile, cpm_options, 'c');

    umask(0);

    if(cpm_options.create_mode > 777 || cpm_options.create_mode < 1){
        FatalError('c', "ZLE PRAVA", 23);
    }

    if ((outfile = open(cpm_options.outfile, O_WRONLY | O_CREAT | O_EXCL, cpm_options.create_mode)) == -1){
        FatalError('c', "SUBOR EXISTUJE", 23);
    }

    while((nemberWord = read(infile, &buffer, 1)) > 0){
        if(checkOpen(infile, outfile)){
            FatalError('c', "INA CHYBA", 23);
        }
        write(outfile, &buffer, nemberWord);
    }

    close(infile);
    close(outfile);
}

void overwriteFile(struct CopymasterOptions cpm_options){
    char buffer[4];
    int numberWord;
    int infile;
    int outfile;

    openInfile(&infile, cpm_options, 'o');

    if((outfile = open(cpm_options.outfile, O_WRONLY | O_TRUNC)) == -1){
        FatalError('o', "SUBOR NEEXISTUJE", 24);
    }

    while((numberWord = read(infile, &buffer, 1)) > 0 ){
        write(outfile, &buffer, numberWord);
    }

    close(infile);
    close(outfile);
}

void appendFile(struct CopymasterOptions cpm_options){
    char buffer[4];
    int numberWord;
    int infile;
    int outfile;

    openInfile(&infile, cpm_options, 'a');

    if((outfile = open(cpm_options.outfile, O_WRONLY | O_APPEND)) == -1){
        FatalError('a', "SUBOR NEEXISTUJE", 22);
    }

    while((numberWord = read(infile, &buffer, 1)) > 0 ){
        write(outfile, &buffer, numberWord);
    }

    close(infile);
    close(outfile);
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
    int words;

    openInfile(&infile, cpm_options, 's');
    openOutfile(&outfile, cpm_options, 's');

    while((words = read(infile, &buffer, 1)) > 0){
        write(outfile, &buffer, words);
    }

    close(infile);
    close(outfile);
}

bool checkOpen(int infile, int outfile){
    return infile == -1 || outfile == -2;
}

void lseekFile(struct CopymasterOptions cpm_options){
    int infile;
    int outfile;
    char buffer[(int)cpm_options.lseek_options.num];
    int bytesOfRead;
    int bytesOfWrite;

    openInfile(&infile, cpm_options, 'l');

    if((outfile = open(cpm_options.outfile, O_WRONLY)) == -1){
        FatalError('l', "SUBOR NEEXISTUJE", 33);
    }
    if(lseek(infile, cpm_options.lseek_options.pos1, SEEK_SET) == -1){
        FatalError('l', "CHYBA POZICIE infile", 33);
    }
    if(lseek(outfile, cpm_options.lseek_options.pos2, cpm_options.lseek_options.x) == -1){
        FatalError('l', "CHYBA POZICIE outfile", 33);
    }
    if((bytesOfRead = read(infile, &buffer, cpm_options.lseek_options.num)) > 0){
        bytesOfWrite = write(outfile, &buffer, (size_t)bytesOfRead);

        if(bytesOfRead != bytesOfWrite){
            FatalError('l', "INA CHYBA", 33);
        }
    }
}

void deleteOptFile(struct CopymasterOptions cpm_options){
    fastCopy(cpm_options, 'd');

    if(regularFile(cpm_options.infile)) {
        unlink(cpm_options.infile);
    } else{
        FatalError('d', "SUBOR NEBOL ZMAZANY", 26);
    }
}

void chmodFile(struct CopymasterOptions cpm_options){
    int infile = open(cpm_options.infile, O_RDONLY);
    int outfile = open(cpm_options.outfile, O_WRONLY | O_CREAT, cpm_options.chmod_mode);

    if(cpm_options.chmod_mode > 777 || cpm_options.chmod_mode < 1){
        FatalError('m', "ZLE PRAVA", 34);
    } 
    if(checkOpen(infile, outfile)){
        FatalError('m', "INA CHYBA", 34);
    }

    int lengthBuffer = lseek(infile, 0, SEEK_END);
    char buffer[lengthBuffer];

    lseek(infile, 0, SEEK_SET);
    if((read(infile, &buffer, lengthBuffer) == -1) || (write(outfile, &buffer, lengthBuffer) == -1)){
        FatalError('m', "INA CHYBA", 34);
    }
    if(fchmod(outfile, cpm_options.chmod_mode) < 0){
        FatalError('m', "ZLE PRAVA", 34);
    }

    close(infile);
    close(outfile);
}

void truncateFile(struct CopymasterOptions cpm_options){
    fastCopy(cpm_options, 't');


    int truncateMyCode = truncate(cpm_options.infile, cpm_options.truncate_size);

    if (truncateMyCode == -1){
        FatalError('t', "ZAPORNA VELKOST", 31);
    }
}

void inodeFile(struct CopymasterOptions cpm_options){
    struct stat statFile;
    stat(cpm_options.infile, &statFile);

    int infile;
    int outfile;

    infile = open(cpm_options.infile, O_RDONLY);

    if(infile == -1){
        FatalError('i', "INA CHYBA", 27);
    }

    int lengthBuffer = lseek(infile, 0, SEEK_END);
    char buffer[lengthBuffer];

    if(cpm_options.inode_number != statFile.st_ino){
        FatalError('i', "ZLY INODE", 27);
    }
    if(!S_ISREG(statFile.st_mode)){
        FatalError('i', "ZLY TYP VSTUPNEHO SUBORU", 27);
    }

    outfile = open(cpm_options.outfile, O_WRONLY | O_CREAT, 0777);
    if(outfile == -1){
        FatalError('i', "INA CHYBA", 27);
    }

    if((read(infile, &buffer, lengthBuffer) == -1) || (write(outfile, &buffer, lengthBuffer) == -1)){
        FatalError('i', "INA CHYBA", 27);
    }

    close(infile);
    close(outfile);
}

void linkFile(struct CopymasterOptions cpm_options){
    int infile;
    if((infile = open(cpm_options.infile, O_RDONLY)) == -1){
        FatalError('K', "VSTUPNY SUBOR NEEXISTUJE", 30);
    }
    close(infile);

    int outfile;
    if((outfile = open(cpm_options.outfile, O_WRONLY)) != -1){
        FatalError('K', "VYSTUPNY SUBOR UZ EXISTUJE", 30);
    }
    close(outfile);

    if(link(cpm_options.infile, cpm_options.outfile) == 0){
        return;
    } else{
        FatalError('K', "INA CHYBA", 30);
    }
}

int writeln(int file){
    if (file == -1){
        return -1;
    } 
    char c[2];  
    c[0] = 13; 
    c[1] = 10;

    return write(file, c, 2);
}

//https://stackoverflow.com/questions/40163270/what-is-s-isreg-and-what-does-it-do
int regularFile(const char *path){
    struct stat stPath;
    stat(path, &stPath);

    //Linux manual (man stat)
    return S_ISREG(stPath.st_mode);
}

void checkOptions(int optionsAmount, int givenOptions[], int legalOptions[]){
    for (int i = 0; i < optionsAmount; i++){
        if((givenOptions[i] == 1) && (legalOptions[i] == 0)){
            fprintf(stderr, "CHYBA PREPINACOV\n"); 
            exit(EXIT_FAILURE);
        }
    }
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
