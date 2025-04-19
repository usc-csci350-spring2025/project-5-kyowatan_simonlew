#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>

#define MAX_UNIQUES 60000

typedef struct
{
    char *word;
    int count;
} count;

int cmpStr(const void *a, const void *b)
{
    return strcmp(((count *)a)->word, ((count *)b)->word);
}

int main()
{
    hcreate(MAX_UNIQUES);
    count *arr = calloc(MAX_UNIQUES, sizeof(count));
    int idx = 0;
    char word[101];
    int v;

    while (scanf("%100s %d", word, &v) == 2)
    {
        ENTRY e = {word, NULL};
        ENTRY *f = hsearch(e, FIND);
        if (f)
        {
            (*(int *)f->data) += v;
        }
        else
        {
            int *p = malloc(sizeof(int));
            *p = v;
            e.key = strdup(word);
            e.data = p;
            hsearch(e, ENTER);
            arr[idx++].word = e.key;
        }
    }

    for (int i = 0; i < idx; i++)
    {
        ENTRY e = {arr[i].word, NULL};
        ENTRY *f = hsearch(e, FIND);
        arr[i].count = *(int *)f->data;
    }

    qsort(arr, idx, sizeof(count), cmpStr);
    for (int i = 0; i < idx; i++)
        printf("%s %d\n", arr[i].word, arr[i].count);

    free(arr);
    return 0;
}
