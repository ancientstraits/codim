#include <stdio.h>

void swap(char* a, char *b) {
    char temp = *a;
    *a = *b;
    *b = temp;
}

void recurse(char* s, int start) {
    if (s[start] == 0) {
        puts(s);
        return;
    }

    for (int i = 0; s[i]; i++) {
        swap(&s[start], &s[i]);
        recurse(s, start + 1);
        swap(&s[start], &s[i]);
    }
}

int main() {
    char s[] = "supercalifragilisticexpialidocious";
    recurse(s, 0);
}
