#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif
#include "scanner.h"

typedef struct
{
    Token previous;
    Token current;
    bool hadError;
    bool panicMode;
} Parser;

typedef void (*ParseFn)(bool canAssign);

typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // . ()
    PREC_PRIMARY
} Precedence;

typedef struct
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct
{
    /**
     * Local variable name
     */
    Token name;
    /**
     * The scope depth of the block where the local variable was declared.
     * `-1` is uninitialized variable
     */
    int depth;
} Local;

typedef struct
{
    Local locals[UINT8_COUNT];
    /**
     * Tracking how many locals are in scope—how many of those array slots are in use.
     */
    int localCount;
    /**
     * This is the number of blocks surrounding the current bit of code we’re compiling.
     * Zero is the global scope, one is the first top-level block, two is inside that.
     */
    int scopeDepth;
} Compiler;

Parser parser;
Compiler *current = NULL;
Chunk *compileChunk;

static void advance();
static void consume(TokenType type, const char *message);
static bool match(TokenType type);
static bool check(TokenType type);
static void emitByte(uint8_t byte);
static void emitBytes(uint8_t byte1, uint8_t byte2);
static void endCompiler();
static void beginScope();
static void endScope();
static void declaration();
static void varDeclaration();
static void statement();
static void printStatement();
static void expressionStatement();
static void binary(bool canAssign);
static void literal(bool canAssign);
static void grouping(bool canAssign);
static void unary(bool canAssign);
static void parsePrecedence(Precedence precedence);
static uint8_t parseVariable(const char *errorMessage);
static void defineVariable(uint8_t global);
static uint8_t identifierConstant(Token *name);
static bool identifiersEqual(Token *a, Token *b);
static int resolveLocal(Compiler *compiler, Token *name);
static void declareVariable();
static void addLocal(Token name);
static void markInitialized();
static ParseRule *getRule(TokenType type);
static void number(bool canAssign);
static void string(bool canAssign);
static void namedVariable(Token name, bool canAssign);
static void variable(bool canAssign);
static void expression();
static void block();
static void emitReturn();
static uint8_t makeConstant(Value value);
static void emitConstant(Value value);
static void initCompiler(Compiler *compiler);
static Chunk *currentChunk();
static void errorAtCurrent(const char *message);
static void error(const char *message);
static void errorAt(Token *token, const char *message);
static void synchronize();

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, NULL, PREC_NONE},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS] = {unary, binary, PREC_TERM},
    [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
    [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
    [TOKEN_BANG] = {unary, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_GREATER] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER] = {variable, NULL, PREC_NONE},
    [TOKEN_STRING] = {string, NULL, PREC_NONE},
    [TOKEN_NUMBER] = {number, NULL, PREC_NONE},
    [TOKEN_AND] = {NULL, NULL, PREC_NONE},
    [TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = {literal, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_FUN] = {NULL, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL] = {literal, NULL, PREC_NONE},
    [TOKEN_OR] = {NULL, NULL, PREC_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER] = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS] = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE] = {literal, NULL, PREC_NONE},
    [TOKEN_VAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

/**
 * A single-pass compiler that translates source directly into bytecode, does not build AST.
 * Syntax analyzing uses **Pratt parsing algorithm**.
 */
bool compile(const char *source, Chunk *chunk)
{
    initScanner(source);
    Compiler compiler;
    initCompiler(&compiler);
    compileChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    advance();

    while (!match(TOKEN_EOF))
    {
        declaration();
    }

    consume(TOKEN_EOF, "Expect end of expression.");
    endCompiler();
    return !parser.hadError;
}

static void advance()
{
    parser.previous = parser.current;

    for (;;)
    {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR)
        {
            break;
        }

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char *message)
{
    if (parser.current.type == type)
    {
        advance();
        return;
    }

    errorAtCurrent(message);
}

/**
 * Advance if match
 */
static bool match(TokenType type)
{
    if (!check(type))
    {
        return false;
    }

    advance();
    return true;
}

static bool check(TokenType type)
{
    return parser.current.type == type;
}

static void emitByte(uint8_t byte)
{
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

static void endCompiler()
{
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
    {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}

static void beginScope()
{
    current->scopeDepth++;
}

static void endScope()
{
    current->scopeDepth--;

    // Pop all local variable of closed scope
    while (
        current->localCount > 0 &&
        current->locals[current->localCount - 1].depth > current->scopeDepth)
    {
        emitByte(OP_POP);
        current->localCount--;
    }
}

static void declaration()
{
    if (match(TOKEN_VAR))
    {
        varDeclaration();
    }
    else
    {
        statement();
    }

    if (parser.panicMode)
    {
        synchronize(); // error recovery for parser
    }
}

static void varDeclaration()
{
    uint8_t global = parseVariable("Expect variable name");

    if (match(TOKEN_EQUAL))
    {
        expression();
    }
    else
    {
        emitByte(OP_NIL);
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration");

    defineVariable(global);
}

static void statement()
{
    if (match(TOKEN_PRINT))
    {
        printStatement();
    }
    else if (match(TOKEN_LEFT_BRACE))
    {
        beginScope();
        block();
        endScope();
    }
    else
    {
        expressionStatement();
    }
}

static void printStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value");
    emitByte(OP_PRINT);
}

static void expressionStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

static void expression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

static void block()
{
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
    {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void binary(bool canAssign)
{
    TokenType operatorType = parser.previous.type;
    ParseRule *rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType)
    {
    case TOKEN_BANG_EQUAL:
    {
        emitBytes(OP_EQUAL, OP_NOT);
        break;
    }
    case TOKEN_EQUAL_EQUAL:
    {
        emitByte(OP_EQUAL);
        break;
    }
    case TOKEN_GREATER:
    {
        emitByte(OP_GREATER);
        break;
    }
    case TOKEN_GREATER_EQUAL:
    {
        emitBytes(OP_LESS, OP_NOT);
        break;
    }
    case TOKEN_LESS:
    {
        emitByte(OP_LESS);
        break;
    }
    case TOKEN_LESS_EQUAL:
    {
        emitBytes(OP_GREATER, OP_NOT);
        break;
    }
    case TOKEN_PLUS:
    {
        emitByte(OP_ADD);
        break;
    }
    case TOKEN_MINUS:
    {
        emitByte(OP_SUBTRACT);
        break;
    }
    case TOKEN_STAR:
    {
        emitByte(OP_MULTIPLY);
        break;
    }
    case TOKEN_SLASH:
    {
        emitByte(OP_DIVIDE);
        break;
    }
    default:
    {
        return;
    }
    }
}

static void literal(bool canAssign)
{
    switch (parser.previous.type)
    {
    case TOKEN_FALSE:
    {
        emitByte(OP_FALSE);
        break;
    }
    case TOKEN_NIL:
    {
        emitByte(OP_NIL);
        break;
    }
    case TOKEN_TRUE:
    {
        emitByte(OP_TRUE);
        break;
    }
    default:
    {
        return;
    }
    }
}

static void grouping(bool canAssign)
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(bool canAssign)
{
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void string(bool canAssign)
{
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign)
{
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name);
    if (arg != -1) // local variable
    {
        setOp = OP_SET_LOCAL;
        getOp = OP_GET_LOCAL;
    }
    else // global variable
    {
        arg = identifierConstant(&name);
        setOp = OP_SET_GLOBAL;
        getOp = OP_GET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL))
    {
        expression();
        emitBytes(setOp, (uint8_t)arg);
    }
    else
    {
        emitBytes(getOp, (uint8_t)arg);
    }
}

static void variable(bool canAssign)
{
    namedVariable(parser.previous, canAssign);
}

static void unary(bool canAssign)
{
    TokenType operatorType = parser.previous.type;

    // Compile the operand
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction
    switch (operatorType)
    {
    case TOKEN_BANG:
    {
        emitByte(OP_NOT);
        break;
    }
    case TOKEN_MINUS:
    {
        emitByte(OP_NEGATE);
        break;
    }
    default:
    {
        return;
    }
    }
}

static void defineVariable(uint8_t global)
{
    if (current->scopeDepth > 0)
    {
        markInitialized();
        return;
    }

    emitBytes(OP_DEFINE_GLOBAL, global);
}

static uint8_t parseVariable(const char *errorMessage)
{
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();
    if (current->scopeDepth > 0)
    {
        return 0;
    }

    return identifierConstant(&parser.previous); // save identifier name in constant table and refer to the name by its index in the table
}

static uint8_t identifierConstant(Token *name)
{
    return makeConstant(OBJ_VAL(copyString(name->start, name->length))); // save identifier name in constant table and refer to the name by its index in the table
}

static bool identifiersEqual(Token *a, Token *b)
{
    if (a->length != b->length)
    {
        return false;
    }

    return memcpy(a->start, b->start, a->length) == 0;
}

/**
 * Resolve slot index of local variable in stack
 */
static int resolveLocal(Compiler *compiler, Token *name)
{
    /**
     * @details
     * Whenever a variable is declared, we append it to the locals array in Compiler.
     * That means the first local variable is at index zero, the next one is at index one, and so on.
     * In other words, the locals array in the compiler has the exact same layout as the VM’s stack will have at runtime.
     * The variable’s index in the locals array is the same as its stack slot.
     */

    for (int i = compiler->localCount - 1; i >= 0; i--)
    {
        Local *local = &(compiler->locals[i]);
        if (identifiersEqual(name, &(local->name)))
        {
            if (local->depth == -1)
            {
                error("Can't read local variable in its own initializer.");
            }

            return i; // The local variable’s index in the locals array is the same as its stack slot.
        }
    }

    return -1; // not found local variable
}

static void declareVariable()
{
    if (current->scopeDepth == 0)
    {
        return;
    }

    Token *name = &parser.previous;
    for (int i = current->localCount - 1; i >= 0; i--)
    {
        Local *local = &(current->locals[i]);
        if (local->depth != -1 && local->depth < current->scopeDepth)
        {
            break;
        }

        if (identifiersEqual(name, &local->name))
        {
            error("Already a variable with this name in this scope.");
        }
    }

    addLocal(*name);
}

static void addLocal(Token name)
{
    if (current->localCount >= UINT8_COUNT)
    {
        error("Too many local variables in function.");
        return;
    }

    Local *local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
}

/**
 * Mark latest local variable of current scope as initialized.
 */
static void markInitialized()
{
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

/**
 * Starts at the current token and parses any expression at the given precedence level or higher.
 *
 * @details Pratt parsing algorithm
 */
static void parsePrecedence(Precedence precedence)
{
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign); // parse the left side of binary operator

    while (precedence <= getRule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign); // parse the right side of binary operator
    }

    if (canAssign && match(TOKEN_EQUAL))
    {
        error("Invalid assignment target.");
    }
}

static ParseRule *getRule(TokenType type)
{
    return &rules[type];
}

static void emitReturn()
{
    emitByte(OP_RETURN);
}

static void emitConstant(Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void initCompiler(Compiler *compiler)
{
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    current = compiler;
}

static uint8_t makeConstant(Value value)
{
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) /** OP_CONSTANT instruction uses a single byte for the index operand, we can store and load only up to 256 constants in a chunk. */
    {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

static Chunk *currentChunk()
{
    return compileChunk;
}

static void errorAtCurrent(const char *message)
{
    errorAt(&parser.current, message);
}

static void error(const char *message)
{
    errorAt(&parser.previous, message);
}

static void errorAt(Token *token, const char *message)
{
    if (parser.panicMode)
    {
        return;
    }

    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR)
    {
        // Nothing.
    }
    else
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void synchronize()
{
    parser.panicMode = false;

    while (TOKEN_EOF != parser.current.type)
    {
        if (TOKEN_SEMICOLON == parser.current.type)
        {
            return;
        }

        switch (parser.current.type)
        {
        case TOKEN_CLASS:
        case TOKEN_FUN:
        case TOKEN_VAR:
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_PRINT:
        case TOKEN_RETURN:
        {
            return;
        }
        default:
        { // Do nothing
        }
        }

        advance();
    }
}