#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_SYMBOLS 100
#define MAX_LEXEME 100
#define MAX_LINE_LENGTH 256

typedef enum {
    // Palavras reservadas
    TOK_PROGRAM, TOK_VAR, TOK_INTEGER, TOK_REAL, TOK_BEGIN, TOK_END,
    TOK_IF, TOK_THEN, TOK_ELSE, TOK_WHILE, TOK_DO,
    
    // Operadores
    OP_EQ, OP_GE, OP_MUL, OP_NE, OP_LE, OP_DIV, OP_GT, OP_AD, 
    OP_ASS, OP_LT, OP_MIN,OP_MOD,
    
    // Símbolos
    SMB_OBC, SMB_COM, SMB_CBC, SMB_SEM, SMB_OPA, SMB_CPA,
    SMB_COLON, SMB_DOT,
    
    // Identificadores e
    ID, LIT_INT, LIT_REAL, LIT_REAL_EXP,TOK_STRING,
    
    // Fim do arquivo e erro
    TOK_EOF, TOK_ERROR
} TokenType;

typedef struct {
    char name[MAX_LEXEME];
    TokenType type;
} Symbol;

typedef struct {
    Symbol symbols[MAX_SYMBOLS];
    int count;
} SymbolTable;

typedef struct {
    TokenType type;
    char lexeme[MAX_LEXEME];
    int line;
    int column;
} Token;

typedef struct {
    FILE* file;
    char current_char;
    int line;
    int column;
    SymbolTable symbol_table;
    char* filename; 
} Lexer;

const char* token_type_to_string(TokenType type);

void init_symbol_table(SymbolTable* table);
int insert_symbol(SymbolTable* table, const char* name, TokenType type);
Symbol* find_symbol(SymbolTable* table, const char* name);
void print_symbol_table(SymbolTable* table);

Lexer* init_lexer(FILE* file, const char* filename);
void free_lexer(Lexer* lexer);
Token get_next_token(Lexer* lexer);
void skip_whitespace(Lexer* lexer);
char peek_char(Lexer* lexer);
bool is_valid_operator_combination(char current, char next);
bool is_valid_single_char_operator(char c);
bool is_valid_operator_start(char c);
void handle_unclosed_comment(Lexer* lexer, Token* token);

void to_lower_case(char* str);

Lexer* global_lexer = NULL;
Token current_token;
int has_syntax_errors = 0;
char* current_filename = NULL;

void TokenHouse(TokenType tipo_esperado);
void SyntacticError(const char* mensagem);
void EndFile();
void ShowError();

void Program();
void Block();
void PartVariableDeclarations();
void VariableDeclararion();
void ListIdentifiers();
void Type();
void CompoundCommand();
void Command();
void Assignment();
void AdditionalCommand();
void RepetitiveCommand();
void Expression();
void Relation();
void SimpleExpression();
void Term();
void Factor();
void Variable();

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOK_PROGRAM: return "PROGRAM";
        case TOK_VAR: return "VAR";
        case TOK_INTEGER: return "INTEGER";
        case TOK_REAL: return "REAL";
        case TOK_BEGIN: return "BEGIN";
        case TOK_END: return "END";
        case TOK_IF: return "IF";
        case TOK_THEN: return "THEN";
        case TOK_ELSE: return "ELSE";
        case TOK_WHILE: return "WHILE";
        case TOK_DO: return "DO";
        
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
        
        case SMB_OBC: return "SMB_OBC";
        case SMB_COM: return "SMB_COM";
        case SMB_CBC: return "SMB_CBC";
        case SMB_SEM: return "SMB_SEM";
        case SMB_OPA: return "SMB_OPA";
        case SMB_CPA: return "SMB_CPA";
        case SMB_COLON: return "SMB_COLON";
        case SMB_DOT: return "SMB_DOT";
        
        case ID: return "ID";
        case LIT_INT: return "LIT_INT";
        case LIT_REAL: return "LIT_REAL";
        case LIT_REAL_EXP: return "LIT_REAL_EXP";
        
        case TOK_EOF: return "EOF";
        case TOK_ERROR: return "ERROR";
        case TOK_STRING: return "STRING";
        case OP_MOD: return "OP_MOD";

        default: return "UNKNOWN";
    }
}

void init_symbol_table(SymbolTable* table) {
    table->count = 0;
    insert_symbol(table, "program", TOK_PROGRAM);
    insert_symbol(table, "var", TOK_VAR);
    insert_symbol(table, "integer", TOK_INTEGER);
    insert_symbol(table, "real", TOK_REAL);
    insert_symbol(table, "begin", TOK_BEGIN);
    insert_symbol(table, "end", TOK_END);
    insert_symbol(table, "if", TOK_IF);
    insert_symbol(table, "then", TOK_THEN);
    insert_symbol(table, "else", TOK_ELSE);
    insert_symbol(table, "while", TOK_WHILE);
    insert_symbol(table, "do", TOK_DO);
    insert_symbol(table, "mod", OP_MOD);
}

int insert_symbol(SymbolTable* table, const char* name, TokenType type) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return 0;
        }
    }
    
    if (table->count < MAX_SYMBOLS) {
        strcpy(table->symbols[table->count].name, name);
        table->symbols[table->count].type = type;
        table->count++;
        return 1;
    }  
    return -1; 
}

Symbol* find_symbol(SymbolTable* table, const char* name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

void print_symbol_table(SymbolTable* table) {
    printf("\n=== TABELA DE SIMBOLOS ===\n");
    printf("%-20s %-15s\n", "Nome", "Tipo");
    printf("--------------------------------\n");
    
    for (int i = 0; i < table->count; i++) {
        printf("%-20s ", table->symbols[i].name);
        
        switch (table->symbols[i].type) {
            case TOK_PROGRAM: printf("\033[1;33m%-15s\033[0m", "PROGRAM"); break;
            case TOK_VAR: printf("\033[1;33m%-15s\033[0m", "VAR"); break;
            case TOK_INTEGER: printf("\033[1;33m%-15s\033[0m", "INTEGER"); break;
            case TOK_REAL: printf("\033[1;33m%-15s\033[0m", "REAL"); break;
            case TOK_BEGIN: printf("\033[1;33m%-15s\033[0m", "BEGIN"); break;
            case TOK_END: printf("\033[1;33m%-15s\033[0m", "END"); break;
            case TOK_IF: printf("\033[1;33m%-15s\033[0m", "IF"); break;
            case TOK_THEN: printf("\033[1;33m%-15s\033[0m", "THEN"); break;
            case TOK_ELSE: printf("\033[1;33m%-15s\033[0m", "ELSE"); break;
            case TOK_WHILE: printf("\033[1;33m%-15s\033[0m", "WHILE"); break;
            case TOK_DO: printf("\033[1;33m%-15s\033[0m", "DO"); break;
            case ID: printf("\033[1;33m%-15s\033[0m", "ID"); break;
            default: printf("\033[1;33m%-15s\033[0m", "UNKNOWN"); break;
        }
        printf("\n");
    }
}

void to_lower_case(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

bool is_valid_operator_start(char c) {
    return c == ':' || c == '<' || c == '>' || c == '=' || 
           c == '+' || c == '-' || c == '*' || c == '/';
}

bool is_valid_single_char_operator(char c) {
    return c == '=' || c == '+' || c == '-' || c == '*' || 
           c == '/' || c == '<' || c == '>';
}

bool is_valid_operator_combination(char current, char next) {
    if (current == ':' && next == '=') return true;    
    if (current == '<' && next == '=') return true;    
    if (current == '<' && next == '>') return true;   
    if (current == '>' && next == '=') return true;    
    
    if (is_valid_single_char_operator(current) && 
        !is_valid_operator_start(next)) {
        return true;
    }
    
    return false;
}

void handle_unclosed_comment(Lexer* lexer, Token* token) {
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    while (lexer->current_char != EOF && lexer->current_char != '}') {
        if (lexer->current_char == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
        lexer->current_char = fgetc(lexer->file);
    }
    
    if (lexer->current_char == '}') {
        token->type = TOK_ERROR;
        sprintf(token->lexeme, "Conteudo entre { } nao permitido (comentarios nao suportados)");
        lexer->current_char = fgetc(lexer->file);
        lexer->column++;
    } else {

        token->type = TOK_ERROR;
        sprintf(token->lexeme, "Comentario nao fechado iniciado na linha %d, coluna %d", 
                start_line, start_column);
    }
}

Lexer* init_lexer(FILE* file, const char* filename) {
    Lexer* lexer = malloc(sizeof(Lexer));
    lexer->file = file;
    lexer->current_char = fgetc(file);
    lexer->line = 1;
    lexer->column = 1;
    lexer->filename = malloc(strlen(filename) + 1);
    strcpy(lexer->filename, filename);
    init_symbol_table(&lexer->symbol_table);
    return lexer;
}

void free_lexer(Lexer* lexer) {
    fclose(lexer->file);
    free(lexer->filename);
    free(lexer);
}

char peek_char(Lexer* lexer) {
    if (feof(lexer->file)) return '\0';
    
    long current_pos = ftell(lexer->file);
    char next_char = fgetc(lexer->file);
    fseek(lexer->file, current_pos, SEEK_SET);
    
    return next_char;
}

void skip_whitespace(Lexer* lexer) {
    while (lexer->current_char != EOF && 
           (lexer->current_char == ' ' || 
            lexer->current_char == '\t' || 
            lexer->current_char == '\n')) {
        
        if (lexer->current_char == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
        
        lexer->current_char = fgetc(lexer->file);
    }
}

Token get_next_token(Lexer* lexer) {
    Token token;
    token.line = lexer->line;
    token.column = lexer->column;
    token.lexeme[0] = '\0';
    
    skip_whitespace(lexer);
    
    if (lexer->current_char == EOF) {
        token.type = TOK_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }
    
    if (isalpha(lexer->current_char)) {
        int i = 0;

        token.lexeme[i++] = tolower(lexer->current_char);
        lexer->current_char = fgetc(lexer->file);
        lexer->column++;
        
        while ((isalnum(lexer->current_char) || lexer->current_char == '_') && i < MAX_LEXEME - 1) {
            token.lexeme[i++] = tolower(lexer->current_char);
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
        }
        token.lexeme[i] = '\0';
        
        if (strcmp(token.lexeme, "program") == 0) token.type = TOK_PROGRAM;
        else if (strcmp(token.lexeme, "var") == 0) token.type = TOK_VAR;
        else if (strcmp(token.lexeme, "integer") == 0) token.type = TOK_INTEGER;
        else if (strcmp(token.lexeme, "real") == 0) token.type = TOK_REAL;
        else if (strcmp(token.lexeme, "begin") == 0) token.type = TOK_BEGIN;
        else if (strcmp(token.lexeme, "end") == 0) token.type = TOK_END;
        else if (strcmp(token.lexeme, "if") == 0) token.type = TOK_IF;
        else if (strcmp(token.lexeme, "then") == 0) token.type = TOK_THEN;
        else if (strcmp(token.lexeme, "else") == 0) token.type = TOK_ELSE;
        else if (strcmp(token.lexeme, "while") == 0) token.type = TOK_WHILE;
        else if (strcmp(token.lexeme, "do") == 0) token.type = TOK_DO;
        else if (strcmp(token.lexeme, "mod") == 0) token.type = OP_MOD;
        else {
            token.type = ID;
            insert_symbol(&lexer->symbol_table, token.lexeme, ID);
        }
        
        return token;
    }
    
    if (isdigit(lexer->current_char) || 
        ((lexer->current_char == '+' || lexer->current_char == '-') && isdigit(peek_char(lexer)))) {
        
        int i = 0;
        int is_real = 0;
        int has_exponent = 0;
        
        if (lexer->current_char == '+' || lexer->current_char == '-') {
            token.lexeme[i++] = lexer->current_char;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
        }
        
        while (isdigit(lexer->current_char) && i < MAX_LEXEME - 1) {
            token.lexeme[i++] = lexer->current_char;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
        }
        
        if (lexer->current_char == '.') {
            is_real = 1;
            token.lexeme[i++] = lexer->current_char;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            
            while (isdigit(lexer->current_char) && i < MAX_LEXEME - 1) {
                token.lexeme[i++] = lexer->current_char;
                lexer->current_char = fgetc(lexer->file);
                lexer->column++;
            }
        }
        
        if (lexer->current_char == 'E' || lexer->current_char == 'e') {
            is_real = 1;
            has_exponent = 1;
            token.lexeme[i++] = 'E';
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            
            if (lexer->current_char == '+' || lexer->current_char == '-') {
                token.lexeme[i++] = lexer->current_char;
                lexer->current_char = fgetc(lexer->file);
                lexer->column++;
            }
            
            while (isdigit(lexer->current_char) && i < MAX_LEXEME - 1) {
                token.lexeme[i++] = lexer->current_char;
                lexer->current_char = fgetc(lexer->file);
                lexer->column++;
            }
        }
        
        token.lexeme[i] = '\0';
        
        if (has_exponent) {
            token.type = LIT_REAL_EXP;
        } else if (is_real) {
            token.type = LIT_REAL;
        } else {
            token.type = LIT_INT;
        }
        
        return token;
    }
    
    char next_char = peek_char(lexer);
    
    if (is_valid_operator_start(lexer->current_char) && lexer->current_char != ':' && lexer->current_char != '.') {
        if (!is_valid_operator_combination(lexer->current_char, next_char)) {
            token.type = TOK_ERROR;
            if (is_valid_operator_start(next_char)) {
                sprintf(token.lexeme, "Operador invalido: '%c%c'", lexer->current_char, next_char);
                lexer->current_char = fgetc(lexer->file);
                lexer->column++;
            } else {
                sprintf(token.lexeme, "Operador invalido: '%c'", lexer->current_char);
            }
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            return token;
        }
    }
    
    switch (lexer->current_char) {
        case ':':
            token.lexeme[0] = lexer->current_char;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            
            if (lexer->current_char == '=') {
                token.lexeme[1] = '=';
                token.lexeme[2] = '\0';
                token.type = OP_ASS;
                lexer->current_char = fgetc(lexer->file);
                lexer->column++;
            } else {
                token.lexeme[1] = '\0';
                token.type = SMB_COLON;
            }
            break;
            
        case '.':
            token.lexeme[0] = lexer->current_char;
            token.lexeme[1] = '\0';
            token.type = SMB_DOT;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            break;
            
        case '"': 
            token.type = TOK_ERROR;
            strcpy(token.lexeme, " O caracter \" nao e permitido");
            while (lexer->current_char != EOF && lexer->current_char != '"' && lexer->current_char != '\n') {
                lexer->current_char = fgetc(lexer->file);
                lexer->column++;
            }
            if (lexer->current_char == '\n') {
                sprintf(token.lexeme, "String nao fechada antes da quebra de linha");
            } else if (lexer->current_char == '"') {
                lexer->current_char = fgetc(lexer->file);
                lexer->column++;
            }
            break;
            
        case '{':
            if (peek_char(lexer) != '}') {
                handle_unclosed_comment(lexer, &token);
                return token;
            }
            
            token.lexeme[0] = lexer->current_char;
            token.lexeme[1] = '\0';
            token.type = SMB_OBC;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            break;
            
        case '<':
            token.lexeme[0] = lexer->current_char;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            
            if (lexer->current_char == '=') {
                token.lexeme[1] = '=';
                token.lexeme[2] = '\0';
                token.type = OP_LE;
                lexer->current_char = fgetc(lexer->file);
                lexer->column++;
            } else if (lexer->current_char == '>') {
                token.lexeme[1] = '>';
                token.lexeme[2] = '\0';
                token.type = OP_NE;
                lexer->current_char = fgetc(lexer->file);
                lexer->column++;
            } else {
                token.lexeme[1] = '\0';
                token.type = OP_LT;
            }
            break;
            
        case '>':
            token.lexeme[0] = lexer->current_char;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            
            if (lexer->current_char == '=') {
                token.lexeme[1] = '=';
                token.lexeme[2] = '\0';
                token.type = OP_GE;
                lexer->current_char = fgetc(lexer->file);
                lexer->column++;
            } else {
                token.lexeme[1] = '\0';
                token.type = OP_GT;
            }
            break;
            
        case '=':
            token.lexeme[0] = lexer->current_char;
            token.lexeme[1] = '\0';
            token.type = OP_EQ;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            break;
            
        case '+':
            token.lexeme[0] = lexer->current_char;
            token.lexeme[1] = '\0';
            token.type = OP_AD;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            break;
            
        case '-':
            token.lexeme[0] = lexer->current_char;
            token.lexeme[1] = '\0';
            token.type = OP_MIN;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            break;
            
        case '*':
            token.lexeme[0] = lexer->current_char;
            token.lexeme[1] = '\0';
            token.type = OP_MUL;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            break;
            
        case '/':
            token.lexeme[0] = lexer->current_char;
            token.lexeme[1] = '\0';
            token.type = OP_DIV;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            break;
            
        case '}':
            token.lexeme[0] = lexer->current_char;
            token.lexeme[1] = '\0';
            token.type = SMB_CBC;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            break;
            
        case '(':
            token.lexeme[0] = lexer->current_char;
            token.lexeme[1] = '\0';
            token.type = SMB_OPA;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            break;
            
        case ')':
            token.lexeme[0] = lexer->current_char;
            token.lexeme[1] = '\0';
            token.type = SMB_CPA;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            break;
            
        case ',':
            token.lexeme[0] = lexer->current_char;
            token.lexeme[1] = '\0';
            token.type = SMB_COM;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            break;
            
        case ';':
            token.lexeme[0] = lexer->current_char;
            token.lexeme[1] = '\0';
            token.type = SMB_SEM;
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            break;

        case '\'': {
        int start_line = lexer->line;
        int start_column = lexer->column;
        int i = 0;

        token.lexeme[i++] = '\''; 

        lexer->current_char = fgetc(lexer->file);
        lexer->column++;

        while (lexer->current_char != '\'' && lexer->current_char != EOF && lexer->current_char != '\n') {
            if (i >= MAX_LEXEME - 2) { 
                token.type = TOK_ERROR;
                sprintf(token.lexeme, "String muito longa na linha %d, coluna %d", start_line, start_column);
                return token;
            }
                token.lexeme[i++] = lexer->current_char;
                lexer->current_char = fgetc(lexer->file);
                lexer->column++;
            }

            if (lexer->current_char == '\'') {
                token.lexeme[i++] = '\''; 
                token.lexeme[i] = '\0';
                token.type = TOK_STRING;
                lexer->current_char = fgetc(lexer->file); 
                lexer->column++;
            } else {
                token.lexeme[i] = '\0';
                token.type = TOK_ERROR;
                sprintf(token.lexeme, "String nao fechada na linha %d, coluna %d", start_line, start_column);
                }
            }
            break;
     
        default:
            token.type = TOK_ERROR;
            sprintf(token.lexeme, "Caractere desconhecido: '%c'", lexer->current_char);
            lexer->current_char = fgetc(lexer->file);
            lexer->column++;
            break;
    }
    return token;
}


void ShowError() {
    if (global_lexer == NULL || global_lexer->file == NULL) return;
    
    long current_pos = ftell(global_lexer->file);
    
    FILE* file = fopen(global_lexer->filename, "r");
    if (!file) return;
    
    char linha[MAX_LINE_LENGTH];
    int linha_atual = 1;
    
    while (linha_atual < current_token.line && fgets(linha, sizeof(linha), file)) {
        linha_atual++;
    }

    if (linha_atual == current_token.line && fgets(linha, sizeof(linha), file)) {
        linha[strcspn(linha, "\n")] = '\0';
        
        printf("     Linha %d: %s\n", current_token.line, linha);
        
        printf("     ");
        for (int i = 1; i < current_token.column; i++) {
            if (i < (int)strlen(linha) && linha[i-1] == '\t') {
                printf("\t"); 
            } else {
                printf(" ");
            }
        }
        printf("\033[1;31m^\033[0m\n");
        for (int i = 1; i < current_token.column; i++) {
            printf(" ");
        }
        printf("\033[1;33mO Erro esta nesta linha acima\033[0m\n");
    }
    
    fclose(file);
    
    fseek(global_lexer->file, current_pos, SEEK_SET);
}

void SyntacticError(const char* mensagem) {
    printf("\033[1;31mERRO SINTATICO (Linha %d): %s", current_token.line, mensagem);
    
    if (current_token.type == TOK_EOF) {
        printf(" - fim de arquivo encontrado\033[0m\n");
    } else {
        printf(" - encontrado [%s]\033[0m\n", current_token.lexeme);
    }
    
    ShowError();
    
    has_syntax_errors = 1;
}

void EndFile() {
    if (current_token.type != TOK_EOF && !has_syntax_errors) {
        SyntacticError("simbolos extras apos fim do programa");
    }
}

void TokenHouse(TokenType tipo_esperado) {
    if (current_token.type == tipo_esperado) {
        current_token = get_next_token(global_lexer);
    } else {
        SyntacticError("token nao esperado");
        if (current_token.type != TOK_EOF && current_token.type != TOK_ERROR) {
            current_token = get_next_token(global_lexer);
        }
    }
}

void Program() {
    printf("programa -> program ID ; bloco .\n");
    TokenHouse(TOK_PROGRAM);
    if (has_syntax_errors) return;
    TokenHouse(ID);
    if (has_syntax_errors) return;
    TokenHouse(SMB_SEM);
    if (has_syntax_errors) return;
    Block();
    if (has_syntax_errors) return;
    TokenHouse(SMB_DOT);
    if (!has_syntax_errors) {
        printf("Programa analisado com sucesso!\n");
    }
    
    EndFile();
}

void Block() {
    if (has_syntax_errors) return;
    printf("bloco -> parte_declaracoes_variaveis comando_composto\n");
    PartVariableDeclarations();
    if (has_syntax_errors) return;
    CompoundCommand();
}

void PartVariableDeclarations() {
    if (has_syntax_errors) return;
    printf("parte_declaracoes_variaveis -> var declaracao_variaveis { ; declaracao_variaveis }\n");
    if (current_token.type == TOK_VAR) {
        TokenHouse(TOK_VAR);
        if (has_syntax_errors) return;
        VariableDeclararion();
        while (current_token.type == SMB_SEM && !has_syntax_errors) {
            TokenHouse(SMB_SEM);
            if (has_syntax_errors) break;
            if (current_token.type == TOK_BEGIN || current_token.type == TOK_EOF) break;
            VariableDeclararion();
        }
    }
}

void VariableDeclararion() {
    if (has_syntax_errors) return;
    printf("declaracao_variaveis -> lista_identificadores : tipo\n");
    ListIdentifiers();
    if (has_syntax_errors) return;
    TokenHouse(SMB_COLON);
    if (has_syntax_errors) return;
    Type();
}

void ListIdentifiers() {
    if (has_syntax_errors) return;
    printf("lista_identificadores -> ID { , ID }\n");
    TokenHouse(ID);
    while (current_token.type == SMB_COM && !has_syntax_errors) {
        TokenHouse(SMB_COM);
        if (has_syntax_errors) break;
        TokenHouse(ID);
    }
}

void Type() {
    if (has_syntax_errors) return;
    printf("tipo -> integer | real\n");
    if (current_token.type == TOK_INTEGER) {
        TokenHouse(TOK_INTEGER);
    } else if (current_token.type == TOK_REAL) {
        TokenHouse(TOK_REAL);
    } else {
        SyntacticError("tipo esperado (integer ou real)");
    }
}

void CompoundCommand() {
    if (has_syntax_errors) return;
    printf("comando_composto -> begin comando ; { comando ; } end\n");
    TokenHouse(TOK_BEGIN);
    if (has_syntax_errors) return;

    if (current_token.type == TOK_EOF) {
        SyntacticError("comando esperado apos begin");
        return;
    }
    
    Command();
    if (has_syntax_errors) return;
    TokenHouse(SMB_SEM);
    if (has_syntax_errors) return;

    while (current_token.type != TOK_END && current_token.type != TOK_EOF && !has_syntax_errors) {
        Command();
        if (has_syntax_errors) break;
        if (current_token.type == TOK_END || current_token.type == TOK_EOF) break;
        TokenHouse(SMB_SEM);
        if (has_syntax_errors) break;
    }
    
    if (!has_syntax_errors) {
        TokenHouse(TOK_END);
    }
}

void Command() {
    if (has_syntax_errors || current_token.type == TOK_EOF) return;
    
    printf("comando -> atribuicao | comando_composto | comando_condicional | comando_repetitivo\n");
    
    if (current_token.type == TOK_EOF) {
        SyntacticError("comando esperado");
        return;
    }
    
    if (current_token.type == ID) {
        Assignment();
    } else if (current_token.type == TOK_BEGIN) {
        CompoundCommand();
    } else if (current_token.type == TOK_IF) {
        AdditionalCommand();
    } else if (current_token.type == TOK_WHILE) {
        RepetitiveCommand();
    } else {
        SyntacticError("comando esperado");
        if (current_token.type != TOK_EOF && current_token.type != TOK_ERROR) {
            current_token = get_next_token(global_lexer);
        }
    }
}

void Assignment() {
    if (has_syntax_errors) return;
    printf("atribuicao -> variavel := expressao\n");
    Variable();
    if (has_syntax_errors) return;
    TokenHouse(OP_ASS);
    if (has_syntax_errors) return;
    Expression();
}

void AdditionalCommand() {
    if (has_syntax_errors) return;
    printf("comando_condicional -> if expressao then comando [ else comando ]\n");
    TokenHouse(TOK_IF);
    if (has_syntax_errors) return;
    Expression();
    if (has_syntax_errors) return;
    TokenHouse(TOK_THEN);
    if (has_syntax_errors) return;
    Command();
    if (current_token.type == TOK_ELSE && !has_syntax_errors) {
        TokenHouse(TOK_ELSE);
        if (has_syntax_errors) return;
        Command();
    }
}

void RepetitiveCommand() {
    if (has_syntax_errors) return;
    printf("comando_repetitivo -> while expressao do comando\n");
    TokenHouse(TOK_WHILE);
    if (has_syntax_errors) return;
    Expression();
    if (has_syntax_errors) return;
    TokenHouse(TOK_DO);
    if (has_syntax_errors) return;
    Command();
}

void Expression() {
    if (has_syntax_errors) return;
    printf("expressao -> expressao_simples [ relacao expressao_simples ]\n");
    SimpleExpression();
    if (!has_syntax_errors && 
        (current_token.type == OP_EQ || current_token.type == OP_NE || 
         current_token.type == OP_LT || current_token.type == OP_LE ||
         current_token.type == OP_GT || current_token.type == OP_GE)) {
        Relation();
        if (has_syntax_errors) return;
        SimpleExpression();
    }
}

void Relation() {
    if (has_syntax_errors) return;
    printf("relacao -> = | < | <= | >= | > | <>\n");
    switch (current_token.type) {
        case OP_EQ: TokenHouse(OP_EQ); break;
        case OP_NE: TokenHouse(OP_NE); break;
        case OP_LT: TokenHouse(OP_LT); break;
        case OP_LE: TokenHouse(OP_LE); break;
        case OP_GT: TokenHouse(OP_GT); break;
        case OP_GE: TokenHouse(OP_GE); break;
        default: SyntacticError("operador relacional esperado");
    }
}

void SimpleExpression() {
    if (has_syntax_errors) return;
    printf("expressao_simples -> [+ | -] termo { (+ | - ) termo }\n");
    if (current_token.type == OP_AD || current_token.type == OP_MIN) {
        if (current_token.type == OP_AD) TokenHouse(OP_AD);
        else TokenHouse(OP_MIN);
    }
    if (has_syntax_errors) return;
    Term();
    while (!has_syntax_errors && (current_token.type == OP_AD || current_token.type == OP_MIN)) {
        if (current_token.type == OP_AD) TokenHouse(OP_AD);
        else TokenHouse(OP_MIN);
        if (has_syntax_errors) break;
        Term();
    }
}

void Term() {
    if (has_syntax_errors) return;
    printf("termo -> fator { (* | / | mod) fator }\n");
    Factor();
    while (!has_syntax_errors && 
           (current_token.type == OP_MUL || current_token.type == OP_DIV || 
            current_token.type == OP_MOD)) { 
        if (current_token.type == OP_MUL) TokenHouse(OP_MUL);
        else if (current_token.type == OP_DIV) TokenHouse(OP_DIV);
        else if (current_token.type == OP_MOD) TokenHouse(OP_MOD); 
        if (has_syntax_errors) break;
        Factor();
    }
}

void Factor() {
    if (has_syntax_errors) return;
    printf("fator -> variavel | numero | ( expressao )\n");
    if (current_token.type == ID) {
        Variable();
    } else if (current_token.type == LIT_INT || current_token.type == LIT_REAL || current_token.type == LIT_REAL_EXP) {
        if (current_token.type == LIT_INT) TokenHouse(LIT_INT);
        else if (current_token.type == LIT_REAL) TokenHouse(LIT_REAL);
        else TokenHouse(LIT_REAL_EXP);
    } else if (current_token.type == SMB_OPA) {
        TokenHouse(SMB_OPA);
        if (has_syntax_errors) return;
        Expression();
        if (has_syntax_errors) return;
        TokenHouse(SMB_CPA);
    } else {
        SyntacticError("fator esperado (variavel, numero ou expressao entre parenteses)");
    }
}

void Variable() {
    if (has_syntax_errors) return;
    printf("variavel -> ID\n");
    TokenHouse(ID);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Uso: %s <arquivo.mpas>\n", argv[0]);
        return 1;
    }
    
    printf("\t\t--- ANALISE LEXICA ---\n");
    
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        printf("Erro ao abrir arquivo: %s\n", argv[1]);
        return 1;
    }
    
    Lexer* lexer = init_lexer(file, argv[1]);
    
    char output_filename[100];
    snprintf(output_filename, sizeof(output_filename), "%s.lex", argv[1]);
    FILE* output_file = fopen(output_filename, "w");
    
    if (!output_file) {
        printf("Erro ao criar arquivo de saida\n");
        free_lexer(lexer);
        return 1;
    }
    
    fprintf(output_file, "\t\t=== TOKENS RECONHECIDOS ===\n");
    fprintf(output_file, "%-15s %-18s %-8s %-8s\n", "TOKEN", "LEXEMA", "LINHA", "COLUNA");
    fprintf(output_file, "--------------------------------------------------\n");
    
    printf("\t   === TOKENS RECONHECIDOS ===\n");
    printf("%-15s %-18s %-8s %-8s\n", "TOKEN", "LEXEMA", "LINHA", "COLUNA");
    printf("------------------------------------------------\n");

    Token token;
    int has_lexical_errors = 0;
    
    do {
        token = get_next_token(lexer);
        
        if (token.type == TOK_ERROR) {
            has_lexical_errors = 1;
            printf("\033[1;31mERRO\033[0m (Linha %d, Coluna %d): %s\n", 
                   token.line, token.column, token.lexeme);
            fprintf(output_file, "ERRO (Linha %d, Coluna %d): %s\n", 
                    token.line, token.column, token.lexeme);
        } else if (token.type != TOK_EOF) {
            printf("\033[1;33m%-15s\033[0m %-20s %-8d %-8d\n", 
                   token_type_to_string(token.type), token.lexeme, 
                   token.line, token.column);
            fprintf(output_file, "%-15s %-20s %-8d %-8d\n", 
                    token_type_to_string(token.type), token.lexeme, 
                    token.line, token.column);
        }
        
    } while (token.type != TOK_EOF);
    
    print_symbol_table(&lexer->symbol_table);
    
    fclose(output_file);
    
    if (has_lexical_errors) {
        printf("\nAnalise lexica concluida com \033[1;31mERROS!\033[0m\ncontinuando analise sintatica...\n");
    } else {
        printf("\n\033[1;32mAnalise lexica concluida com SUCESSO!\033[0m\n");
    }
    
    printf("\n\033[1;35mTokens salvos em:\033[0m %s\n", output_filename);
    
    printf("\n\t---- ANALISE SINTATICA ----\n");
    
    file = fopen(argv[1], "r");
    if (!file) {
        printf("Erro ao reabrir arquivo para analise sintatica: %s\n", argv[1]);
        return 1;
    }
    
    lexer = init_lexer(file, argv[1]);
    global_lexer = lexer;
    
    current_token = get_next_token(lexer);
    has_syntax_errors = 0;
    
    Program();
    
    if (has_syntax_errors) {
        printf("\n\033[1;31mAnalise sintatica concluida com ERROS!\033[0m\n");
    } else {
        printf("\n\033[1;32mAnalise sintatica concluida com SUCESSO!\033[0m\n");
    }
    
    char syntax_filename[100];
    snprintf(syntax_filename, sizeof(syntax_filename), "%s.syntax", argv[1]);
    FILE* syntax_file = fopen(syntax_filename, "w");
    
    if (syntax_file) {
        int saved_stdout = dup(fileno(stdout));
        freopen(syntax_filename, "w", stdout);
        
        rewind(file);
        Lexer* syntax_lexer = init_lexer(file, argv[1]);
        global_lexer = syntax_lexer;
        current_token = get_next_token(syntax_lexer);
        has_syntax_errors = 0;
        
        printf("=== SEQUENCIA DE REGRAS DE PRODUCAO ===\n");
        Program();
        
        fflush(stdout);
        dup2(saved_stdout, fileno(stdout));
        close(saved_stdout);
        
        fclose(syntax_file);
        free_lexer(syntax_lexer);
        
        printf("\n\033[1;35mRegras de producao salvas em:\033[0m %s\n", syntax_filename);
    }
    
    free_lexer(lexer);
    return has_syntax_errors || has_lexical_errors;
}