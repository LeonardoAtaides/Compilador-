// gcc teste.c -o analisadorlexico
// .\analisadorlexico.exe teste.arquivocerto
// .\analisadorlexico.exe teste.arquivoerrado

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_SYMBOLS 100
#define MAX_LEXEME 100

typedef enum {
    // Palavras reservadas
    TOK_PROGRAM, TOK_VAR, TOK_INTEGER, TOK_REAL, TOK_BEGIN, TOK_END,
    TOK_IF, TOK_THEN, TOK_ELSE, TOK_WHILE, TOK_DO,
    
    // Operadores
    OP_EQ, OP_GE, OP_MUL, OP_NE, OP_LE, OP_DIV, OP_GT, OP_AD, 
    OP_ASS, OP_LT, OP_MIN,
    
    // Símbolos
    SMB_OBC, SMB_COM, SMB_CBC, SMB_SEM, SMB_OPA, SMB_CPA,
    SMB_COLON, SMB_DOT,
    
    // Identificadores e
    ID, LIT_INT, LIT_REAL, LIT_REAL_EXP,TOK_STRING,
    
    // Fim de arquivo e erro
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
} Lexer;

// FUNÇÕES

const char* token_type_to_string(TokenType type);

void init_symbol_table(SymbolTable* table);
int insert_symbol(SymbolTable* table, const char* name, TokenType type);
Symbol* find_symbol(SymbolTable* table, const char* name);
void print_symbol_table(SymbolTable* table);

Lexer* init_lexer(FILE* file);
void free_lexer(Lexer* lexer);
Token get_next_token(Lexer* lexer);
void skip_whitespace(Lexer* lexer);
char peek_char(Lexer* lexer);
bool is_valid_operator_combination(char current, char next);
bool is_valid_single_char_operator(char c);
bool is_valid_operator_start(char c);
void handle_unclosed_comment(Lexer* lexer, Token* token);

void to_lower_case(char* str);

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
    printf("\n=== TABELA DE SÍMBOLOS ===\n");
    printf("%-20s %-15s\n", "Nome", "Tipo");
    printf("--------------------------------\n");
    
    for (int i = 0; i < table->count; i++) {
        printf("%-20s ", table->symbols[i].name);
        
        switch (table->symbols[i].type) {
            case TOK_PROGRAM: printf("%-15s", "PROGRAM"); break;
            case TOK_VAR: printf("%-15s", "VAR"); break;
            case TOK_INTEGER: printf("%-15s", "INTEGER"); break;
            case TOK_REAL: printf("%-15s", "REAL"); break;
            case TOK_BEGIN: printf("%-15s", "BEGIN"); break;
            case TOK_END: printf("%-15s", "END"); break;
            case TOK_IF: printf("%-15s", "IF"); break;
            case TOK_THEN: printf("%-15s", "THEN"); break;
            case TOK_ELSE: printf("%-15s", "ELSE"); break;
            case TOK_WHILE: printf("%-15s", "WHILE"); break;
            case TOK_DO: printf("%-15s", "DO"); break;
            case ID: printf("%-15s", "ID"); break;
            default: printf("%-15s", "UNKNOWN"); break;
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
    if (current == ':' && next == '=') return true;    // :=
    if (current == '<' && next == '=') return true;    // <=
    if (current == '<' && next == '>') return true;    // <>
    if (current == '>' && next == '=') return true;    // >=
    
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
        sprintf(token->lexeme, "Conteúdo entre { } não permitido (comentários não suportados)");
        lexer->current_char = fgetc(lexer->file);
        lexer->column++;
    } else {

        token->type = TOK_ERROR;
        sprintf(token->lexeme, "Comentário não fechado iniciado na linha %d, coluna %d", 
                start_line, start_column);
    }
}

Lexer* init_lexer(FILE* file) {
    Lexer* lexer = malloc(sizeof(Lexer));
    lexer->file = file;
    lexer->current_char = fgetc(file);
    lexer->line = 1;
    lexer->column = 1;
    init_symbol_table(&lexer->symbol_table);
    return lexer;
}

void free_lexer(Lexer* lexer) {
    fclose(lexer->file);
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
                sprintf(token.lexeme, "Operador inválido: '%c%c'", lexer->current_char, next_char);
                lexer->current_char = fgetc(lexer->file);
                lexer->column++;
            } else {
                sprintf(token.lexeme, "Operador inválido: '%c'", lexer->current_char);
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
            strcpy(token.lexeme, " O caracter \" não é permitido");
            while (lexer->current_char != EOF && lexer->current_char != '"' && lexer->current_char != '\n') {
                lexer->current_char = fgetc(lexer->file);
                lexer->column++;
            }
            if (lexer->current_char == '\n') {
                sprintf(token.lexeme, "String não fechada antes da quebra de linha");
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
                sprintf(token.lexeme, "String não fechada na linha %d, coluna %d", start_line, start_column);
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

// MAIN

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Uso: %s <arquivo.mpas>\n", argv[0]);
        return 1;
    }
    
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        printf("Erro ao abrir arquivo: %s\n", argv[1]);
        return 1;
    }
    
    Lexer* lexer = init_lexer(file);
    
    char output_filename[100];
    snprintf(output_filename, sizeof(output_filename), "%s.lex", argv[1]);
    FILE* output_file = fopen(output_filename, "w");
    
    if (!output_file) {
        printf("Erro ao criar arquivo de saída\n");
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
    int has_errors = 0;
    
    do {
        token = get_next_token(lexer);
        
        if (token.type == TOK_ERROR) {
            has_errors = 1;
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
    free_lexer(lexer);
    
    if (has_errors) {
        printf("\nAnálise léxica concluída com ERROS!\n");
        return 1;
    } else {
        printf("\nAnálise léxica concluída com SUCESSO!\n");
        printf("Tokens salvos em: %s\n", output_filename);
        return 0;
    }
}
