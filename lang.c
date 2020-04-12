/// NOTICE:
///
/// Please read README.md for an description of this source.
/// It was made in a short period of time and as such many
/// things were left out, left broken, or left undocumented.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAYCNT(X) (sizeof(X)/sizeof(X[0]))

typedef enum {
    AL_NONE,
    AL_NAME,
    AL_NAME_RESV,
    AL_NUMBER,
    AL_SPECIAL,
    AL_STRING,
} al_ttype_t;

typedef struct {
    size_t line;
    size_t offset;
    al_ttype_t type;
    union {
        char* str;
        uint64_t num;
    } data;
} al_token_t;

typedef enum {
    AL_AST_LET,
    AL_AST_TYPE,
    AL_AST_EXPR,
    AL_AST_NUM,
    AL_AST_STR,
    AL_AST_PROG,
    AL_AST_VAR,
    AL_AST_BLOCK,
    AL_AST_LIST,
    AL_AST_MAP,
    AL_AST_CALL,
    AL_AST_LEXPR,
    AL_AST_FUNCTION,
    AL_AST_RETURN
} al_asttype_t;

const char* al_asttype_str[] = {
    "LET",
    "TYPE",
    "EXPR",
    "NUM",
    "STR",
    "PROG",
    "VAR",
    "BLOCK",
    "LIST",
    "MAP",
    "CALL",
    "LEXPR",
    "FUNCTION",
    "RETURN"
};

struct al_ast_s;
typedef struct al_ast_s al_ast_t;

const char* al_type_str[] = {
    "NUMBER",
    "STRING",
    "LIST",
    "MAP",
    "FUNCTION",
    "NONE"
};

typedef enum {
    AL_TYPE_NUMBER,
    AL_TYPE_STRING,
    AL_TYPE_LIST,
    AL_TYPE_MAP,
    AL_TYPE_FUNCTION,
    AL_TYPE_NONE
} al_funtype_t;

#define MAX_SUBTYPES 16

struct al_ast_type_s;
typedef struct al_ast_type_s al_ast_type_t;

struct al_ast_type_s {
    al_funtype_t type;
    al_ast_t** subtypes;
    size_t nsubtypes;
};

#define t_type data.type->type
#define t_subtypes data.type->subtypes
#define t_nsubtypes data.type->nsubtypes

typedef enum {
    AL_OPER_ADD,
    AL_OPER_SUB,
    AL_OPER_MUL,
    AL_OPER_DIV,
    AL_OPER_APPLY,
    AL_OPER_OR,
    AL_OPER_AND,
    AL_OPER_XOR,
    AL_OPER_NOT,
    AL_OPER_INV
} al_ast_oper_t;

const char* al_ast_oper_str[] = {
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "APPLY",
    "OR",
    "AND",
    "XOR",
    "NOT",
    "INV"
};

struct al_ast_expr_s;
typedef struct al_ast_expr_s al_ast_expr_t;

struct al_ast_expr_s {
    al_ast_oper_t oper;
    al_ast_t* lhs;
    al_ast_t* rhs;
};

#define e_oper data.expr->oper
#define e_lhs data.expr->lhs
#define e_rhs data.expr->rhs

typedef struct {
    al_ast_t** items;
    size_t nitems;
} al_ast_list_t;

#define li_items data.list->items
#define li_nitems data.list->nitems

typedef struct {
    al_ast_t** keys;
    al_ast_t** values;
    size_t nkeys;
} al_ast_map_t;

#define m_keys data.map->keys
#define m_values data.map->values
#define m_nkeys data.map->nkeys

typedef struct {
    al_ast_t* func;
    al_ast_t** args;
    size_t nargs;
} al_ast_call_t;

#define c_func data.call->func
#define c_args data.call->args
#define c_nargs data.call->nargs

typedef struct {
    char* name;
    al_ast_t* type;
    al_ast_t* value;
} al_ast_let_t;

#define l_name data.let->name
#define l_type data.let->type
#define l_value data.let->value

typedef struct {
    al_ast_t* lhs;
    al_ast_t* rhs;
} al_ast_lexpr_t;

#define le_lhs data.lexpr->lhs
#define le_rhs data.lexpr->rhs

typedef struct {
    char** args;
    al_ast_t** argtypes;
    size_t nargs;
    al_ast_t* ret;
    al_ast_t* block;
} al_ast_function_t;

#define fn_args data.func->args
#define fn_nargs data.func->nargs
#define fn_argtypes data.func->argtypes
#define fn_ret data.func->ret
#define fn_block data.func->block

typedef struct {
    al_ast_t* value;
} al_ast_return_t;

#define r_value data.ret->value

typedef struct {
    al_ast_t** children;
    size_t nchildren;
} al_ast_block_t;

#define b_children data.block->children
#define b_nchildren data.block->nchildren

struct al_ast_s {
    al_asttype_t type;
    union {
        al_ast_let_t* let;
        al_ast_expr_t* expr;
        al_ast_type_t* type;
        al_ast_block_t* block;
        al_ast_list_t* list;
        al_ast_map_t* map;
        al_ast_call_t* call;
        al_ast_lexpr_t* lexpr;
        al_ast_function_t* func;
        al_ast_return_t* ret;
        uint64_t num;
        char* str;
    } data;
};

const char* al_specials[] = {
    // 2len
    "=>", "->",
    "<=", ">=", "==", "!=",

    // 1len
    "+", "-", "/", "*",
    "[", "]", ":", "{", "}",
    "<", ">", "=", "!", "~",
    "(", ")", ";", "@", ","
};

const char* al_name_resv[] = {
    "number", "string", "let",
    "list", "map", "function",
    "return"
};

#define MAX_NAME 128
#define MAX_STR 1024
#define INRANGE(O, X, Y) (((O) >= (X)) && (O) <= (Y))

al_token_t* al_lex(char* contents, size_t len) {
    size_t token_length = 64;
    al_token_t* tokens = (al_token_t*) malloc(sizeof(al_token_t) * token_length);
    al_token_t* ctok = tokens;
    al_token_t* etok = tokens + token_length - 1;

    // 1 index
    size_t line = 1;
    size_t offset = 1;

    while ( len > 0 ) {
        if ( ctok == etok ) {
            size_t off = ctok - tokens;
            tokens = realloc(tokens, off * sizeof(al_token_t) * 2);
            assert(tokens != NULL);
            ctok = tokens + off;
            etok = tokens + (2*off);
        }

        ctok->type = AL_NONE;
        ctok->line = line;
        ctok->offset = offset;

        if ( *contents == '\n' || *contents == 0 ) {
            line++;
            offset = 1;
            contents++;
            len--;
            continue;
        }

        if ( *contents == ' ' || *contents == '\r' || *contents == '\t' ) {
            offset++;
            contents++;
            len--;
            continue;
        }

        if ( INRANGE(*contents, '0', '9') ) {
            ctok->data.num = 0;

            while ( INRANGE(*contents, '0', '9' ) && len > 0 ) {
                ctok->data.num *= 10;
                ctok->data.num += (uint64_t)(*contents - '0');
                contents++;
                len--;
                offset++;
            }

            ctok->type = AL_NUMBER;
            ctok++;
            continue;
        }

    #define ISNAME(X, I) \
        (INRANGE((X), 'a', 'z') || INRANGE((X), 'A', 'Z') || (X) == '_' || ((I != 0) && INRANGE((X), '0', '9')))

        // Parse name
        if ( ISNAME(*contents, 0) ) {
            ctok->type = AL_NAME;
            ctok->data.str = (char*) malloc((MAX_NAME + 1) * sizeof(char));
            char* cstr = ctok->data.str;
            char* estr = cstr + MAX_NAME;

            while ( ISNAME(*contents, 1) && len > 0 ) {
                if ( cstr == estr ) {
                    assert(0); // TODO: ERROR
                }

                *cstr = *contents;
                *(cstr + 1) = '\0';
                contents++;
                cstr++;
                len--;
                offset++;
            }

            size_t namelen = ((size_t) cstr - (size_t) ctok->data.str);

            for ( size_t i = 0; i < ARRAYCNT(al_name_resv); i++ ){
                size_t sl = strlen(al_name_resv[i]);

                if ( namelen < sl ) {
                    continue;
                }

                if ( strncmp(al_name_resv[i], ctok->data.str, sl) != 0 ) {
                    continue;
                }

                ctok->type = AL_NAME_RESV;
                break;
            }

            ctok++;
            continue;
        }

        // Parse special
        for ( size_t i = 0; i < ARRAYCNT(al_specials); i++ ) {
            size_t sl = strlen(al_specials[i]);

            if ( len < sl ) {
                continue;
            }

            if ( strncmp(al_specials[i], contents, sl) != 0 ){
                continue;
            }

            ctok->type = AL_SPECIAL;
            ctok->data.str = strdup(al_specials[i]);
            contents += sl;
            len -= sl;
            offset += sl;
            break;
        }

        if ( ctok->type == AL_SPECIAL ) {
            ctok++;
            continue;
        }

        // Parse strings
        if ( *contents == '"' ) {
            contents++;
            len--;
            ctok->type = AL_STRING;
            ctok->data.str = (char*) malloc((MAX_STR+1)*sizeof(char));
            char* cstr = ctok->data.str;
            char* estr = cstr + MAX_STR;

            *cstr = 0;

            while ( *contents != '"' && len > 1) {
                if ( cstr == estr ) {
                    assert(0); // TODO: error
                }

                if ( *contents == '\\' && len > 1 ) {
                    contents++;
                    len--;
                    offset++;

                    switch ( *contents ) {
                        case 'n': *cstr = '\n'; break;
                        case 'b': *cstr = '\b'; break;
                        case 'r': *cstr = '\r'; break;
                        default: {
                            printf("invalid control char \n");
                            assert(0);
                            break;
                        }
                    }
                }
                else {
                    *cstr = *contents;
                }

                *(cstr + 1) = 0;
                cstr++;
                contents++;
                len--;
                offset++;
            }

            contents++;
            offset++;
            len--;
            ctok++;
            continue;
        }

        fprintf(stderr, "unknown char: %u (%c)", *contents & 0xFF, *contents);
        assert(0); // TODO: error
    }

    #undef REFIT

    return tokens;
}

void al_print_token(al_token_t* token) {
    switch ( token->type ) {
        case AL_SPECIAL: printf("<SPECIAL: %s ", token->data.str); break;
        case AL_NUMBER: printf("<NUMBER: %llu ", token->data.num); break;
        case AL_NAME_RESV: printf("<NAME_RESV: %s ", token->data.str); break;
        case AL_NAME: printf("<NAME: %s ", token->data.str); break;
        case AL_STRING: printf("<STRING: \"%s\" ", token->data.str); break;
        default: printf("<UNKNOWN: %d ", token->type);
    }

    printf("ON %zu:%zu>", token->line, token->offset);

    printf("\n");
}

void al_print_tokens(al_token_t* tokens) {
    while ( tokens->type != AL_NONE ) {
        al_print_token(tokens);
        tokens++;
    }
}

#define ADV(T, X)         \
    (                     \
        (X)               \
            ? (((*T)++), 1) \
            : (0)         \
    )

#define TKN_TYPECMP(T, X) \
    ((*T)->type == X) \

#define TKN_NONE(T) \
    TKN_TYPECMP(T, AL_NONE)

#define TKN_STRCMP(T, X) \
    (strcmp((*T)->data.str, X) == 0)

#define TKN_TYPESTR(T, X, Y) \
    (TKN_TYPECMP((T), (X)) && (TKN_STRCMP((T), (Y))))

#define ALOBJ(X) ((X*) malloc(sizeof(X)))

al_ast_t* al_parse_type(al_token_t** tokens) {
    if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, ":")) ) {
        return NULL;
    }

    al_token_t* nametk = *tokens;

    if ( !ADV(tokens, TKN_TYPECMP(tokens, AL_NAME_RESV)) ) {
        assert(0); // TODO: errors
    }

    al_ast_t* ast = ALOBJ(al_ast_t);
    ast->type = AL_AST_TYPE;
    ast->data.type = ALOBJ(al_ast_type_t);

    #define ALTYPE(X, T, Y) \
        if ( TKN_STRCMP(X, T) ) { ast->data.type->type = (Y); }

    ast->t_type = AL_TYPE_NONE;
    ALTYPE(&nametk, "string", AL_TYPE_STRING);
    ALTYPE(&nametk, "number", AL_TYPE_NUMBER);
    ALTYPE(&nametk, "list", AL_TYPE_LIST);
    ALTYPE(&nametk, "map", AL_TYPE_MAP);
    ALTYPE(&nametk, "function", AL_TYPE_FUNCTION);

    if ( ast->data.type->type == AL_TYPE_NONE ) {
        assert(0); // TODO: error
    }

    if ( ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "[")) ) {
        ast->t_nsubtypes = MAX_SUBTYPES;
        ast->t_subtypes = (al_ast_t**) malloc(sizeof(al_ast_t**) * ast->t_nsubtypes);
        al_ast_t** csub = ast->t_subtypes;
        al_ast_t** esub = csub + ast->t_nsubtypes - 1;

        while ( !TKN_TYPESTR(tokens, AL_SPECIAL, "]") ) {
            if ( csub == esub ) {
                assert(0); //TODO: error
            }

            al_ast_t* subtype = al_parse_type(tokens);

            if ( subtype == NULL ) {
                break;
            }

            *csub = subtype;
            csub++;

            if ( ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, ",")) ) {
                continue;
            }
            else {
                break;
            }
        }

        if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "]")) ) {
            assert(0); // TODO: error
        }

        ast->t_nsubtypes = ((size_t) csub - (size_t) ast->t_subtypes) / sizeof(al_ast_t*);
    }
    else {
        ast->t_nsubtypes = 0;
    }

    return ast;
}

al_ast_t* al_parse_expr(al_token_t** tokens);

al_ast_t* al_parse_block(al_token_t** tokens, int braces);

// note: theres no actual al_ast_term_t for laziness
al_ast_t* al_parse_term(al_token_t** tokens) {
    al_token_t* first = *tokens;

    if ( ADV(tokens, TKN_TYPECMP(tokens, AL_NUMBER)) ) {
        al_ast_t* ast = ALOBJ(al_ast_t);
        ast->type = AL_AST_NUM;
        ast->data.num = first->data.num;
        return ast;
    }

    if ( ADV(tokens, TKN_TYPECMP(tokens, AL_STRING)) ) {
        al_ast_t* ast = ALOBJ(al_ast_t);
        ast->type = AL_AST_STR;
        ast->data.str = first->data.str;
        return ast;
    }

     if ( ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "(")) ) {
        al_ast_t* subexpr = al_parse_expr(tokens);

        if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, ")")) ) {
            assert(0); // TODO: error
        }

        // TODO: wrap in another ast?
        return subexpr;
    }

    if ( ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "[")) ) {
        al_ast_t* ast = ALOBJ(al_ast_t);
        ast->type = AL_AST_LIST;
        ast->data.list = ALOBJ(al_ast_list_t);
        
        ast->li_nitems = 64;
        ast->li_items = (al_ast_t**) malloc(sizeof(al_ast_t**) * ast->li_nitems);

        al_ast_t** citem = ast->li_items;
        al_ast_t** eitem = citem + ast->li_nitems - 1;

        while ( !TKN_TYPESTR(tokens, AL_SPECIAL, "]") ) {
            if ( citem == eitem ) {
                size_t off = citem - ast->li_items;
                ast->li_items = realloc(ast->li_items, off * sizeof(al_ast_t*) * 2);
                assert(ast->li_items != NULL);
                citem = ast->li_items + off;
                eitem = ast->li_items + (2*off);
            }

            al_ast_t* item = al_parse_expr(tokens);

            if ( item == NULL ) {
                break;
            }

            *citem = item;
            citem++;

            if ( ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, ",")) ) {
                continue;
            }
            else {
                break;
            }
        }

        if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "]")) ) {
            assert(0); // TODO: error
        }

        ast->li_nitems = ((size_t) citem - (size_t) ast->li_items) / sizeof(al_ast_t*);
        return ast;
    }

    if ( ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "{")) ) {
        al_ast_t* ast = ALOBJ(al_ast_t);
        ast->type = AL_AST_MAP;
        ast->data.map = ALOBJ(al_ast_map_t);
        
        ast->m_nkeys = 64;
        ast->m_keys = (al_ast_t**) malloc(sizeof(al_ast_t*) * ast->m_nkeys);
        ast->m_values = (al_ast_t**) malloc(sizeof(al_ast_t*) * ast->m_nkeys);

        al_ast_t** ckey = ast->m_keys;
        al_ast_t** cval = ast->m_values;
        al_ast_t** ekey = ckey + ast->m_nkeys - 1;

        while ( !TKN_TYPESTR(tokens, AL_SPECIAL, "]") ) {
            if ( ckey == ekey ) {
                size_t off = ckey - ast->m_keys;
                ast->m_keys = realloc(ast->m_keys, off * sizeof(al_ast_t*) * 2);
                ast->m_values = realloc(ast->m_values, off * sizeof(al_ast_t*) * 2);
                assert(ast->m_keys != NULL);
                assert(ast->m_values != NULL);
                ckey = ast->m_keys + off;
                cval = ast->m_values + off;
                ekey = ast->m_keys + (2*off);
            }

            al_ast_t* key = al_parse_expr(tokens);

            if ( key == NULL ) {
                break;
            }

            *ckey = key;
            ckey++;

            if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "=>")) ) {
                assert(0); // TODO: error
            }

            al_ast_t* val = al_parse_expr(tokens);

            if ( val == NULL ) {
                break;
            }

            *cval = val;
            cval++;

            if ( ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, ",")) ) {
                continue;
            }
            else {
                break;
            }
        }

        if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "}")) ) {
            printf("failed to find } found "); al_print_token(*tokens);
            assert(0); // TODO: error
        }

        ast->m_nkeys = ((size_t) ckey - (size_t) ast->m_keys) / sizeof(al_ast_t*);
        return ast;
    }

    if ( ADV(tokens, TKN_TYPESTR(tokens, AL_NAME_RESV, "function")) ) {
        al_ast_t* ast = ALOBJ(al_ast_t);
        ast->type = AL_AST_FUNCTION;
        ast->data.func = ALOBJ(al_ast_function_t);
        
        ast->fn_nargs = 64;
        ast->fn_args = (char**) malloc(sizeof(char**) * ast->fn_nargs);
        ast->fn_argtypes = (al_ast_t**) malloc(sizeof(al_ast_t**) * ast->fn_nargs);

        char** carg = ast->fn_args;
        al_ast_t** cargtype = ast->fn_argtypes;
        char** earg = carg + ast->fn_nargs - 1;

        if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "(")) ) {
            assert(0); // TODO: error
        }

        while ( !TKN_TYPESTR(tokens, AL_SPECIAL, ")") ) {
            if ( carg == earg ) {
                size_t off = carg - ast->fn_args;
                ast->fn_args = realloc(ast->fn_args, off * sizeof(char*) * 2);
                ast->fn_argtypes = realloc(ast->fn_argtypes, off * sizeof(al_ast_t*) * 2);
                assert(ast->fn_args != NULL);
                carg = ast->fn_args + off;
                earg = ast->fn_args + (2*off);
            }

            al_token_t* curtok = *tokens;

            if ( !ADV(tokens, TKN_TYPECMP(tokens, AL_NAME)) ) {
                assert(0); // TODO: error
            }

            *carg = curtok->data.str;
            carg++;

            al_ast_t* type = al_parse_type(tokens);

            if ( type == NULL ) {
                assert(0); // TODO: er ror
            }

            *cargtype = type;
            cargtype++;

            if ( ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, ",")) ) {
                continue;
            }
            else {
                break;
            }
        }

        if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, ")")) ) {
            assert(0); // TODO: error
        }

        ast->fn_nargs = ((size_t) carg - (size_t) ast->fn_args) / sizeof(char*);

        ast->fn_ret = al_parse_type(tokens);

        if ( ast->fn_ret == NULL ) {
            assert(0); // TOODO: errors
        }

        ast->fn_block = al_parse_block(tokens, 1);

        return ast;
    }

    if ( ADV(tokens, TKN_TYPECMP(tokens, AL_NAME)) ) { 
        al_ast_t* ast = ALOBJ(al_ast_t);
        ast->type = AL_AST_VAR;
        ast->data.str = first->data.str;
        return ast;
    }

    return NULL;
}

al_ast_t* al_parse_postfix(al_token_t** tokens) {
    al_ast_t* term = al_parse_term(tokens);

    if ( term == NULL ) {
        return NULL;
    }

    if ( ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "(")) ) {
        al_ast_t* ast = ALOBJ(al_ast_t);
        ast->type = AL_AST_CALL;
        ast->data.call = ALOBJ(al_ast_call_t);
        
        ast->c_func = term;
        ast->c_nargs = 64;
        ast->c_args = (al_ast_t**) malloc(sizeof(al_ast_t**) * ast->c_nargs);

        al_ast_t** carg = ast->c_args;
        al_ast_t** earg = carg + ast->c_nargs - 1;

        while ( !TKN_TYPESTR(tokens, AL_SPECIAL, ")") ) {
            if ( carg == earg ) {
                size_t off = carg - ast->c_args;
                ast->c_args = realloc(ast->c_args, off * sizeof(al_ast_t*) * 2);
                assert(ast->c_args != NULL);
                carg = ast->c_args + off;
                earg = ast->c_args + (2*off);
            }

            al_ast_t* arg = al_parse_expr(tokens);

            if ( arg == NULL ) {
                break;
            }

            *carg = arg;
            carg++;

            if ( ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, ",")) ) {
                continue;
            }
            else {
                break;
            }
        }

        if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, ")")) ) {
            assert(0); // TODO: error
        }

        ast->c_nargs = ((size_t) carg - (size_t) ast->c_args) / sizeof(al_ast_t*);
        return ast;
    }

    return term;
}

const char* prefix_op_str[] = { "!", "~" };
al_ast_oper_t prefix_op[] = { AL_OPER_NOT, AL_OPER_INV };

const char* infix_op_str[] = { "@", "+", "-", "*", "/" };
al_ast_oper_t infox_op[] = { AL_OPER_APPLY, AL_OPER_ADD, AL_OPER_SUB, AL_OPER_MUL, AL_OPER_DIV };

al_ast_t* al_parse_expr(al_token_t** tokens) {
    for ( int i = 0; i < ARRAYCNT(prefix_op_str); i++ ) {
        if ( ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, prefix_op_str[i])) ) {
            al_ast_t* inner = al_parse_expr(tokens);
            al_ast_t* ast = ALOBJ(al_ast_t);
            ast->type = AL_AST_EXPR;
            ast->data.expr = ALOBJ(al_ast_expr_t);
            ast->e_oper = prefix_op[i];
            ast->e_lhs = inner;
            ast->e_rhs = NULL;
            return ast;
        }
    }

    al_ast_t* lhs = al_parse_postfix(tokens);

    if ( lhs == NULL ) {
        return NULL;
    }

    for ( int i = 0; i < ARRAYCNT(infix_op_str); i++ ) {
        if ( ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, infix_op_str[i])) ) {
            al_ast_t* ast = ALOBJ(al_ast_t);
            ast->type = AL_AST_EXPR;
            ast->data.expr = ALOBJ(al_ast_expr_t);
            ast->e_oper = infox_op[i];
            ast->e_lhs = lhs;
            ast->e_rhs = al_parse_expr(tokens);

            if ( ast->e_rhs == NULL ) {
                assert(0); // TODO: error
            }

            return ast;
        }
    }

    return lhs;
}

al_ast_t* al_parse_let(al_token_t** tokens) {
    if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_NAME_RESV, "let")) ) {
        return NULL;
    }

    al_token_t* nametkn = *tokens;

    if ( !ADV(tokens, TKN_TYPECMP(tokens, AL_NAME)) ) {
        // TODO: error
        assert(0);
    }

    al_ast_t* ast = ALOBJ(al_ast_t);
    ast->type = AL_AST_LET;
    ast->data.let = ALOBJ(al_ast_let_t);

    ast->l_name = nametkn->data.str;
    ast->l_type = al_parse_type(tokens);

    if ( ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "=")) ) {
        ast->l_value = al_parse_expr(tokens);
    }
    else {
        ast->l_value = NULL;
    }

    if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, ";")) ) {
        printf("wanted ; found "); al_print_token(*tokens);
        assert(0); // TODO: error
    }

    return ast;
}

al_ast_t* al_parse_lexpr(al_token_t** tokens) {
    al_ast_t* term = al_parse_postfix(tokens);

    if ( term == NULL ) {
        return NULL;
    }

    al_ast_t* ast = ALOBJ(al_ast_t);
    ast->type = AL_AST_LEXPR;
    ast->data.lexpr = ALOBJ(al_ast_lexpr_t);
    ast->le_lhs = term;
    ast->le_rhs = NULL;

    if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "=")) ) {
        if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, ";")) ) {
            assert(0); // TODO: error
        }

        ast->le_rhs = ast->le_lhs;
        ast->le_lhs = NULL;

        return ast;
    }

    al_ast_t* value = al_parse_expr(tokens);

    if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, ";")) ) {
        assert(0); // TODO: error
    }

    ast->le_rhs = value;

    return ast;
}

al_ast_t* al_parse_ret(al_token_t** tokens) {
    if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_NAME_RESV, "return")) ) {
        return NULL;
    }

    al_ast_t* ast = ALOBJ(al_ast_t);
    ast->type = AL_AST_RETURN;
    ast->data.ret = ALOBJ(al_ast_return_t);
    ast->r_value = al_parse_expr(tokens);

    if ( !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, ";")) ) {
        assert(0); // TODO: error
    }

    return ast;
}

al_ast_t* al_parse_stmt(al_token_t** tokens) {
    al_ast_t* ast = NULL;

    if ( (ast = al_parse_let(tokens)) != NULL ) {
        return ast;
    }

    if ( (ast = al_parse_ret(tokens)) != NULL ) {
        return ast;
    }

    if ( (ast = al_parse_lexpr(tokens)) != NULL ) {
        return ast;
    }

    return NULL;
}

al_ast_t* al_parse_block(al_token_t** tokens, int braces) {
    if ( braces && !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "{")) ) {
        assert(0); // TODO: error
    }

    al_ast_t* ast = ALOBJ(al_ast_t);
    ast->type = AL_AST_BLOCK;
    ast->data.block = ALOBJ(al_ast_block_t);
    ast->b_nchildren = 64;
    ast->b_children = (al_ast_t**) malloc(sizeof(al_ast_t*) * ast->b_nchildren);
    al_ast_t** cchild = ast->b_children;
    al_ast_t** echild = cchild + ast->b_nchildren;

    al_ast_t* child = NULL;


    while ( (child = al_parse_stmt(tokens)) != NULL ) {
        if ( cchild == echild ) {
            size_t off = cchild - ast->b_children;
            ast->b_children = realloc(ast->b_children, off * sizeof(al_ast_t*) * 2);
            assert(ast->b_children != NULL);
            cchild = ast->b_children + off;
            echild = ast->b_children + (2*off);
        }

        *cchild = child;
        cchild++;
    }

    ast->b_nchildren = ((size_t) cchild - (size_t) ast->b_children) / sizeof(al_ast_t*);

    if ( braces && !ADV(tokens, TKN_TYPESTR(tokens, AL_SPECIAL, "}")) ) {
        printf("expected close block found: "); al_print_token(*tokens);
        assert(0); // TODO: error
    }

    return ast;
}

#define PRINTOFF(N) for(int __i = 0; __i < N*4; __i++) { printf(" "); }

void al_ast_print(al_ast_t* ast, size_t index) {
    PRINTOFF(index);

    if ( ast == NULL ) {
        printf("<NULL>\n");
        return;
    }

    switch ( ast->type ) {
        case AL_AST_LET:
            printf("LET %s\n", ast->l_name);
            al_ast_print(ast->l_type, index+1);

            if ( ast->l_value != NULL ) {
                al_ast_print(ast->l_value, index+1);                
            }

            break;
        case AL_AST_TYPE:
            printf("TYPE %s\n", al_type_str[ast->t_type]);

            for ( int i = 0; i < ast->t_nsubtypes; i++ ) {
                al_ast_print(ast->t_subtypes[i], index+1);
            }

            break;
        case AL_AST_BLOCK:
            printf("BLOCK (%zu)\n", ast->b_nchildren);

            for ( int i = 0; i < ast->b_nchildren; i++ ) {
                al_ast_print(ast->b_children[i], index+1);
            }

            break;
        case AL_AST_LIST:
            printf("LIST (%zu)\n", ast->li_nitems);

            for ( int i = 0; i < ast->li_nitems; i++ ) {
                al_ast_print(ast->li_items[i], index+1);
            }

            break;
        case AL_AST_MAP:
            printf("MAP (%zu)\n", ast->m_nkeys);

            for ( int i = 0; i < ast->m_nkeys; i++ ) {
                al_ast_print(ast->m_keys[i], index+1);
                al_ast_print(ast->m_values[i], index+1);
            }

            break;
        case AL_AST_LEXPR:
            printf("LEXPR\n");
            if ( ast->le_lhs != NULL ) al_ast_print(ast->le_lhs, index+1);
            if ( ast->le_rhs != NULL ) al_ast_print(ast->le_rhs, index+1);
            break;
        case AL_AST_EXPR:
            printf("EXPR %s\n", al_ast_oper_str[ast->e_oper]);

            if ( ast->e_lhs != NULL ) al_ast_print(ast->e_lhs, index+1);
            if ( ast->e_rhs != NULL ) al_ast_print(ast->e_rhs, index+1);

            break;
        case AL_AST_NUM:
            printf("NUM %llu\n", ast->data.num);
            break;
        case AL_AST_STR:
            printf("STR %s\n", ast->data.str);
            break;
        case AL_AST_PROG:
            printf("PROG\n");
            break;
        case AL_AST_VAR:
            printf("VAR %s\n", ast->data.str);
            break;
        case AL_AST_CALL:
            printf("CALL\n");
            al_ast_print(ast->c_func, index+1);

            for ( int i = 0; i < ast->c_nargs; i++ ) {
                al_ast_print(ast->c_args[i], index+1);
            }
            break;
        case AL_AST_RETURN:
            printf("RET\n");
            al_ast_print(ast->r_value, index+1);
            break;
        case AL_AST_FUNCTION:
            printf("FUNCTION\n");
            for ( int i = 0; i < ast->fn_nargs; i++ ) {
                PRINTOFF(index+1); printf("ARG %s\n", ast->fn_args[i]);
                al_ast_print(ast->fn_argtypes[i], index+1);
            }
            al_ast_print(ast->fn_ret, index + 1);
            al_ast_print(ast->fn_block, index + 1);
            break;
    }
}

typedef enum {
    AL_VAL_NUM,
    AL_VAL_STR,
    AL_VAL_LIST,
    AL_VAL_MAP,
    AL_VAL_FUNC
} al_value_type_t;

struct al_value_s;
typedef struct al_value_s al_value_t;

typedef struct {
    size_t length;
    size_t count;
    al_value_t** values;
} al_value_list_t;

#define vl_length data.list->length
#define vl_count data.list->count
#define vl_values data.list->values

typedef struct {
    size_t length;
    size_t count;
    al_value_t* keys;
    al_value_t* values;    
} al_value_map_t;

#define vm_length data.map->length
#define vm_count data.map->count
#define vm_keys data.map->keys
#define vm_values data.map->values

struct al_runtype_s;
typedef struct al_runtype_s al_runtype_t;

struct al_runtype_s {
    al_funtype_t type;
    al_runtype_t** subtypes;
    size_t nsubtypes;
};

struct al_namespace_s;
typedef struct al_namespace_s al_namespace_t;
typedef al_value_t* (al_builtin_t)(al_namespace_t* ns);

typedef struct {
    char** args;
    al_runtype_t** argtypes;
    size_t nargs;

    al_runtype_t* rettype;

    al_ast_t* body;
    al_builtin_t* builtin;
} al_value_func_t;

#define vf_args data.func->args
#define vf_nargs data.func->nargs
#define vf_rettype data.func->rettype
#define vf_argtypes data.func->argtypes
#define vf_body data.func->body
#define vf_builtin data.func->builtin

al_runtype_t* al_runtype_deriv(al_ast_t* type) {
    al_runtype_t* rt = ALOBJ(al_runtype_t);
    rt->type = type->t_type;

    switch ( rt->type ) {
        case AL_TYPE_NUMBER:
        case AL_TYPE_STRING:
        case AL_TYPE_NONE:
            if ( type->t_nsubtypes > 0 ) {
                printf("invalid subtypes");
                assert(0); // TODO error handling
            }

        case AL_TYPE_LIST:
        case AL_TYPE_MAP:
        case AL_TYPE_FUNCTION:
            rt->nsubtypes = type->t_nsubtypes;

            if ( rt->nsubtypes > 0 ) {
                rt->subtypes = (al_runtype_t**) malloc(sizeof(al_runtype_t*) * rt->nsubtypes);
            }

            for ( int i = 0; i < rt->nsubtypes; i++ ) {
                rt->subtypes[i] = al_runtype_deriv(type->t_subtypes[i]);
            }
    }

    return rt;
}

al_runtype_t* al_runtype_simple(al_funtype_t funtype) {
    al_runtype_t* rt = ALOBJ(al_runtype_t);
    rt->type = funtype;
    rt->nsubtypes = 0;
    return rt;
}

void al_runtype_set_subtypes(al_runtype_t* runtype, al_runtype_t** subtypes, size_t nsubtypes) {
    runtype->nsubtypes = nsubtypes;
    runtype->subtypes = (al_runtype_t**) malloc(sizeof(al_runtype_t*) * nsubtypes);

    for ( int i = 0; i < nsubtypes; i++ ) {
        runtype->subtypes[i] = subtypes[i];
    }
}

int al_exec_typecheck(al_runtype_t* a, al_runtype_t* b) {
    if ( a->type != b->type ) {
        return 0;
    }

    if ( a->nsubtypes != b->nsubtypes ) {
        return 0;
    }

    for ( int i = 0; i < a->nsubtypes; i++ ) {
        if ( !al_exec_typecheck(a->subtypes[i], b->subtypes[i]) ) {
            return 0;
        }
    }

    return 1;
}

struct al_value_s {
    al_runtype_t* type;

    union {
        uint64_t num;
        char* str;
        al_value_list_t* list;
        al_value_map_t* map;
        al_value_func_t* func;
    } data;
};

void al_value_print(al_value_t* value) {
    switch ( value->type->type ) {
        case AL_TYPE_NUMBER:
            printf("%llu", value->data.num);
            break;
        case AL_TYPE_STRING:
            printf("%s", value->data.str);
            break;
        case AL_TYPE_LIST:
            printf("[");
            for ( int i = 0; i < value->vl_length; i++ ) {
                al_value_print(value->vl_values[i]);
                if ( i < value->vl_length - 1) {
                    printf(", ");
                }
            }
            printf("]");
            break;
        case AL_TYPE_MAP:
            printf("TODO MAP");
            break;
        case AL_TYPE_FUNCTION:
            printf("TODO FUNC");
            break;
        case AL_TYPE_NONE:
            printf("NONE");
            break;
    }
}

al_value_t* al_value_list_empty() {
    al_value_t* value = ALOBJ(al_value_t);
    value->data.list = ALOBJ(al_value_list_t);    
    
    value->type = ALOBJ(al_runtype_t);
    value->type->type = AL_TYPE_LIST;
    value->type->nsubtypes = 0;

    value->vl_length = 0;
    value->vl_count = 64;
    value->vl_values = (al_value_t**) malloc(sizeof(al_value_t*) * value->vl_count);
    return value;
}

void al_value_list_check_valtype(al_value_t* list, al_value_t* value) {
    // Inherit type from value's type
    if ( list->type->nsubtypes == 0 ) {
        al_runtype_set_subtypes(list->type, &value->type, 1);
    }
    else {
        if ( list->type->nsubtypes != 1 ) {
            printf("list_append weird subtype");
            assert(0); // TODO: error
        }

        if ( !al_exec_typecheck(list->type->subtypes[0], value->type) ) {
            printf("list_append typecheck fail\n");
            assert(0); // TODO: error
        }
    }
}

void al_value_list_append(al_value_t* list, al_value_t* value) {
    assert(list->type->type == AL_TYPE_LIST);

    al_value_list_check_valtype(list, value);

    if ( list->vl_length == list->vl_count ) {
        list->vl_count *= 2;
        list->vl_values = (al_value_t**) realloc(list->vl_values, sizeof(al_value_t*) * list->vl_count);
        assert(list->vl_values != NULL);
    }

    list->vl_values[list->vl_length++] = value;
}

al_value_t* al_value_list_fromval(al_value_t** values, size_t count) {
    al_value_t* list = al_value_list_empty();

    for ( int i = 0; i < count; i++ ) {
        al_value_list_append(list, values[i]);
    }

    return list;
}

typedef struct {
    char* name;
    al_runtype_t* type;
    al_value_t* value;
} al_trip_t;

struct al_namespace_s {
    al_trip_t** trips;
    size_t atrips;
    size_t ntrips;

    al_namespace_t* parent;
};

typedef struct {
    al_value_t* (*op_default)();
    al_value_t* (*op_add)(al_value_t*, al_value_t*);
    al_value_t* (*op_sub)(al_value_t*, al_value_t*);
    al_value_t* (*op_mul)(al_value_t*, al_value_t*);
    al_value_t* (*op_div)(al_value_t*, al_value_t*);
} al_oper_dispatch_t;

static inline
al_value_t* al_num_fromval(uint64_t val) {
    al_value_t* r = ALOBJ(al_value_t);
    r->type = al_runtype_simple(AL_TYPE_NUMBER);
    r->data.num = val;
    return r;
}

al_value_t* al_num_default() {
    return al_num_fromval(0);
}

al_value_t* al_num_add(al_value_t* a, al_value_t* b) {
    return al_num_fromval(a->data.num + b->data.num);
}

al_value_t* al_num_sub(al_value_t* a, al_value_t* b) {
    return al_num_fromval(a->data.num - b->data.num);
}

al_value_t* al_num_mul(al_value_t* a, al_value_t* b) {
    return al_num_fromval(a->data.num * b->data.num);
}

al_value_t* al_num_div(al_value_t* a, al_value_t* b) {
    return al_num_fromval(a->data.num / b->data.num);
}

static inline
al_value_t* al_str_fromval(char* val) {
    al_value_t* r = ALOBJ(al_value_t);
    r->type = al_runtype_simple(AL_TYPE_STRING);
    r->data.str = val;
    return r;
}

al_value_t* al_str_default() {
    return al_str_fromval("");
}

al_value_t* al_str_add(al_value_t* a, al_value_t* b) {
    char* mem = (char*) malloc(sizeof(char) * (strlen(a->data.str) + strlen(b->data.str) + 1));
    *mem = 0;
    strcat(mem, a->data.str);
    strcat(mem, b->data.str);
    return al_str_fromval(mem);
}

al_value_t* al_list_add(al_value_t* a, al_value_t* b) {
    al_value_t* list = al_value_list_empty();

    if ( a->type->nsubtypes == 0 ) {
        if ( b->type->nsubtypes == 0 ) {
            return al_value_list_empty();
        }

        al_runtype_set_subtypes(list->type, b->type->subtypes, b->type->nsubtypes);
    }
    else if ( b->type->nsubtypes == 0 ) {
        al_runtype_set_subtypes(list->type, a->type->subtypes, a->type->nsubtypes);
    }
    else {
        if ( !al_exec_typecheck(a->type, b->type) ) {
            printf("failed typecheck list add\n");
            assert(0); // TODO: err
        }
    }

    for ( int i = 0; i < a->vl_length; i++ ) {
        al_value_list_append(list, a->vl_values[i]);
    }

    for ( int i = 0; i < b->vl_length; i++ ) {
        al_value_list_append(list, b->vl_values[i]);
    }

    return list;
}

al_oper_dispatch_t al_dispatches[] = {
    /* AL_TYPE_NUMBER */ {
        al_num_default, al_num_add, al_num_sub, al_num_mul, al_num_div
    },
    /* AL_TYPE_STRING */ {
        al_str_default, al_str_add, NULL, NULL, NULL
    },
    /* AL_TYPE_LIST */ {
        al_value_list_empty, al_list_add, NULL, NULL, NULL
    },
    /* AL_TYPE_MAP */ {
        NULL, NULL, NULL, NULL, NULL
    },
    /* AL_TYPE_FUNCTION */ {
        NULL, NULL, NULL, NULL, NULL
    },
    /* AL_TYPE_NONE */ {
        NULL, NULL, NULL, NULL, NULL
    }
};

al_trip_t* al_namespace_lookup(al_namespace_t* ns, char* var) {
    for ( int i = 0; i < ns->ntrips; i++ ) {
        if ( strcmp(var, ns->trips[i]->name) == 0 ) {
            return ns->trips[i];
        }
    }

    if ( ns->parent != NULL ) {
        return al_namespace_lookup(ns->parent, var);
    }

    return NULL;
}

void al_namespace_add(al_namespace_t* ns, char* var, al_runtype_t* type, al_value_t* value) {
    if ( ns->atrips == ns->ntrips ) {
        ns->atrips *= 2;
        ns->trips = (al_trip_t**) realloc(ns->trips, ns->atrips * sizeof(al_trip_t*));
        assert(ns->trips != NULL);
    }

    ns->trips[ns->ntrips] = ALOBJ(al_trip_t);
    ns->trips[ns->ntrips]->name = var;
    ns->trips[ns->ntrips]->type = type;
    ns->trips[ns->ntrips]->value = value;
    ns->ntrips++;
}

al_runtype_t* al_runtype_dup(al_runtype_t* runtype) {
    al_runtype_t* output = ALOBJ(al_runtype_t);
    output->type = runtype->type;
    output->nsubtypes = runtype->nsubtypes;

    if ( output->nsubtypes > 0 ) {
        output->subtypes = (al_runtype_t**) malloc(sizeof(al_runtype_t*) * output->nsubtypes);

        for ( int i = 0; i < output->nsubtypes; i++ ) {
            output->subtypes[i] = al_runtype_dup(runtype->subtypes[i]);
        }
    }
    else {
        output->subtypes = NULL;
    }

    return output;
} 

void al_namespace_register_builtin(
    al_namespace_t* ns, char* name, al_builtin_t* builtin,
    char* args[], al_runtype_t* argtypes[], size_t nargs,
    al_runtype_t* rettype
) {
    al_value_t* value = ALOBJ(al_value_t);
    value->data.func = ALOBJ(al_value_func_t);
    value->vf_nargs = nargs;
    value->vf_args = (char**) malloc(sizeof(char*) * nargs);
    value->vf_argtypes = (al_runtype_t**) malloc(sizeof(al_runtype_t*) * nargs);
    value->vf_rettype = rettype;
    value->vf_body = NULL;
    value->vf_builtin = builtin;

    value->type = al_runtype_simple(AL_TYPE_FUNCTION);
    value->type->nsubtypes = nargs + 1;
    value->type->subtypes = (al_runtype_t**) malloc(sizeof(al_runtype_t*) * value->type->nsubtypes);

    for ( int i = 0; i < nargs; i++ ) {
        value->type->subtypes[i] = al_runtype_dup(argtypes[i]);
        value->vf_args[i] = strdup(args[i]);
        value->vf_argtypes[i] = al_runtype_dup(argtypes[i]);
    }

    value->type->subtypes[nargs] = al_runtype_dup(rettype);

    al_namespace_add(ns, name, value->type, value);
}

al_value_t* al_string_len(al_namespace_t* ns) {
    al_value_t* val = al_namespace_lookup(ns, "<arg1>")->value;
    return al_num_fromval(strlen(val->data.str));
}

al_value_t* al_print(al_namespace_t* ns) {
    al_value_t* val = al_namespace_lookup(ns, "<arg1>")->value;
    al_value_print(val);
    return al_num_fromval(0);
}

void al_register_builtins(al_namespace_t* ns) {
    al_namespace_register_builtin(
        ns, "strlen", al_string_len,
        (char*[]) { "<arg1>", }, (al_runtype_t*[]) { al_runtype_simple(AL_TYPE_STRING), }, 1,
        al_runtype_simple(AL_TYPE_NUMBER)
    );

    al_namespace_register_builtin(
        ns, "print", al_print,
        (char*[]) { "<arg1>", }, (al_runtype_t*[]) { al_runtype_simple(AL_TYPE_STRING), }, 1,
        al_runtype_simple(AL_TYPE_NUMBER)
    );
}

al_value_t* al_function_fromast(al_ast_t* expr) {
    assert(expr->type == AL_AST_FUNCTION);

    al_value_t* value = ALOBJ(al_value_t);
    value->data.func = ALOBJ(al_value_func_t);

    value->vf_builtin = NULL;
    value->vf_nargs = expr->fn_nargs;
    value->vf_args = (char**) malloc(sizeof(char*) * value->vf_nargs);
    value->vf_argtypes = (al_runtype_t**) malloc(sizeof(al_runtype_t*) * value->vf_nargs);

    for ( int i = 0; i < value->vf_nargs; i++ ) {
        value->vf_args[i] = expr->fn_args[i];
        value->vf_argtypes[i] = al_runtype_deriv(expr->fn_argtypes[i]);
    }

    value->vf_rettype = al_runtype_deriv(expr->fn_ret);
    value->vf_body = expr->fn_block;

    value->type = al_runtype_simple(AL_TYPE_FUNCTION);
    value->type->nsubtypes = value->vf_nargs + 1;
    value->type->subtypes = (al_runtype_t**) malloc(sizeof(al_runtype_t*) * value->type->nsubtypes);

    for ( int i = 0; i < value->vf_nargs; i++ ) {
        value->type->subtypes[i] = value->vf_argtypes[i];
    }

    value->type->subtypes[value->vf_nargs] = value->vf_rettype;

    return value;
}

void al_exec_block(al_namespace_t* ns, al_ast_t* block);

al_namespace_t* al_namespace_init() {
    al_namespace_t* root = ALOBJ(al_namespace_t);
    root->atrips = 64;
    root->ntrips = 0;
    root->trips = malloc(root->atrips * sizeof(al_trip_t*));
    root->parent = NULL;
    al_register_builtins(root);
    return root;
}

#define AL_RET_STR "<ret>"

al_value_t* al_exec_expr(al_namespace_t* ns, al_ast_t* expr) {
    if ( expr->type == AL_AST_VAR ) {
        al_trip_t* trip = al_namespace_lookup(ns, expr->data.str);

        if ( trip == NULL ) {
            printf("unknown var %s\n", expr->data.str);
            assert(0); // TODO: error
        }

        return trip->value;
    }

    if ( expr->type == AL_AST_NUM ) {
        al_value_t* value = ALOBJ(al_value_t);
        value->data.num = expr->data.num;
        value->type = al_runtype_simple(AL_TYPE_NUMBER);
        return value;
    }

    if ( expr->type == AL_AST_STR ) {
        al_value_t* value = ALOBJ(al_value_t);
        value->data.str = expr->data.str;
        value->type = al_runtype_simple(AL_TYPE_STRING);
        return value;
    }

    if ( expr->type == AL_AST_LIST ) {
        al_value_t* value = al_value_list_empty();

        for ( int i = 0; i < expr->li_nitems; i++ ) {
            al_value_list_append(value, al_exec_expr(ns, expr->li_items[i]));
        }

        return value;
    }

    if ( expr->type == AL_AST_FUNCTION ) {
        al_value_t* value = al_function_fromast(expr);
        return value;
    }

    if ( expr->type == AL_AST_CALL ) {
        al_value_t* func = al_exec_expr(ns, expr->c_func);

        if ( func == NULL ) {
            assert(0); // TODO: error
        }

        if ( expr->c_nargs != func->vf_nargs ) {
            printf("arg count mismatch");
            assert(0);
        }

        al_namespace_t* ns2 = al_namespace_init();
        ns2->parent = ns;

        for ( int i = 0; i < expr->c_nargs; i++ ) {
            al_value_t* argval = al_exec_expr(ns, expr->c_args[i]);

            if ( !al_exec_typecheck(func->vf_argtypes[i], argval->type) ) {
                printf("failed type check %d\n", i);
                assert(0); // TODO: error
            }

            al_namespace_add(ns2, func->vf_args[i], argval->type, argval);
        }

        al_value_t* ret = NULL;

        if ( func->vf_body ) {
            al_exec_block(ns2, func->vf_body);          

            al_trip_t* trip = al_namespace_lookup(ns2, AL_RET_STR);

            if ( trip != NULL ) {
                ret = trip->value;
            } 
        }
        else if ( func->vf_builtin ) {
            ret = func->vf_builtin(ns2);
        }
        else {
            assert(0);
        }


        if ( ret == NULL ) {
            if ( al_dispatches[func->vf_rettype->type].op_default != NULL ) {
                printf("DEFAULT\n");
                return al_dispatches[func->vf_rettype->type].op_default();
            }
            else {
                printf("no return");
                assert(0); // TODO: error
            }
        }

        if ( !al_exec_typecheck(func->vf_rettype, ret->type) ) {
            printf("rettype wrong");
            assert(0);
        }

        return ret;
    }

    if ( expr->type == AL_AST_EXPR ) {
        if ( expr->e_rhs != NULL ) {
            al_value_t* lhs = al_exec_expr(ns, expr->e_lhs);
            al_value_t* rhs = al_exec_expr(ns, expr->e_rhs);

            if ( !al_exec_typecheck(lhs->type, rhs->type) ) {
                printf("mismatch type\n");
                assert(0); // TODO: error
            }

            switch ( expr->e_oper ) {
                case AL_OPER_ADD:
                    if ( al_dispatches[lhs->type->type].op_add != NULL )
                        return al_dispatches[lhs->type->type].op_add(lhs, rhs);
                    else
                        goto no_dispatch;

                case AL_OPER_SUB:
                    if ( al_dispatches[lhs->type->type].op_sub != NULL )
                        return al_dispatches[lhs->type->type].op_sub(lhs, rhs);
                    else
                        goto no_dispatch;
                    break;

                case AL_OPER_MUL:
                    if ( al_dispatches[lhs->type->type].op_mul != NULL )
                        return al_dispatches[lhs->type->type].op_mul(lhs, rhs);
                    else
                        goto no_dispatch;
                    break;

                case AL_OPER_DIV:
                    if ( al_dispatches[lhs->type->type].op_div != NULL )
                        return al_dispatches[lhs->type->type].op_div(lhs, rhs);
                    else
                        goto no_dispatch;
                    break;

                case AL_OPER_APPLY:
                case AL_OPER_OR:
                case AL_OPER_AND:
                case AL_OPER_XOR:
                case AL_OPER_NOT:
                case AL_OPER_INV:
                    break;

            }

            no_dispatch:
            printf("Unable to find dispatch");
            assert(0); // TODO: error
        }
    }

    return NULL;
}

void al_exec_let(al_namespace_t* ns, al_ast_t* let) {
    assert(let->type == AL_AST_LET);

    al_value_t* val = al_exec_expr(ns, let->l_value);

    if ( val == NULL ) {
        return;
    }

    al_runtype_t* type = al_runtype_deriv(let->l_type);

    if ( type == NULL ) {
        printf("type deriv failed\n");
        assert(0); // TODO error
    }

    if ( !al_exec_typecheck(type, val->type) ) {
        printf("typecheck failed for ");
        al_value_print(val);
        printf("\n");

        assert(0); // TODO error
    }

    al_namespace_add(ns, let->l_name, type, val);
}

void al_exec_lexpr(al_namespace_t* ns, al_ast_t* lexpr) {
    al_value_t* val = al_exec_expr(ns, lexpr->le_rhs);

    if ( lexpr->le_lhs != NULL ) {
        al_ast_t* lhs = lexpr->le_lhs;

        if ( lhs->type == AL_AST_VAR ) {
            al_trip_t* trip = al_namespace_lookup(ns, lhs->data.str);

            if ( trip == NULL ) {
                printf("unknown var %s\n", lhs->data.str);
                return;
            }

            if ( !al_exec_typecheck(trip->type, val->type) ) {
                printf("bad typecheck for asgn\n");
                return;
            }

            trip->value = val;
        }
    }
}

int al_exec_stmt(al_namespace_t* ns, al_ast_t* stmt) {
    if ( stmt->type == AL_AST_LET ) {
        al_exec_let(ns, stmt);
        return 1;
    }

    if (stmt->type == AL_AST_RETURN ) {
        if ( stmt->r_value != NULL ) {
            al_value_t* val = al_exec_expr(ns, stmt->r_value);
            al_namespace_add(ns, AL_RET_STR, val->type, val);
        }

        return 0;
    }

    if (stmt->type == AL_AST_LEXPR ) {
        al_exec_lexpr(ns, stmt);
        return 1;
    }

    printf("Unhandled type %s\n", al_asttype_str[stmt->type]);

    return 0;
}

void al_exec_block(al_namespace_t* ns, al_ast_t* block) {
    assert(block->type == AL_AST_BLOCK);

    for ( int i = 0; i < block->b_nchildren; i++ ) {
        if ( !al_exec_stmt(ns, block->b_children[i]) ) {
            break;
        }
    }
}

void al_exec(char* contents, size_t len) {
    al_token_t* tokens = al_lex(contents, len);
    // al_print_tokens(tokens);
    al_token_t* ttokens = tokens;
    al_ast_t* ast = al_parse_block(&ttokens, 0);
    // al_ast_print(ast, 0);

    al_namespace_t* root = al_namespace_init();

    al_exec_block(root, ast);

    al_trip_t* trip;
    if ( (trip = al_namespace_lookup(root, AL_RET_STR)) != NULL ) {
        al_value_print(trip->value);
    }

    free(tokens);
}

void repl() {
    char input[1024];

    al_namespace_t* root = al_namespace_init();

    while ( 1 ) {
        printf("> ");
        fgets(input, 1024, stdin);

        if ( input[0] == '.' ) {
            if ( strncmp(input, ".exit", 5) == 0 ) {
                break;
            }
            else {
                printf("Unknown command: %s\n", input);
                continue;
            }
        }

        al_token_t* tokens = al_lex(input, strlen(input));
        // al_print_tokens(tokens);
        al_token_t* ttokens = tokens;
        al_ast_t* ast = al_parse_stmt(&ttokens);

        if ( ast == NULL ) {
            printf("parse fail\n");
            continue;
        }

        al_exec_stmt(root, ast);

        al_trip_t* trip = NULL;
        if ( (trip = al_namespace_lookup(root, AL_RET_STR)) != NULL ) {
            al_value_print(trip->value);
        }

        free(tokens);
    }
}

void usage(char* argv[]) {
    fprintf(stderr, "Usage: %s [file.al] ...", argv[0]);
    exit(1);
}

int main(int argc, char* argv[]) {
    if ( argc == 1 ) {
        repl();
    }
    else {
        char* filename = argv[1];
        FILE* file = fopen(filename, "r");

        if ( file == NULL ) {
            usage(argv);
        }
        size_t start = (size_t) ftell(file);
        fseek(file, 0, SEEK_END);
        size_t size = (size_t) ftell(file) - start;
        rewind(file);

        char* contents = (char*) malloc(size);
        fread(contents, size, 1, file);
        fclose(file);

        // printf("contents (%zu): `%s`\n", size, contents);
        al_exec(contents, size);
        free(contents);
    }
}
