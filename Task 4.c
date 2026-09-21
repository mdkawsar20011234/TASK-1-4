#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEX 600
#define MAX_STR 128
#define MAX_SYM 200
#define MAX_ERR 100
#define MAX_FUNCS 50

typedef enum {
    KEYWORD, IDENTIFIER, OPERATOR, LITERAL, DELIMITER
} Cat;

typedef struct {
    char val[MAX_STR];
    Cat cat;
} Lexeme;

Lexeme lexList[MAX_LEX];
int lexCount = 0;
int pos;

char symVar[MAX_SYM][MAX_STR];
char symType[MAX_SYM][MAX_STR];
int symSize;

char errors[MAX_ERR][256];
int errCount;

char funcNames[MAX_FUNCS][MAX_STR];
int funcCount;

char *keywords[] = {"int","float","char","return","for", NULL};

int isKeyword(char *s){
    for(int i=0; keywords[i];i++){
        if(strcmp(s, keywords[i])==0) return 1;
    }
    return 0;
}

int isFunctionName(char *name){
    for(int i=0; i<funcCount; i++){
        if(strcmp(funcNames[i], name)==0) return 1;
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
    funcCount = 0;
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
            char op[2]={0};
            op[0]=ch;
            if(strchr("();{},",ch))
                addLex(op, DELIMITER);
            else
                addLex(op, OPERATOR);
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

        if(isalpha(ch)||ch=='_'){
            char id[MAX_STR]={0};
            int p=0;
            while(i<len && (isalnum(src[i])||src[i]=='_')){
                id[p++] = src[i++];
            }
            if(isKeyword(id))
                addLex(id, KEYWORD);
            else
                addLex(id, IDENTIFIER);
            continue;
        }
        i++;
    }

    /* collect function names: identifier followed by '(' */
    for(int k=0; k < lexCount; k++){
        if(lexList[k].cat == IDENTIFIER){
            if(k+1 < lexCount && strcmp(lexList[k+1].val, "(")==0){
                int dup=0;
                for(int f=0;f<funcCount;f++){
                    if(strcmp(funcNames[f], lexList[k].val)==0){
                        dup=1;
                        break;
                    }
                }
                if(!dup && funcCount < MAX_FUNCS){
                    strncpy(funcNames[funcCount], lexList[k].val, MAX_STR-1);
                    funcCount++;
                }
            }
        }
    }
}

Lexeme* peek(void){
    if(pos >= lexCount) return NULL;
    return &lexList[pos];
}

Lexeme* consume(void){
    if(pos >= lexCount) return NULL;
    return &lexList[pos++];
}

int match(char *s){
    Lexeme *p = peek();
    if(p && strcmp(p->val, s)==0){
        consume();
        return 1;
    }
    return 0;
}

void addError(char *msg){
    if(errCount >= MAX_ERR) return;
    strncpy(errors[errCount], msg, 255);
    errCount++;
}

void symInit(void){
    symSize=0;
}

void symInsert(char *var, char *typ){
    for(int i=0;i<symSize;i++){
        if(strcmp(symVar[i],var)==0) return;
    }
    strncpy(symVar[symSize], var, MAX_STR-1);
    strncpy(symType[symSize], typ, MAX_STR-1);
    symSize++;
}

char* symLookup(char *var){
    for(int i=0;i<symSize;i++){
        if(strcmp(symVar[i],var)==0){
            return symType[i];
        }
    }
    return NULL;
}

char tmpRetType[MAX_STR];
char* parseType(void);
void parseParameterList(void);
void parseParameter(void);
void parseStatementList(void);
void parseStatement(void);
void parseDeclaration(void);
void parseAssignment(void);
void parseReturnStatement(void);
char* parseExpression(void);
char* parseTerm(void);
char* parseFactor(void);

void parseProgram(void){
    parseType();
    consume();
    match("(");
    parseParameterList();
    match(")");
    match("{");
    parseStatementList();
    match("}");
}

char* parseType(void){
    Lexeme *p = peek();
    if(p && (strcmp(p->val,"int")==0 || strcmp(p->val,"float")==0 || strcmp(p->val,"char")==0)){
        return consume()->val;
    }
    return NULL;
}

void parseParameterList(void){
    Lexeme *p = peek();
    if(p && (strcmp(p->val,"int")==0||strcmp(p->val,"float")==0||strcmp(p->val,"char")==0)){
        parseParameter();
        while(match(",")){
            parseParameter();
        }
    }
}

void parseParameter(void){
    char *t = parseType();
    Lexeme *id = consume();
    symInsert(id->val, t);
}

void parseStatementList(void){
    while(1){
        Lexeme *n = peek();
        if(!n || strcmp(n->val,"}")==0){
            break;
        }
        parseStatement();
    }
}

void parseStatement(void){
    Lexeme *n = peek();
    if(strcmp(n->val,"int")==0 || strcmp(n->val,"float")==0 || strcmp(n->val,"char")==0){
        parseDeclaration();
    }else if(strcmp(n->val,"return")==0){
        parseReturnStatement();
    }else{
        parseAssignment();
    }
}

void parseDeclaration(void){
    char *declT = parseType();
    Lexeme *varTok = consume();
    symInsert(varTok->val, declT);
    if(match("=")){
        char *eT = parseExpression();
        if(eT && strcmp(eT, declT)!=0){
            char buf[256];
            sprintf(buf,"Type Compatibility Error: Can't assign %s to %s.", eT, declT);
            addError(buf);
        }
    }
    match(";");
}

void parseAssignment(void){
    Lexeme *varTok = consume();
    char *varName = varTok->val;
    char *exprT = parseExpression();
    char *lhsT = symLookup(varName);
    if(!lhsT){
        char buf[256];
        sprintf(buf,"Type Compatibility Error: Variable %s not declared.", varName);
        addError(buf);
    }else{
        if(exprT && strcmp(exprT, lhsT)!=0){
            char buf[256];
            sprintf(buf,"Type Compatibility Error: Can't assign %s to %s.", exprT, lhsT);
            addError(buf);
        }
    }
    match(";");
}

void parseReturnStatement(void){
    consume();
    parseExpression();
    match(";");
}

char* parseExpression(void){
    char *t = parseTerm();
    while(1){
        Lexeme *p = peek();
        if(p && (strcmp(p->val,"+")==0 || strcmp(p->val,"-")==0)){
            consume();
            char *rt = parseTerm();
            if( (strcmp(t,"int")==0 && rt && strcmp(rt,"float")==0) ){
                strcpy(tmpRetType, "float");
                t = tmpRetType;
            }
        }else{
            break;
        }
    }
    return t;
}

char* parseTerm(void){
    char *f = parseFactor();
    while(1){
        Lexeme *p = peek();
        if(p && (strcmp(p->val,"*")==0 || strcmp(p->val,"/")==0)){
            consume();
            char *rt = parseFactor();
            if( (strcmp(f,"int")==0 && rt && strcmp(rt,"float")==0) ){
                strcpy(tmpRetType, "float");
                f = tmpRetType;
            }
        }else{
            break;
        }
    }
    return f;
}

char* parseFactor(void){
    Lexeme *p = peek();
    if(strcmp(p->val,"(")==0){
        consume();
        char *et = parseExpression();
        match(")");
        return et;
    }
    else if(p->cat == IDENTIFIER){
        Lexeme *idtok = consume();
        if(isFunctionName(idtok->val)){
            /* skip function call arguments inside () */
            if(match("(")){
                int depth=1;
                while(depth>0 && peek()!=NULL){
                    if(match("(")) depth++;
                    else if(match(")")) depth--;
                    else consume();
                }
            }
            strcpy(tmpRetType, "int");
            return tmpRetType;
        }
        char *tp = symLookup(idtok->val);
        if(!tp){
            addError("Type Compatibility Error: Type mismatched.");
            return NULL;
        }
        return tp;
    }
    else if(p->cat == LITERAL){
        Lexeme *lt = consume();
        if(strchr(lt->val, '.')){
            strcpy(tmpRetType,"float");
            return tmpRetType;
        }else{
            strcpy(tmpRetType,"int");
            return tmpRetType;
        }
    }
    return NULL;
}

int main(void){
    /* Test source (removed for‑loop, BNF grammar does not support for) */
    char source[] =
"int func(int xyz)\n"
"{\n"
"xyz=xyz+1;\n"
"return xyz;\n"
"}\n"
"int main()\n"
"{ int a, b=5;\n"
"{ b=b+func(5);\n"
"return 0;\n"
"}\n";

    tokenize(source);
    pos = 0;
    symInit();
    errCount = 0;

    printf("==== TASK4 : PARSE TREE & SEMANTIC ANALYSIS ====\n");
    parseProgram();

    if(errCount == 0){
        printf("Semantic Check Result: Passed\n");
    }else{
        printf("Semantic Check Result: Failed\n");
        for(int i=0;i<errCount;i++){
            printf("> %s\n", errors[i]);
        }
    }
    return 0;
}
