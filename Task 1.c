#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEX 500
#define MAX_STR 128

typedef enum {
    KEYWORD, IDENTIFIER, OPERATOR, LITERAL, DELIMITER
} Cat;

typedef struct {
    char val[MAX_STR];
    Cat cat;
} Lexeme;

Lexeme lexList[MAX_LEX];
int lexCount = 0;

char *keywords[] = {"int","float","char","return","for", NULL};

int isKeyword(char *s){
    for(int i=0; keywords[i];i++){
        if(strcmp(s, keywords[i])==0) return 1;
    }
    return 0;
}

void addLex(char *buf, Cat c){
    if(lexCount >= MAX_LEX) return;
    strncpy(lexList[lexCount].val, buf, MAX_STR-1);
    lexList[lexCount].cat = c;
    lexCount++;
}

void tokenize(char *src){
    lexCount = 0;
    int i=0;
    int len = strlen(src);
    while(i < len){
        char ch = src[i];
        if(isspace(ch)){ i++; continue; }


        if( (ch=='<'&&src[i+1]=='=') || (ch=='>'&&src[i+1]=='=') ||
            (ch=='+'&&src[i+1]=='+') || (ch=='-'&&src[i+1]=='-') ){
            char op[3]={0};
            op[0]=ch; op[1]=src[i+1];
            addLex(op, OPERATOR);
            i+=2;
            continue;
        }

        if(strchr("+-*/=();{},", ch)){
            char op[2]={0}; op[0]=ch;
            if(strchr("();{},",ch)) addLex(op, DELIMITER);
            else addLex(op, OPERATOR);
            i++;
            continue;
        }

        if(isdigit(ch)){
            char num[MAX_STR]={0};
            int p=0;
            while(i<len && (isdigit(src[i])||src[i]=='.')){
                num[p++] = src[i++];
            }
            addLex(num, LITERAL);
            continue;
        }

        if(isalpha(ch) || ch=='_'){
            char id[MAX_STR]={0};
            int p=0;
            while(i<len && (isalnum(src[i])||src[i]=='_')){
                id[p++] = src[i++];
            }
            if(isKeyword(id)) addLex(id, KEYWORD);
            else addLex(id, IDENTIFIER);
            continue;
        }
        i++;
    }
}

int main(void){
    char source[] =
"int func(int xyz)\n"
"{\n"
"xyz=xyz+1;\n"
"return xyz;\n"
"}\n"
"int main()\n"
"{ int a, b=5;\n"
"for(int i=0;i<=10;i++)\n"
"{ b=b+func(i);\n"
"return 0;\n"
"}\n";

    tokenize(source);
    printf("==== TASK 1 : LEXEME SEPARATION ====\n");
    for(int k=0;k<lexCount;k++){
        printf("%s\n", lexList[k].val);
    }
    return 0;
}
