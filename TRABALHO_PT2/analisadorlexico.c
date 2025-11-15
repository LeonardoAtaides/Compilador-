// Como compilar e executar:
// cd TRABALHO
// gcc analisadorlexico.c -o analisadorlexico
// .\analisadorlexico.exe testecerto.1
// .\analisadorlexico.exe testeerrado.1

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

// FUNÇÕES LÉXICAS

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

// VARIÁVEIS GLOBAIS PARA O PARSER
Lexer* global_lexer = NULL;
Token current_token;
int has_syntax_errors = 0;

// FUNÇÕES DO PARSER
void CasaToken(TokenType tipo_esperado);
void erro_sintatico(const char* mensagem);

void Programa();
void Bloco();
void ParteDeclaracoesVariaveis();
void DeclaracaoVariaveis();
void ListaIdentificadores();
void Tipo();
void ComandoComposto();
void Comando();
void Atribuicao();
void ComandoCondicional();
void ComandoRepetitivo();
void Expressao();
void Relacao();
void ExpressaoSimples();
void Termo();
void Fator();
void Variavel();

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

// ============ IMPLEMENTAÇÃO DO PARSER ============

void erro_sintatico(const char* mensagem) {
    if (current_token.type == TOK_EOF) {
        printf("\033[1;31mERRO SINTÁTICO (Linha %d): fim de arquivo não esperado.\033[0m\n", 
               current_token.line);
    } else {
        printf("\033[1;31mERRO SINTÁTICO (Linha %d): %s [%s].\033[0m\n", 
               current_token.line, mensagem, current_token.lexeme);
    }
    has_syntax_errors = 1;
}

void CasaToken(TokenType tipo_esperado) {
    if (current_token.type == tipo_esperado) {
        current_token = get_next_token(global_lexer);
    } else {
        erro_sintatico("token não esperado");
        // Tenta recuperar avançando para o próximo token
        if (current_token.type != TOK_EOF && current_token.type != TOK_ERROR) {
            current_token = get_next_token(global_lexer);
        }
    }
}

void Programa() {
    printf("Analisando: programa\n");
    CasaToken(TOK_PROGRAM);
    if (has_syntax_errors) return;
    CasaToken(ID);
    if (has_syntax_errors) return;
    CasaToken(SMB_SEM);
    if (has_syntax_errors) return;
    Bloco();
    if (has_syntax_errors) return;
    CasaToken(SMB_DOT);
    if (!has_syntax_errors) {
        printf("Programa analisado com sucesso!\n");
    }
}

void Bloco() {
    if (has_syntax_errors) return;
    printf("Analisando: bloco\n");
    ParteDeclaracoesVariaveis();
    if (has_syntax_errors) return;
    ComandoComposto();
}

void ParteDeclaracoesVariaveis() {
    if (has_syntax_errors) return;
    printf("Analisando: parte de declarações de variáveis\n");
    if (current_token.type == TOK_VAR) {
        CasaToken(TOK_VAR);
        if (has_syntax_errors) return;
        DeclaracaoVariaveis();
        while (current_token.type == SMB_SEM && !has_syntax_errors) {
            CasaToken(SMB_SEM);
            if (has_syntax_errors) break;
            if (current_token.type == TOK_BEGIN || current_token.type == TOK_EOF) break;
            DeclaracaoVariaveis();
        }
    }
}

void DeclaracaoVariaveis() {
    if (has_syntax_errors) return;
    printf("Analisando: declaração de variáveis\n");
    ListaIdentificadores();
    if (has_syntax_errors) return;
    CasaToken(SMB_COLON);
    if (has_syntax_errors) return;
    Tipo();
}

void ListaIdentificadores() {
    if (has_syntax_errors) return;
    printf("Analisando: lista de identificadores\n");
    CasaToken(ID);
    while (current_token.type == SMB_COM && !has_syntax_errors) {
        CasaToken(SMB_COM);
        if (has_syntax_errors) break;
        CasaToken(ID);
    }
}

void Tipo() {
    if (has_syntax_errors) return;
    printf("Analisando: tipo\n");
    if (current_token.type == TOK_INTEGER) {
        CasaToken(TOK_INTEGER);
    } else if (current_token.type == TOK_REAL) {
        CasaToken(TOK_REAL);
    } else {
        erro_sintatico("tipo esperado (integer ou real)");
    }
}

void ComandoComposto() {
    if (has_syntax_errors) return;
    printf("Analisando: comando composto\n");
    CasaToken(TOK_BEGIN);
    if (has_syntax_errors) return;
    
    // Verificação adicional para evitar loop infinito
    if (current_token.type == TOK_EOF) {
        erro_sintatico("comando esperado após begin");
        return;
    }
    
    Comando();
    if (has_syntax_errors) return;
    CasaToken(SMB_SEM);
    if (has_syntax_errors) return;
    
    // Loop modificado para evitar infinito
    while (current_token.type != TOK_END && current_token.type != TOK_EOF && !has_syntax_errors) {
        Comando();
        if (has_syntax_errors) break;
        if (current_token.type == TOK_END || current_token.type == TOK_EOF) break;
        CasaToken(SMB_SEM);
        if (has_syntax_errors) break;
    }
    
    if (!has_syntax_errors) {
        CasaToken(TOK_END);
    }
}

void Comando() {
    if (has_syntax_errors || current_token.type == TOK_EOF) return;
    
    printf("Analisando: comando\n");
    
    // Verificação adicional para EOF
    if (current_token.type == TOK_EOF) {
        erro_sintatico("comando esperado");
        return;
    }
    
    if (current_token.type == ID) {
        Atribuicao();
    } else if (current_token.type == TOK_BEGIN) {
        ComandoComposto();
    } else if (current_token.type == TOK_IF) {
        ComandoCondicional();
    } else if (current_token.type == TOK_WHILE) {
        ComandoRepetitivo();
    } else {
        erro_sintatico("comando esperado");
        // Avança para tentar recuperar
        if (current_token.type != TOK_EOF && current_token.type != TOK_ERROR) {
            current_token = get_next_token(global_lexer);
        }
    }
}

void Atribuicao() {
    if (has_syntax_errors) return;
    printf("Analisando: atribuição\n");
    Variavel();
    if (has_syntax_errors) return;
    CasaToken(OP_ASS);
    if (has_syntax_errors) return;
    Expressao();
}

void ComandoCondicional() {
    if (has_syntax_errors) return;
    printf("Analisando: comando condicional\n");
    CasaToken(TOK_IF);
    if (has_syntax_errors) return;
    Expressao();
    if (has_syntax_errors) return;
    CasaToken(TOK_THEN);
    if (has_syntax_errors) return;
    Comando();
    if (current_token.type == TOK_ELSE && !has_syntax_errors) {
        CasaToken(TOK_ELSE);
        if (has_syntax_errors) return;
        Comando();
    }
}

void ComandoRepetitivo() {
    if (has_syntax_errors) return;
    printf("Analisando: comando repetitivo\n");
    CasaToken(TOK_WHILE);
    if (has_syntax_errors) return;
    Expressao();
    if (has_syntax_errors) return;
    CasaToken(TOK_DO);
    if (has_syntax_errors) return;
    Comando();
}

void Expressao() {
    if (has_syntax_errors) return;
    printf("Analisando: expressão\n");
    ExpressaoSimples();
    if (!has_syntax_errors && 
        (current_token.type == OP_EQ || current_token.type == OP_NE || 
         current_token.type == OP_LT || current_token.type == OP_LE ||
         current_token.type == OP_GT || current_token.type == OP_GE)) {
        Relacao();
        if (has_syntax_errors) return;
        ExpressaoSimples();
    }
}

void Relacao() {
    if (has_syntax_errors) return;
    printf("Analisando: relação\n");
    switch (current_token.type) {
        case OP_EQ: CasaToken(OP_EQ); break;
        case OP_NE: CasaToken(OP_NE); break;
        case OP_LT: CasaToken(OP_LT); break;
        case OP_LE: CasaToken(OP_LE); break;
        case OP_GT: CasaToken(OP_GT); break;
        case OP_GE: CasaToken(OP_GE); break;
        default: erro_sintatico("operador relacional esperado");
    }
}

void ExpressaoSimples() {
    if (has_syntax_errors) return;
    printf("Analisando: expressão simples\n");
    if (current_token.type == OP_AD || current_token.type == OP_MIN) {
        if (current_token.type == OP_AD) CasaToken(OP_AD);
        else CasaToken(OP_MIN);
    }
    if (has_syntax_errors) return;
    Termo();
    while (!has_syntax_errors && (current_token.type == OP_AD || current_token.type == OP_MIN)) {
        if (current_token.type == OP_AD) CasaToken(OP_AD);
        else CasaToken(OP_MIN);
        if (has_syntax_errors) break;
        Termo();
    }
}

void Termo() {
    if (has_syntax_errors) return;
    printf("Analisando: termo\n");
    Fator();
    while (!has_syntax_errors && (current_token.type == OP_MUL || current_token.type == OP_DIV)) {
        if (current_token.type == OP_MUL) CasaToken(OP_MUL);
        else CasaToken(OP_DIV);
        if (has_syntax_errors) break;
        Fator();
    }
}

void Fator() {
    if (has_syntax_errors) return;
    printf("Analisando: fator\n");
    if (current_token.type == ID) {
        Variavel();
    } else if (current_token.type == LIT_INT || current_token.type == LIT_REAL || current_token.type == LIT_REAL_EXP) {
        if (current_token.type == LIT_INT) CasaToken(LIT_INT);
        else if (current_token.type == LIT_REAL) CasaToken(LIT_REAL);
        else CasaToken(LIT_REAL_EXP);
    } else if (current_token.type == SMB_OPA) {
        CasaToken(SMB_OPA);
        if (has_syntax_errors) return;
        Expressao();
        if (has_syntax_errors) return;
        CasaToken(SMB_CPA);
    } else {
        erro_sintatico("fator esperado (variável, número ou expressão entre parênteses)");
    }
}

void Variavel() {
    if (has_syntax_errors) return;
    printf("Analisando: variável\n");
    CasaToken(ID);
}

// ============ FUNÇÃO PRINCIPAL ============

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
    global_lexer = lexer;
    
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
    int has_lexical_errors = 0;
    
    // Primeira passagem: análise léxica
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
        printf("\nAnálise léxica concluída com ERROS! Análise sintática não realizada.\n");
        free_lexer(lexer);
        return 1;
    }
    
    printf("\nAnálise léxica concluída com SUCESSO!\n");
    printf("Tokens salvos em: %s\n", output_filename);
    
    // Segunda passagem: análise sintática
    printf("\n=== INICIANDO ANÁLISE SINTÁTICA ===\n");
    
    // Reinicializa o lexer para a análise sintática
    fseek(file, 0, SEEK_SET);
    free_lexer(lexer);
    lexer = init_lexer(file);
    global_lexer = lexer;
    
    current_token = get_next_token(lexer);
    has_syntax_errors = 0;
    
    // Executa a análise sintática
    Programa();
    
    if (current_token.type != TOK_EOF && !has_syntax_errors) {
        erro_sintatico("fim de arquivo esperado");
    }
    
    if (has_syntax_errors) {
        printf("\n\033[1;31mAnálise sintática concluída com ERROS!\033[0m\n");
    } else {
        printf("\n\033[1;32mAnálise sintática concluída com SUCESSO!\033[0m\n");
    }
    
    free_lexer(lexer);
    return has_syntax_errors;
}