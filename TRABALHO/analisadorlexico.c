// cd Trabalho -> gcc analisadorlexico.c -o analisadorlexico
// .\analisadorlexico.exe teste.arquivocerto

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEMA 100
#define TS_SIZE 1000

typedef enum {
    // Identificadores e literais
    TK_IDENTIFIER,
    TK_KEYWORD,
    TK_INTEGER,
    TK_REAL,
    
    // Operadores
    OP_EQ,      // =
    OP_GE,      // >=
    OP_MUL,     // *
    OP_NE,      // <>
    OP_LE,      // <=
    OP_DIV,     // /
    OP_GT,      // >
    OP_AD,      // +
    OP_ASS,     // :=
    OP_LT,      // <
    OP_MIN,     // -
    
    // Símbolos
    SMB_OBC,    // {
    SMB_COM,    // ,
    SMB_CBC,    // }
    SMB_SEM,    // ;
    SMB_OPA,    // (
    SMB_CPA,    // )
    SMB_DOT,    // .
    SMB_COL,    // :
    
    TK_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[MAX_LEXEMA];
    int line;
    int column;
} Token;

typedef struct {
    char lexeme[MAX_LEXEMA];
    TokenType type;
} Symbol;

Symbol symbolTable[TS_SIZE];
int symbolCount = 0;

// Lista de palavras-chave
const char* keywords[] = {
    "program", "var", "integer", "real", "begin", "end", 
    "if", "then", "else", "while", "do"
};
const int keywordCount = 11;

// Funções auxiliares
void toLowerCase(char* str) {
    for(int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

int isKeyword(char* lexeme) {
    char temp[MAX_LEXEMA];
    strcpy(temp, lexeme);
    toLowerCase(temp);
    
    for(int i = 0; i < keywordCount; i++) {
        if(strcmp(temp, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void initializeSymbolTable() {
    // Adiciona palavras reservadas à tabela de símbolos
    for(int i = 0; i < keywordCount; i++) {
        strcpy(symbolTable[symbolCount].lexeme, keywords[i]);
        symbolTable[symbolCount].type = TK_KEYWORD;
        symbolCount++;
    }
}

void addSymbol(char* lexeme, TokenType type) {
    // Verifica se já existe na tabela
    for(int i = 0; i < symbolCount; i++) {
        if(strcmp(symbolTable[i].lexeme, lexeme) == 0) {
            return;
        }
    }
    
    // Adiciona se não existir
    if(symbolCount < TS_SIZE) {
        strcpy(symbolTable[symbolCount].lexeme, lexeme);
        symbolTable[symbolCount].type = type;
        symbolCount++;
    }
}

const char* getTokenTypeName(TokenType type) {
    switch(type) {
        case TK_IDENTIFIER: return "IDENTIFIER";
        case TK_KEYWORD: return "KEYWORD";
        case TK_INTEGER: return "INTEGER";
        case TK_REAL: return "REAL";
        
        // Operadores
        case OP_EQ: return "OP_EQ";
        case OP_GE: return "OP_GE";
        case OP_MUL: return "OP_MUL";
        case OP_NE: return "OP_NE";
        case OP_LE: return "OP_LE";
        case OP_DIV: return "OP_DIV";
        case OP_GT: return "OP_GT";
        case OP_AD: return "OP_AD";
        case OP_ASS: return "OP_ASS";
        case OP_LT: return "OP_LT";
        case OP_MIN: return "OP_MIN";
        
        // Símbolos
        case SMB_OBC: return "SMB_OBC";
        case SMB_COM: return "SMB_COM";
        case SMB_CBC: return "SMB_CBC";
        case SMB_SEM: return "SMB_SEM";
        case SMB_OPA: return "SMB_OPA";
        case SMB_CPA: return "SMB_CPA";
        case SMB_DOT: return "SMB_DOT";
        case SMB_COL: return "SMB_COL";
        
        default: return "UNKNOWN";
    }
}

void writeToken(FILE* fp, Token token) {
    fprintf(fp, "<%s, %s> Line: %d, Column: %d\n", 
            getTokenTypeName(token.type), token.lexeme, token.line, token.column);
}

// Analisa identificador ou palavra-chave
Token analyzeIdentifier(FILE* fp, int startLine, int startCol, int firstChar) {
    Token token;
    int c, len = 0;
    char lexeme[MAX_LEXEMA];
    
    lexeme[len++] = firstChar;
    
    while((c = fgetc(fp)) != EOF && (isalnum(c) || c == '_')) {
        if(len < MAX_LEXEMA - 1) {
            lexeme[len++] = c;
        }
    }
    
    if(c != EOF) {
        fseek(fp, -1, SEEK_CUR);
    }
    
    lexeme[len] = '\0';
    
    strcpy(token.lexeme, lexeme);
    token.line = startLine;
    token.column = startCol;
    
    if(isKeyword(lexeme)) {
        token.type = TK_KEYWORD;
    } else {
        token.type = TK_IDENTIFIER;
        addSymbol(lexeme, TK_IDENTIFIER);
    }
    
    return token;
}

// Analisa número (inteiro ou real)
Token analyzeNumber(FILE* fp, int startLine, int startCol, int firstChar) {
    Token token;
    int c, len = 0;
    char lexeme[MAX_LEXEMA];
    int isReal = 0;
    int hasExponent = 0;
    
    lexeme[len++] = firstChar;
    
    while((c = fgetc(fp)) != EOF) {
        if(isdigit(c)) {
            if(len < MAX_LEXEMA - 1) {
                lexeme[len++] = c;
            }
        }
        else if(c == '.' && !isReal && !hasExponent) {
            if(len < MAX_LEXEMA - 1) {
                lexeme[len++] = c;
                isReal = 1;
            }
        }
        else if((c == 'E' || c == 'e') && !hasExponent) {
            if(len < MAX_LEXEMA - 1) {
                lexeme[len++] = c;
                hasExponent = 1;
                isReal = 1;
                
                // Verificar se há sinal no expoente
                int next = fgetc(fp);
                if(next == '+' || next == '-') {
                    if(len < MAX_LEXEMA - 1) {
                        lexeme[len++] = next;
                    }
                } else if(next != EOF) {
                    fseek(fp, -1, SEEK_CUR);
                }
            }
        }
        else {
            // Não é parte do número, voltar
            if(c != EOF) {
                fseek(fp, -1, SEEK_CUR);
            }
            break;
        }
    }
    
    lexeme[len] = '\0';
    
    strcpy(token.lexeme, lexeme);
    token.line = startLine;
    token.column = startCol;
    token.type = isReal ? TK_REAL : TK_INTEGER;
    
    return token;
}

// Analisa operadores, delimitadores e atribuição
Token analyzeSymbol(FILE* fp, int startLine, int startCol, int firstChar) {
    Token token;
    int c = fgetc(fp);
    
    token.line = startLine;
    token.column = startCol;
    
    // Verificar operadores de dois caracteres
    if(c != EOF) {
        char twoChars[3] = {firstChar, c, '\0'};
        
        if(strcmp(twoChars, ":=") == 0) {
            strcpy(token.lexeme, twoChars);
            token.type = OP_ASS;
            return token;
        }
        else if(strcmp(twoChars, "<=") == 0) {
            strcpy(token.lexeme, twoChars);
            token.type = OP_LE;
            return token;
        }
        else if(strcmp(twoChars, ">=") == 0) {
            strcpy(token.lexeme, twoChars);
            token.type = OP_GE;
            return token;
        }
        else if(strcmp(twoChars, "<>") == 0) {
            strcpy(token.lexeme, twoChars);
            token.type = OP_NE;
            return token;
        }
        else {
            // Não é operador de dois caracteres, voltar o caractere
            fseek(fp, -1, SEEK_CUR);
        }
    }
    
    // Operadores e símbolos de um caractere
    token.lexeme[0] = firstChar;
    token.lexeme[1] = '\0';
    
    switch(firstChar) {
        case '=': token.type = OP_EQ; break;
        case '<': token.type = OP_LT; break;
        case '>': token.type = OP_GT; break;
        case '+': token.type = OP_AD; break;
        case '-': token.type = OP_MIN; break;
        case '*': token.type = OP_MUL; break;
        case '/': token.type = OP_DIV; break;
        
        case '{': token.type = SMB_OBC; break;
        case '}': token.type = SMB_CBC; break;
        case ',': token.type = SMB_COM; break;
        case ';': token.type = SMB_SEM; break;
        case '(': token.type = SMB_OPA; break;
        case ')': token.type = SMB_CPA; break;
        case '.': token.type = SMB_DOT; break;
        case ':': token.type = SMB_COL; break;
            
        default:
            token.type = TK_UNKNOWN;
            printf("Erro léxico: caractere desconhecido '%c' na linha %d, coluna %d\n", 
                   firstChar, startLine, startCol);
            break;
    }
    
    return token;
}

// Função principal do analisador léxico
void lexicalAnalyzer(FILE* sourceFile, FILE* lexFile) {
    int c, line = 1, column = 0;
    
    while((c = fgetc(sourceFile)) != EOF) {
        column++;
        
        // Ignorar espaços em branco e tabs
        if(c == ' ' || c == '\t') {
            continue;
        }
        
        // Contar linhas
        if(c == '\n') {
            line++;
            column = 0;
            continue;
        }
        
        Token token;
        
        if(isalpha(c)) {
            // Identificador ou palavra-chave
            token = analyzeIdentifier(sourceFile, line, column, c);
        }
        else if(isdigit(c)) {
            // Número
            token = analyzeNumber(sourceFile, line, column, c);
        }
        else if(c == '+' || c == '-') {
            // Verificar se é operador ou início de número
            int next = fgetc(sourceFile);
            if(isdigit(next)) {
                // É um sinal de número
                fseek(sourceFile, -1, SEEK_CUR);
                token = analyzeNumber(sourceFile, line, column, c);
            } else {
                // É um operador
                fseek(sourceFile, -1, SEEK_CUR);
                token = analyzeSymbol(sourceFile, line, column, c);
            }
        }
        else {
            // Operadores, delimitadores ou caracteres desconhecidos
            token = analyzeSymbol(sourceFile, line, column, c);
        }
        
        writeToken(lexFile, token);
    }
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        printf("Uso: %s <arquivo fonte>\n", argv[0]);
        return 1;
    }
    
    FILE* sourceFile = fopen(argv[1], "r");
    if(!sourceFile) {
        printf("Erro ao abrir arquivo %s\n", argv[1]);
        return 1;
    }
    
    char outputFilename[MAX_LEXEMA];
    strcpy(outputFilename, argv[1]);
    char* dot = strrchr(outputFilename, '.');
    if(dot) *dot = '\0';
    strcat(outputFilename, ".lex");
    
    FILE* lexFile = fopen(outputFilename, "w");
    if(!lexFile) {
        printf("Erro ao criar arquivo de saída.\n");
        fclose(sourceFile);
        return 1;
    }
    
    // Inicializar tabela de símbolos com palavras reservadas
    initializeSymbolTable();
    
    // Executar análise léxica
    lexicalAnalyzer(sourceFile, lexFile);
    
    fclose(sourceFile);
    fclose(lexFile);
    
    // Mostrar tabela de símbolos
    printf("Tabela de Símbolos:\n");
    printf("-------------------\n");
    for(int i = 0; i < symbolCount; i++) {
        printf("<%s, %s>\n", symbolTable[i].lexeme, getTokenTypeName(symbolTable[i].type));
    }
    
    printf("\nAnálise léxica concluída. Resultados salvos em %s\n", outputFilename);
    
    return 0;
}
