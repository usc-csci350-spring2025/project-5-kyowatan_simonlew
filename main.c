#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 8
#define M (N / 4)
#define BUF_SIZE 4096

int getReducerIndex(const char *word)
{
    char c = word[0] | 0x20;
    return (c >= 'a' && c <= 'm') ? 0 : 1;
}

void checkPipe(int result)
{
    if (result == -1)
    {
        perror("pipe");
        exit(1);
    }
}

void checkWrite(ssize_t result)
{
    if (result == -1)
    {
        perror("write");
        exit(1);
    }
}

int main()
{
    int toMapper[N][2], fromMapper[N][2];
    int toReducer[M][2], fromReducer[M][2];
    pid_t mappers[N], reducers[M];

    for (int i = 0; i < N; i++)
    {
        checkPipe(pipe(toMapper[i]));
        checkPipe(pipe(fromMapper[i]));
    }
    for (int i = 0; i < M; i++)
    {
        checkPipe(pipe(toReducer[i]));
        checkPipe(pipe(fromReducer[i]));
    }

    for (int i = 0; i < N; i++)
    {
        if ((mappers[i] = fork()) == 0)
        {
            dup2(toMapper[i][0], 0);
            dup2(fromMapper[i][1], 1);
            for (int j = 0; j < N; j++)
            {
                close(toMapper[j][0]);
                close(toMapper[j][1]);
                close(fromMapper[j][0]);
                close(fromMapper[j][1]);
            }
            for (int j = 0; j < M; j++)
            {
                close(toReducer[j][0]);
                close(toReducer[j][1]);
                close(fromReducer[j][0]);
                close(fromReducer[j][1]);
            }
            execl("./mapper", "mapper", NULL);
            perror("exec mapper");
            exit(1);
        }
    }

    for (int i = 0; i < M; i++)
    {
        if ((reducers[i] = fork()) == 0)
        {
            dup2(toReducer[i][0], 0);
            dup2(fromReducer[i][1], 1);
            for (int j = 0; j < N; j++)
            {
                close(toMapper[j][0]);
                close(toMapper[j][1]);
                close(fromMapper[j][0]);
                close(fromMapper[j][1]);
            }
            for (int j = 0; j < M; j++)
            {
                close(toReducer[j][0]);
                close(toReducer[j][1]);
                close(fromReducer[j][0]);
                close(fromReducer[j][1]);
            }
            execl("./reducer", "reducer", NULL);
            perror("exec reducer");
            exit(1);
        }
    }

    for (int i = 0; i < N; i++)
        close(toMapper[i][0]);
    for (int i = 0; i < N; i++)
        close(fromMapper[i][1]);
    for (int i = 0; i < M; i++)
        close(toReducer[i][0]);
    for (int i = 0; i < M; i++)
        close(fromReducer[i][1]);

    char *chunks[N];
    for (int i = 0; i < N; i++)
    {
        chunks[i] = calloc(BUF_SIZE, 1);
        if (!chunks[i])
        {
            perror("calloc");
            exit(1);
        }
    }

    int turn = 0;
    char line[1024];
    while (fgets(line, sizeof(line), stdin))
    {
        strcat(chunks[turn], line);
        turn = (turn + 1) % N;
    }

    for (int i = 0; i < N; i++)
    {
        checkWrite(write(toMapper[i][1], chunks[i], strlen(chunks[i])));
        close(toMapper[i][1]);
        free(chunks[i]);
    }

    FILE *mapperOut[N];
    for (int i = 0; i < N; i++)
    {
        mapperOut[i] = fdopen(fromMapper[i][0], "r");
        if (!mapperOut[i])
        {
            perror("fdopen mapperOut");
            exit(1);
        }
    }

    FILE *reducerIn[M];
    for (int i = 0; i < M; i++)
    {
        reducerIn[i] = fdopen(toReducer[i][1], "w");
        if (!reducerIn[i])
        {
            perror("fdopen reducerIn");
            exit(1);
        }
    }

    char word[101];
    int count;
    for (int i = 0; i < N; i++)
    {
        while (fscanf(mapperOut[i], "%100s %d", word, &count) == 2)
        {
            int r = getReducerIndex(word);
            fprintf(reducerIn[r], "%s %d\n", word, count);
        }
        fclose(mapperOut[i]);
    }

    for (int i = 0; i < M; i++)
        fclose(reducerIn[i]);

    FILE *reducerOut[M];
    for (int i = 0; i < M; i++)
    {
        reducerOut[i] = fdopen(fromReducer[i][0], "r");
        if (!reducerOut[i])
        {
            perror("fdopen reducerOut");
            exit(1);
        }
    }

    char buf[256];
    for (int i = 0; i < M; i++)
    {
        while (fgets(buf, sizeof(buf), reducerOut[i]))
            fputs(buf, stdout);
        fclose(reducerOut[i]);
    }

    for (int i = 0; i < N; i++)
        waitpid(mappers[i], NULL, 0);
    for (int i = 0; i < M; i++)
        waitpid(reducers[i], NULL, 0);
    return 0;
}
