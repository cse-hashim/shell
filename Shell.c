/*
 ============================================================================
 Name        : hashim_shell.c
 Author      : hashim
 Version     : v1.0
 Copyright   : use it but not publish with your name
 Description : shell in C, Ansi-style, you ma use -std=c99
 Compile     : gcc -Wall -c "%f"
 Build       : gcc -Wall -o "%e" "%f"
 Execute     : "./%e"
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <wait.h>
#include <assert.h>
#include <signal.h>
#include <stdarg.h>

//void sigquit_handler(int signo);
void show_h();//prints history array
void h_add(char* str);//add command to history array
void calc_path();//calculate the path of log file
void gedit_path();// return command for openning log file
void append(char *a);//append str to log
void el();//append \n to log
void appendi(int i);//append int to log
void appendp(pid_t);//append pid_t to log
void append2(char*);//append str then \n to log
void user_prompt();//prints user prompt
char* read_command();//read line entered by user and return it
//void strToLower(char *str);
void remove_tab_char(char* str);//replace any tab charachter with space char
int str_empty(char* str);//return true if string is empty or contains no command
void remove_pre_post_spaces(char* str);//remove starting and ending spaces from str
void remove__rn(char* str);//remove \r and \n from  str
void remove_comment(char* str);//remove comment from str
void removeExtraSpaces(char* str);//remove repeated spaces between command and prmtrs
//static void catch_function(int signo);
//static int sig();
void execute(char **argv, int is_background);//execute command in a child process
char** str_split(char* a_str, const char a_delim);//splits str to tockens
void ambersanded(char* str);//check if there is & char at the end of str
void remove_dq(char* str);//remove ' or " from str
int contains_dq(char* str);//chek if str contains ' or "
int contains_char(char* str, char c);//cheks if str contains c or not 
void sig_handler(int signo);//a signal handler
void sig_immetter(int sig, char* err_msg);//a signal immetter
void interactive_batch(int argc, char *argv[], int is_batch);//run the program in two modes interactive and batch
void store_hist();//store history array to history file
void intro();//a shell promo
void resore_hist();//read history from history file and assign it to history array


char comment[100];//stores comment
char* curr_path;//the current path of the process changed by cd
int END_OF_FILE = 0;//end of file flag
int AMBERSAND_FLAG = 0;//set to true if this is a background process
int ARR_MAX_SIZE = 100000;//max size of arr
int FALSE = 0, TRUE = 1; //bool
int logCounter = 1;
char log_path[512];
char hist_path[512];
FILE * f;//log file
int batch = 0;//is batch mode enabled
int cdf = 0;
char** historyA;//history array
int history_count = 0;
int old_h_count = 0;
FILE* historyF;//history file

/**
 * @warning SIGSEGV
 * @brief calc_path
 * it calculates the path of the log file by getting
 * the current user name then put log file in home
 * folder by _LOG_ name
 */
void
calc_path() {
    //strcpy(log_path, "/home/h/_LOG_");

    char str[512];
    int i;
    for (i = 0; i < 512; i++) {
        str[i] = '\0';
    }
    void* ptr;
    char chs[100];
    sprintf(chs, "%s", getenv("USER"));

    ptr = memccpy(str, "/home/", '\0', 512);
    ptr = memccpy(ptr - 1, chs, '\0', 512);
    memccpy(hist_path, str, '\0', 512);
    //    hist_path = strdup(str);
    memccpy(ptr - 1, "/_LOG_", '\0', 512);
    memcpy(log_path, str, 512);
    memccpy(ptr - 1, "/_HIST_", '\0', 512);
    memcpy(hist_path, str, 512);
    //        log_path[strlen(log_path)] = '\0';
    //    log_path = str;
}

/**
 * @brief gedit_path
 * @return command for auto openning log file after closing shell
 */
//char*

void
gedit_path() {
    //    char str[1024];
    //    strcpy(str, "gedit /home/");
    //    strcat(str, getlogin());
    //    strcat(str, "/_LOG_ &");
    char str[512];
    int i;
    for (i = 0; i < 512; i++) {
        str[i] = '\0';
    }
    void* ptr;
    //    char chs[100];
    //    sprintf(str, "vim -g ");
    ptr = memccpy(str, "vim -g ", '\0', 512);
    ptr = memccpy(ptr - 1, log_path, '\0', 512);
    memccpy(ptr - 1, " &", '\0', 512);
    system(str);
    //    return str;
    //    return "vim -g /home/h/_LOG_ &";
    //    return str;
}

/**
 * @brief append
 * append line to the log without new line
 * @param a
 */
void
append(char *a) {
    f = fopen(log_path, "a");
    fputs("-", f);
    fputs(a, f);
    fflush(f);
}

/**
 * @brief el
 * append end of line to log
 */
void
el() {
    f = fopen(log_path, "a");
    fputs("\n", f);
    fflush(f);
}

/**
 * @brief appendi
 * append int to log
 * @param i
 */
void appendi(int i) {
    f = fopen(log_path, "a");
    fprintf(f, "%d", i);
    fputs("\n", f);
    fflush(f);
}

/**
 * @brief appendp
 * append pid_t to the log
 * @param p
 */
void appendp(pid_t p) {
    f = fopen(log_path, "a");
    fprintf(f, "%d", p);
    fputs("\n", f);
    fflush(f);
}

/**
 * @brief append2
 * append p to the log and append also new line '\n'
 * @param p
 */
void append2(char* p) {
    f = fopen(log_path, "a");
    fprintf(f, "%s", p);
    //    fputc(0xA, f); //
    //    fputc(0xD, f); //
    fflush(f);
}

/**
 * @brief user_prompt
 * print user prompt to notify user to input his command
 */
void
user_prompt() {
    //    printf("Shell> ");
    printf("%c", 7);
    printf("%s@Shell:%s> ", getenv("USER"), curr_path/*getenv("PWD")*/);

}

/**
 * @brief read_command
 * @return the line entered by the user
 */
char*
read_command() {
    char command[513];
    int i = 0;
    char c;
    while ((c = getchar()) != '\n') {
        command[i++] = c;
    }
    command[i] = '\0';
    //        strcpy(str, command);
    return command; //
    //////////
    //    char com[512];
    //    scanf("%s", com);
    //    return com;



    //    if (command == EOF) {
    //        END_OF_FILE = 1;
    //    }
    //    return command;
}

//XXX: make sring without spaces in front and end

/**
 * @brief strToLower
 * @param str ip,op ->  changed to lower case
 */
void
strToLower(char *str) {

    //    int i = 0;
    //    while (str[i] != '\0') {
    //        if (isupper(str[i])) {
    //            str[i] = tolower(str[i]);
    //        }
    //        i++;
    //    }
}

void
intro() {
    printf("\e[0;33m######################################################################################################################################################\n");
    printf("##                                                                                                                                              [R] ##\n");
    printf("##  H                  H            A            SSSSSSSSSSSSSSSSSSSS  H                  H  IIIIIIIIIIIIIIIIIIIII  MMM                     MMM     ##\n");
    printf("##  H                  H           A A           S                     H                  H            I            M  M                   M  M     ##\n");
    printf("##  H                  H          A   A          S                     H                  H            I            M   M                 M   M     ##\n");
    printf("##  H                  H         A     A         S                     H                  H            I            M    M               M    M     ##\n");
    printf("##  H                  H        A       A        S                     H                  H            I            M     M             M     M     ##\n");
    printf("##  HHHHHHHHHHHHHHHHHHHH       AAAAAAAAAAA       SSSSSSSSSSSSSSSSSSSS  HHHHHHHHHHHHHHHHHHHH            I            M      M           M      M     ##\n");
    printf("##  H                  H      A           A                         S  H                  H            I            M       M         M       M     ##\n");
    printf("##  H                  H     A             A                        S  H                  H            I            M        M       M        M     ##\n");
    printf("##  H                  H    A               A                       S  H                  H            I            M         M     M         M     ##\n");
    printf("##  H                  H   A                 A                      S  H                  H            I            M          M   M          M     ##\n");
    printf("##  H                  H  A                   A  SSSSSSSSSSSSSSSSSSSS  H                  H  IIIIIIIIIIIIIIIIIIIII  M           MMM           M     ##\n");
    printf("##                                                                                                                                                  ##\n");
    printf("######################################################################################################################################################\n");
    printf("\e[45m                                                                                                                                                      \e[0m\n");
    printf("\e[45m                                                     \e[0;31mHI \e[1;36m%15s! \e[0;31mWELCOME TO \e[1;32mHASHIM \e[0;31mSHELL! ENJOY \e[0;36m:)\e[45m                                            \e[0m\n", getenv("USER")); //RED
    printf("\e[45m                                                                                                                                                      \e[0m\n\n");

}

void
remove_tab_char(char* str) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\t') {
            str[i] = ' ';
        }
        i++;
    }
}

/**
 * @brief str_empty
 * check if user input contains command or not
 * @param str
 * @return
 */
int
str_empty(char* str) {
    int i = 0;
    while (str[i] != '\0') {
        if (isalpha(str[i])) {
            return FALSE;
        }
        i++;
    }
    return TRUE;
}

/**
 * @brief remove_pre_post_spaces
 * remove starting and ending space to avoid errors
 * @param str
 *
 */
void
remove_pre_post_spaces(char* str) {
    if (str[0] == ' ') {
        memmove(str, &str[1], strlen(str) - 1);
        str[strlen(str) - 1] = '\0';
    }
    if (str[strlen(str) - 1] == ' ') {
        str[strlen(str) - 1] = '\0';
    }
}

void
remove__rn(char* str) {
    //    if (str[strlen(str) - 2] == '\r') {
    //        str[strlen(str) - 2] = '\0';
    //        //        str[strlen(str) - 2] = '\n';
    //    }
    //    if (str[strlen(str) - 1] == '\n') {
    //        str[strlen(str) - 1] = '\0';
    //        //        str[strlen(str) - 2] = '\n';
    //    }
    int i;
    for (i = strlen(str) - 1; i >= 0; i--) {
        if (str[i] == '\r' || str[i] == '\n') {
            str[i] = '\0';
        }

    }

    //    remove_comment(str);
}

void remove_comment(char* str) {

    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '#') {
            strcpy(comment, &str[i]);
            str[i] = '\0';
        }
        i++;
    }
}

/**
 * @brief removeExtraSpaces
 * removes duplicated spaces to ensure
 * that there is only one space between
 * each token "for adjusting split"
 * @param str
 */
void
removeExtraSpaces(char *str) {
    char tmp[strlen(str)];
    int i = 0;
    int j = 0;
    int count = 0;
    while (str[i] != '\0') {
        if (str[i] != ' ') {
            tmp[j] = str[i];
            count = 0;
            j++;
        } else {
            count++;
            if (count > 1) {
                count--;
            } else {
                tmp[j] = str[i];
                j++;
            }
        }
        i++;
    }
    tmp[j] = '\0';
    strcpy(str, tmp);
}

/**@debricated*/
static void catch_function(int signo) {
    //    puts("Interactive attention signal caught.");
    printf("i cathed a signal = %d", signo);
}

/**@debricated*/
static int sig() {
    // ///////////////
    if (signal(SIGCHLD, catch_function) == SIG_ERR) {
        fputs("An error occurred while setting a signal handler.\n", stderr);
        return EXIT_FAILURE;
    }
    puts("Raising the interactive attention signal.");
    if (raise(SIGCHLD) != 0) {
        fputs("Error raising the signal.\n", stderr);
        return EXIT_FAILURE;
    }
    puts("Exiting.");
    // ////////////////
}

/**
 * @brief execute
 * execute command and create child processes
 * firstly it fork() if successful call execvp() in the child process
 * if ‘&’ is not existed it calls wait(&status) to make parent wait for child else it won’t call
 * it.  It also call raise(17) to test if SIGCHLD happened to log Child process was terminated
 * @param argv
 * @param is_background
 */
void
execute(char **argv, int is_background) {
    sig_immetter(SIGCHLD, "cannot transmigt sigchld");
    sig_immetter(3, "cannot transmigt ctrl-d");
    //    if (signal(SIGINT, sigquit_handler) == SIG_ERR) perror("sig quit failed");
    //step 2:forking
    pid_t parent_pid = getpid();
    append("parent pid is:");
    appendp(parent_pid);
    pid_t pid;
    int status;

    if ((pid = fork()) < 0) { /* fork a child process           */
        el();
        append("*** ERROR: forking child process failed");
        printf("*** ERROR: forking child process failed\n");
        exit(1);
    } else if (pid == 0) { /* for the child process:         */
        el();
        append("child process created successfully and pid is ");
        appendp(getpid());
        //step 3: The child process passes the C strings—the command and parameter(s)—to execvp ().
        // puts("[response] ");//TODO

        if (execvp(*argv, argv) < 0) { /* execute the command  */
            //        chdir("/home/");

            //        if (execv("/bin/ls", (char**) {
            //                "ls", "-l", (void*) 0
            //            }) < 0) { /* execute the command  */

            el();
            append("execution failed");
            perror("*** ERROR: exec failed\n");


            exit(1);

        }


        printf("\n");
        el();
        append("execution success");
        //          el();append("******************Child process was terminated*******************");



    } else if (!is_background) { /* for the parent:      */
        //step 4: waiting

        el();
        append("{this is parent process}. not background. parent process will wait for child to exit");

        while (wait(&status) != pid) {
            //            sig_immetter(SIGCHLD, "cannot transmigt sigchld");

        } /* wait for completion  */


        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///        if (raise(17) == 0/*SIG_ERR*/) {                                                                          ///
        ///            el();                                                                                                 ///
        ///            append("******************Child process was terminated*******************");                          ///
        ///        }                                                                                                         ///
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //          printf("%d",signal(SIGCLD,catch_function));

    } else {
        el();
        append("i will not wait");

    }
    if (is_background) {
        el();
        append("reset background flag to 0");
        AMBERSAND_FLAG = 0;

    }

}

/**
 * @brief str_split
 * split string and return an array of strings
 * @param a_str
 * @param a_delim
 * @return
 */
char**
str_split(char* a_str, const char a_delim) {
    el();
    append("i will split user input on delemeter \"\\\\s\" {space}");
    char** result = 0;
    size_t count = 0;
    char* tmp = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof (char*) * count);

    if (result) {
        size_t idx = 0;
        char* token = strtok(a_str, delim);

        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

/**
 * @brief ambersanded
 * check if & exists for manual skip of wait "background process"
 * @param str
 */
void
ambersanded(char* str) {
    if (str[strlen(str) - 1] == '&') {
        AMBERSAND_FLAG = 1;
        str[strlen(str) - 1] = '\0';
    }
    remove_pre_post_spaces(str);
}

void
remove_dq(char* str) {//remove (") char
    int i = 0;
    while (str[i] != '\0') {

        if (str[i] == 0x27 || str[i] == '"') {
            str[i] = ' ';
        }
        i++;
    }
}

int
contains_dq(char* str) {//remove (") char
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == 0x27 || str[i] == '"') {
            return 1;
        }
        i++;
    }
    return 0;
}

int
contains_char(char* str, char c) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == c) {
            return 1;
        }
        i++;
    }
    return 0;
}

void sig_handler(int signo) {
    if (signo == SIGINT)
        printf("received SIGINT\n");
    if (signo == SIGCHLD) {
        //        printf("received SIGCHLD\n");
        el(); ///
        append("******************Child process was terminated*******************");
        el();
    }
    if (signo == SIGQUIT)
        printf("received SIGQUIT\n");
}

void sig_immetter(int sig, char* err_msg) {
    if (signal(sig, sig_handler) == SIG_ERR) perror(err_msg);

}

void interactive_batch(int argc, char *argv[], int is_batch) {

    //    if (signal(SIGQUIT, sigquit_handler) == SIG_ERR) perror("sig quit failed");
    //    FILE* f;
    FILE* batch_file;
    //    char* historyA[99999];
    char line[1000000];
    //    calc_path();
    f = fopen(log_path, "w");
    f = fopen(log_path, "a");
    el();
    append("log file created in path=");
    append2(log_path);
    el();
    append("shell started");
    if (!is_batch) {
        append(" in interactive mode");
    } else {
        append(" in batch mode and file name is ");
        append(argv[1]);
        batch_file = fopen(argv[1], "r");
        if (batch_file == NULL) perror("Error opening file");
    }
    //    append(" in interactive mode");//i
    while (1) {
        //        chroot(curr_path);
        //        if (signal(SIGQUIT, sigquit_handler) == SIG_ERR) perror("sig quit failed");

        if (!is_batch) {
            el();
            append("i will notify user to write command");
            user_prompt();
        }
        sig_immetter(3, "cannot transmigt ctrl-d");

        el();
        append("i will read command to variable {line}");
        if (!is_batch) {//read
            memcpy(line, strdup(read_command()), 512);
        } else {
            if (fgets(line, 512, batch_file) != NULL) {//b
                puts(line); //b
            } else {//b
                //perror("EOF");//b
                printf("EOF\n"); //b
                el(); //b
                append("isExit=true\nEXITTING :)\n"); //b
                gedit_path(); //b
                break; //b
            }//b
        }
        if (strlen(line) > 512) {
            perror("line length > 512");
            continue;
        }
        //        h_add(line); //add to history
hist:
        el();

        append("line entered is:");
        append2(line);
        //        if (contains_char(line, 4)) {
        //            break;
        //        }
        //        SIGQUIT;

        remove_tab_char(line);
        strToLower(line);
        el();

        append("line after tolower:");
        append2(line);
        removeExtraSpaces(line);
        el();
        append("line after removintg extra spaces:");
        append2(line);
        remove_pre_post_spaces(line);
        el();
        append("line after removing prefix and postfix spaces:");
        append2(line);
        ambersanded(line);
        el();
        append("line after testing background process=");
        append2(line);
        el();
        append("isBackground=");
        appendi(AMBERSAND_FLAG);
        if (strcmp(line, "exit") == 0) {
            el();
            append("isExit=true\nEXITTING :)\n");
            gedit_path();
            //            system(gedit_path());
            break;
        }
        el();
        append("isExit=false");
        remove__rn(line);
        remove_comment(line);

        if (str_empty(line) == TRUE) {
            el();
            append("isEmpty=true");
            continue;
        }
        el();
        append("isEmpty=false");
        el();
        //        remove__r(line);
        append("final input after shaving:");
        append2(line);

        // step 1: split
        char** tokens;
        char tmp_line[512];
        strcpy(tmp_line, line);
        tokens = str_split(tmp_line, ' ');
        if (strcmp(tokens[0], "history") != 0) {
            h_add(line);
        }
        if (strcmp(tokens[0], "cd") == 0) {
            cdf = 1;
            if (chdir(tokens[1]) != 0) {
                perror("not a dir");
            }
            getcwd(curr_path, 200);
            el();
            append("i will change directory");
            el();
        } else if (strcmp(tokens[0], "echo") == 0) {
            if (contains_dq(line)) {
                int i = 1;
                while (tokens[i] != NULL) {
                    if (contains_dq(tokens[i])) {
                        remove_dq(tokens[i]);
                        remove_pre_post_spaces(tokens[i]);
                    }
                    i++;
                }
                execute(tokens, AMBERSAND_FLAG);

            } else if (contains_char(line, '$')) {
                int i = 0;
                while (tokens[i] != NULL) {
                    if (contains_char(tokens[i], '$')) {
                        char tmp[100];
                        strcpy(tmp, tokens[i]);
                        memccpy(tmp, &tmp[1], '\0', 200);
                        //                        tmp[strlen(tokens[i]) - 1] = '\0';
                        char* env = getenv(tmp);
                        if (env != NULL) {
                            tokens[i] = env;
                        } else {
                            tokens[i][0] = '\0';
                        }
                    }
                    i++;
                }

                //            if (contains_dq(line)) {
                //                remove_dq(line);
                //            }
                execute(tokens, AMBERSAND_FLAG);

            } else {
                execute(tokens, AMBERSAND_FLAG);

            }
        } else if (contains_char(line, '=')) {
            char val[512];
            char** tokens2;
            char tmp_toks2[512];
            strcpy(tmp_toks2, line);
            tokens2 = str_split(tmp_toks2, '=');
            if (contains_dq(line)) {
                int i = 0;
                while (tokens2[i] != NULL) {
                    if (contains_dq(tokens2[i])) {
                        remove_dq(tokens2[i]);
                        remove_pre_post_spaces(tokens2[i]);
                    }
                    i++;
                }
                //                i = 1;
                //
                //                void* ptr;
                //                ptr = memccpy(val, tokens2[i++], '\0', 300);
                //                while (tokens2[i] != NULL) {
                //                    ptr = memccpy(ptr - 1, " ", '\0', 300);
                //                    ptr = memccpy(ptr - 1, tokens2[i], '\0', 300);
                //                    i++;
                //                }


                //                setenv(tokens2[0], val, 1);
                setenv(tokens2[0], tokens2[1], 1);

            } else if ((contains_char(line, ' ')&&!contains_dq(line))) {
                perror("not a valid command");
            } else {
                setenv(tokens2[0], tokens2[1], 1);
            }


        } else if (strcmp(tokens[0], "history") == 0) {
            if (tokens[1]) {
                memccpy(line, strdup(historyA[atoi(tokens[1])]), '\0', 512);
                goto hist; //bad line i know :)
            } else {
                show_h();
            }
        } else {

            el();
            append("i will execute command");
            el();
            execute(tokens, AMBERSAND_FLAG);
        }
        el();
        append("______________________________________________________________");
        el();

        // signal()
    }//step 5: repeating
    fclose(f);
}

void show_h() {
    int i = 0;
    while (historyA[i]) {
        printf("[%d]\t%", i);
        puts(historyA[i]);
        i++;
    }

}

void h_add(char* str) {


    historyA[history_count++] = strdup(str);
    //    memccpy(*(*historyA + history_count), str, '\l0', 512);
    //    historyA[history_count][strlen(historyA[history_count]) - 1] = '\0';
    //    history_count++;
    //    *(old_h_count + *historyA + history_count++) = str;
    //    historyA[][] + history_count)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              = str;

}

void store_hist() {
    historyF = fopen(hist_path, "w");
    int i = 0;

    while (historyA[i]) {//b
        fprintf(historyF, "%s\n", historyA[i]);
        if (!historyA[i][strlen(historyA[i]) - 1] == '\n') {
            fprintf(historyF, "\n");
        }
        //        fputs(historyA[i], historyF);
        i++;
    }
}

void resore_hist() {
    historyF = fopen(hist_path, "a+");
    int i = 0;
    char str[512] = {};
    while (fgets(str, 512, historyF) != NULL) {//b

        historyA[i] = strdup(str);
        remove__rn(historyA[i]);
        i++;
    }
    history_count = i;
}

/**
 * @brief main
 * @return
 */
int
main(int argc, char *argv[]) {
    calc_path();
    printf("%c", 7);
    //    historyF = fopen(hist_path, "a+");
    historyA = malloc(sizeof (char*) * 100000);
    resore_hist();
    //    hptr = historyA;
    //    if (signal(SIGQUIT, sigquit_handler) == SIG_ERR) perror("sig quit failed");
    curr_path = getenv("PWD");
    //    char* arr[] = {"/home/h/NetBeansProjects/c1/dist/Debug/GNU-Linux-x86/c1", "/home/h/currdeb/testLab1.sh"};
    //    batch_m(2, arr);
    if (argc == 1) {
        //        access()itnt void [] Srtring a[args ]]
        intro();
        interactive_batch(argc, argv, 0); //interactive
    } else if (argc == 2) {
        interactive_batch(argc, argv, 1); //batch
    }
    //    printf("exited");
    store_hist();
    return 0;
}

