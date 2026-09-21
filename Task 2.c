#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEX 500
#define MAX_STR 128
#define MAX_LIST 200

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
            char op[3]={0}; op[0]=ch; op[1]=src[i+1];
            addLex(op, OPERATOR); i+=2; continue;
        }
        if(strchr("+-*/=();{},", ch)){
            char op[2]={0}; op[0]=ch;
            if(strchr("();{},",ch)) addLex(op, DELIMITER);
            else addLex(op, OPERATOR);
            i++; continue;
        }
        if(isdigit(ch)){
            char num[MAX_STR]={0}; int p=0;
            while(i<len && (isdigit(src[i])||src[i]=='.')) num[p++]=src[i++];
            addLex(num, LITERAL); continue;
        }
        if(isalpha(ch)||ch=='_'){
            char id[MAX_STR]={0}; int p=0;
            while(i<len && (isalnum(src[i])||src[i]=='_')) id[p++]=src[i++];
            if(isKeyword(id)) addLex(id, KEYWORD);
            else addLex(id, IDENTIFIER);
            continue;
        }
        i++;
    }
}


void addUnique(char arr[][MAX_STR], int *cnt, char *s){
    for(int i=0;i<*cnt;i++){
        if(strcmp(arr[i],s)==0) return;
    }
    strncpy(arr[*cnt], s, MAX_STR-1);
    (*cnt)++;
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

    char funcs[MAX_LIST][MAX_STR]; int fCnt=0;
    char vars[MAX_LIST][MAX_STR]; int vCnt=0;
    char ops[MAX_LIST][MAX_STR]; int oCnt=0;
    char kw[MAX_LIST][MAX_STR]; int kwCnt=0;
    char others[MAX_LIST][MAX_STR]; int otCnt=0;

    for(int i=0;i<lexCount;i++){
        Lexeme *lx = &lexList[i];
        if(lx->cat == KEYWORD){
            addUnique(kw, &kwCnt, lx->val);
        }else if(lx->cat == OPERATOR){
            addUnique(ops, &oCnt, lx->val);
        }else if(lx->cat == DELIMITER || lx->cat == LITERAL){
            addUnique(others, &otCnt, lx->val);
        }else if(lx->cat == IDENTIFIER){

            if( (i+1 < lexCount) && strcmp(lexList[i+1].val,"(")==0 ){
                addUnique(funcs, &fCnt, lx->val);
            }else{
                addUnique(vars, &vCnt, lx->val);
            }
        }
    }

    printf("==== TASK 2 : LEXEME GROUPING ====\n");
    printf("Functions: ");
    for(int i=0;i<fCnt;i++) printf("%s%s",funcs[i],(i<fCnt-1)?", ":"");
    printf("\nVariables: ");
    for(int i=0;i<vCnt;i++) printf("%s%s",vars[i],(i<vCnt-1)?", ":"");
    printf("\nOperators: ");
    for(int i=0;i<oCnt;i++) printf("%s%s",ops[i],(i<oCnt-1)?", ":"");
    printf("\nKeywords: ");
    for(int i=0;i<kwCnt;i++) printf("%s%s",kw[i],(i<kwCnt-1)?", ":"");
    printf("\nOthers: ");
    for(int i=0;i<otCnt;i++) printf("%s%s",others[i],(i<otCnt-1)?", ":"");
    printf("\n");
    return 0;
}
