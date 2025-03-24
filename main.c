#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define LSH_RL_BUFSIZE 1024

char *lsh_read_line(void) {
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error \n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null chartacter and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

int lsh_launch(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      // waitpid is a system call that suspends the parent process until the
      // specified child process (identified by pid) changes state.
      // The status variable is used to store information about how the child
      // process terminated. WUNTRACED is a flag that tells waitpid() to also
      // return if the child process is stoppped (but not terminated). The
      // function returns the process ID of the chiled whose state changed, or
      // -1 on error.

      wpid = waitpid(pid, &status, WUNTRACED);
    } while (
        !WIFEXITED(status) &&
        !WIFSIGNALED(
            status)); // Wait for the child process to either exit normally or
                      // be killed by a signal. Don't continue with the shell
                      // prompt until the commmand has finished executing.
  }
  return 1;
}

char **lsh_split_line(char *line) {
  int count = 0;
  int size = 10;
  char *token;
  char *delims = " ";
  char *line_copy = strdup(line);

  char **multiline_string = malloc(size * sizeof(char *));
  if (multiline_string == NULL) {
    perror("Memory allocation");
    exit(EXIT_FAILURE);
  }

  token = strtok(line_copy, delims);

  while (token != NULL) {
    if (count >= size) {
      size *= 2;
      char **temp = realloc(multiline_string, size * sizeof(char *));
      if (temp == NULL) {
        perror("Memory allocation");
        free(multiline_string);
        exit(EXIT_FAILURE);
      }
      multiline_string = temp;
    }

    multiline_string[count] = malloc(strlen(token) + 1);
    if (multiline_string[count] == NULL) {
      perror("Memory allocation");
      free(multiline_string);
      exit(EXIT_FAILURE);
    }

    strcpy(multiline_string[count], token);
    count++;
    token = strtok(NULL, delims);
  }
  return multiline_string;
}

void lsh_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char *argv[]) {
  // Load configuration files if any.

  // Run command loop.
  lsh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
