#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int error(char *str)
{
    int i;

    i = -1;
    while (str[++i])
        write(2, &str[i], 1);
    return 1;
}

int cd(char **argv, int i)
{
    if (i != 2)
        return error("error: cd: bad arguments\n");
    
    if (chdir(argv[1]))
        return error("error: cd: cannot change directory to "), error(argv[1]), error("\n");

    return 0;
}

int execute(char **argv, int i, char **env)
{
    int pid;
    int status;
    int pipe_exist;
    int fd[2];

    pipe_exist = argv[i] && !strcmp(argv[i], "|");
    if (pipe_exist && pipe(fd) == -1)
        return error("error: fatal\n");

    pid = fork();
    if (pid == 0)
    {
        argv[i] = 0;
        if (pipe_exist && (dup2(fd[1], 1) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
            return error("error: fatal\n");
        execve(argv[0], argv, env);
        return error("error: cannot execute "), error(argv[0]), error("\n");
    }

    waitpid(pid, &status, 0);
    if (pipe_exist && (dup2(fd[0], 0) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
        return error("error: fatal\n");
    return WIFEXITED(status) && WEXITSTATUS(status);
}

int main(int argc, char **argv, char **env)
{
    (void)argc;
    int i;
    int status;

    i = 0;
    status = 0;
    while (argv[i] && argv[++i])
    {
        argv += i;
        i = 0;

        while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
            i++;
        if (!strcmp(argv[0], "cd"))
            status = cd(argv, i);
        else if (i != 0)
            status = execute(argv, i, env);
    }
    return status;
}