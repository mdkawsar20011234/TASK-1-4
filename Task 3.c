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

typedef struct {
    int id;
    char varName[MAX_STR];
    char type[MAX_STR];
} LookupEntry;

Lexeme lexList[MAX_LEX];
int lexCount = 0;
LookupEntry table[MAX_LIST];
int tableSize = 0;

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

int getVarId(char *name){
    for(int i=0;i<tableSize;i++){
        if(strcmp(table[i].varName, name)==0) return table[i].id;
    }
    return -1;
}

void buildLookup(){
    tableSize = 0;
    int nextId = 1;
    for(int i=0;i<lexCount;i++){
        Lexeme *lx = &lexList[i];
        if(lx->cat == IDENTIFIER){

            if(i+1<lexCount && strcmp(lexList[i+1].val,"(")==0) continue;
            if(getVarId(lx->val) == -1){
                table[tableSize].id = nextId;
                strncpy(table[tableSize].varName, lx->val, MAX_STR-1);
                strcpy(table[tableSize].type, "unset");
                tableSize++;
                nextId++;
            }
        }
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
    buildLookup();

    printf("==== TASK3 : IDENTIFIER NUMBERING & LOOKUP TABLE ====\n");
    printf("Variables: ");
    for(int i=0;i<tableSize;i++){
        printf("<%d id> ", table[i].id);
    }
    printf("\nLookup table:\n");
    printf("id    variable     type\n");
    for(int i=0;i<tableSize;i++){
        printf("%-5d %-12s %s\n", table[i].id, table[i].varName, table[i].type);
    }
    return 0;
}
