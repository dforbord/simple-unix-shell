/**
 * @file myshell.c
 * @author ** Dylan Forbord **
 * @date 2024-06-10
 * @brief Acts as a simple command line interpreter.  It reads commands from
 *        standard input entered from the terminal and executes them. The
 *        shell does not include any provisions for control structures,
 *        redirection, background processes, environmental variables, pipes,
 *        or other advanced properties of a modern shell. All commands are
 *        implemented internally and do not rely on external system programs.
 *
 */

#include <pwd.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_SIZE          256
#define MAX_PATH_LENGTH      256
#define MAX_FILENAME_LENGTH  256

static char buffer[BUFFER_SIZE] = {0};
static char filename[MAX_FILENAME_LENGTH] = {0};

// Function prototypes for shell commands
int do_cat(const char* filename);
int do_cd(char* dirname);
int do_ls(const char* dirname);
int do_mkdir(const char* dirname);
int do_pwd(void);
int do_rm(const char* filename);
int do_rmdir(const char* dirname);
int do_stat(char* filename);
int execute_command(char* buffer);

/**
 * Removes whitespace at the end of a string
 */
void strip_trailing_whitespace(char* string) {
  int i = strnlen(string, BUFFER_SIZE) - 1;

  while (isspace(string[i]))
    string[i--] = 0;
}


/**
 * Displays the prompt with current working directory
 */
void display_prompt(void) {
    char current_dir[MAX_PATH_LENGTH];

    if (getcwd(current_dir, sizeof(current_dir)) != NULL)
        fprintf(stdout, "myshell:\033[32;1m%s\033[0m> ", current_dir);
}

/**
 * Main shell loop
 */
int main(int argc, char** argv) {
    while (true) {
        display_prompt();

        if (fgets(buffer, BUFFER_SIZE, stdin) != 0) {

            strip_trailing_whitespace(buffer);

            bzero(filename, MAX_FILENAME_LENGTH);

            if ((sscanf(buffer, "cd %s", filename) == 1) ||
                (!strncmp(buffer, "cd", BUFFER_SIZE)))
                do_cd(filename);

            else if (!strncmp(buffer, "exit", BUFFER_SIZE))
                exit(EXIT_SUCCESS);

            else
                execute_command(buffer);
        }
    }

    return EXIT_SUCCESS;
}

/**
 * Implements cd command
 */
int do_cd(char* dirname) {
    struct passwd* p = getpwuid(getuid());

    if (strnlen(dirname, MAX_PATH_LENGTH) == 0)
        strncpy(dirname, p->pw_dir, MAX_PATH_LENGTH);

    if (chdir(dirname) < 0) {
        fprintf(stderr, "cd: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * Lists directory contents
 */
int do_ls(const char* dirname) {
    DIR* dir = opendir(dirname);
    struct dirent* d;

    if (dir == NULL) {
        fprintf(stderr, "ls: %s\n", strerror(errno));
        return -1;
    }

    while ((d = readdir(dir)) != NULL) {
        printf("%s\n", d->d_name);
    }

    closedir(dir);
    return 0;
}

/**
 * Outputs contents of a file
 */
int do_cat(const char* filename) {
    int fd = open(filename, O_RDONLY);
    char buf[256];
    ssize_t bytes;

    if (fd < 0) {
        fprintf(stderr, "cat: %s\n", strerror(errno));
        return -1;
    }

    while ((bytes = read(fd, buf, sizeof(buf))) > 0) {
        write(1, buf, bytes);
    }

    if (bytes < 0) {
        fprintf(stderr, "cat: %s\n", strerror(errno));
    }

    close(fd);
    return 0;
}

/**
 * Creates new directory
 */
int do_mkdir(const char* dirname) {
    if (mkdir(dirname, 0755) < 0) {
        fprintf(stderr, "mkdir: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * Removes directory
 */
int do_rmdir(const char* dirname) {
    if (rmdir(dirname) < 0) {
        fprintf(stderr, "rmdir: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * Prints working directory
 */
int do_pwd(void) {
    char current_dir[MAX_PATH_LENGTH];

    if (getcwd(current_dir, sizeof(current_dir)) != NULL)
        printf("%s\n", current_dir);

    return 1;
}

/**
 * Removes a file
 */
int do_rm(const char* filename) {
    if (unlink(filename) < 0) {
        fprintf(stderr, "rm: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * Outputs file information similar to stat command
 */
int do_stat(char* filename) {
    struct stat st;

    if (stat(filename, &st) < 0) {
        fprintf(stderr, "stat: %s\n", strerror(errno));
        return -1;
    }

    printf("File: %s\n", filename);
    printf("Size: %u bytes\n", (unsigned) st.st_size);
    printf("Links: %u\n", (unsigned) st.st_nlink);
    printf("Inode: %u\n", (unsigned) st.st_ino);

    return 0;
}

/**
 * Executes a command based on buffer
 */
int execute_command(char* buffer) {
    if (sscanf(buffer, "cat %s", filename) == 1)
        return do_cat(filename);

    if (sscanf(buffer, "stat %s", filename) == 1)
        return do_stat(filename);

    if (sscanf(buffer, "mkdir %s", filename) == 1)
        return do_mkdir(filename);

    if (sscanf(buffer, "rmdir %s", filename) == 1)
        return do_rmdir(filename);

    if (sscanf(buffer, "rm %s", filename) == 1)
        return do_rm(filename);

    if ((sscanf(buffer, "ls %s", filename) == 1) ||
        (!strncmp(buffer, "ls", BUFFER_SIZE))) {

        if (strnlen(filename, BUFFER_SIZE) == 0)
            sprintf(filename, ".");

        return do_ls(filename);
    }

    if (!strncmp(buffer, "pwd", BUFFER_SIZE))
        return do_pwd();

    if (strnlen(buffer, BUFFER_SIZE) != 0)
        fprintf(stderr, "myshell: %s: No such file or directory\n", buffer);

    return -1;
}
