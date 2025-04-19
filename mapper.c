#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main()
{
    char word[101];
    while (scanf("%100s", word) == 1)
    {
        int i = 0, j = 0;
        while (word[i])
        {
            if (isalnum(word[i]))
                word[j++] = tolower(word[i]);
            i++;
        }
        word[j] = '\0';
        if (j > 0)
            printf("%s 1\n", word);
    }
    return 0;
}
