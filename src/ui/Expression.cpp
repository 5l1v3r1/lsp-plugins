/*
 * Expression.cpp
 *
 *  Created on: 28 окт. 2016 г.
 *      Author: sadko
 */

#include <ui/ui.h>

namespace lsp
{
    Expression::Expression(): IUIPortListener()
    {
        pRoot       = NULL;
        pUI         = NULL;
        pListener   = NULL;
    }

    Expression::~Expression()
    {
        destroy_data(pRoot);
        pRoot       = NULL;
        pUI         = NULL;
        pListener   = NULL;
    }

    void Expression::init(plugin_ui *ui, IUIPortListener *listener)
    {
        pRoot       = NULL;
        pUI         = ui;
        pListener   = listener;
    }

    void Expression::destroy_data(binding_t *ptr)
    {
        if (ptr == NULL)
            return;

        if (ptr->enOp == OP_LOAD)
        {
            if (ptr->sLoad.pPort != NULL)
            {
                ptr->sLoad.pPort->unbind(this);
                ptr->sLoad.pPort = NULL;
            }
        }
        else
        {
            destroy_data(ptr->sCalc.pLeft);
            ptr->sCalc.pLeft = NULL;
            destroy_data(ptr->sCalc.pRight);
            ptr->sCalc.pRight = NULL;
        }

        // Delete binding
        delete ptr;
    }

    void Expression::destroy()
    {
        destroy_data(pRoot);
        pRoot = NULL;
        pListener = NULL;
    }

    void Expression::notify(IUIPort *port)
    {
        if (pListener != NULL)
            pListener->notify(port);
    }

    inline bool Expression::isalpha(char ch)
    {
        return ((ch >= 'a') && (ch <= 'z')) ||
               ((ch >= 'A') && (ch <= 'Z')) ||
               (ch == '_');
    }

    inline bool Expression::isnum(char ch)
    {
        return ((ch >= '0') && (ch <= '9'));
    }

    inline bool Expression::isalnum(char ch)
    {
        return isalpha(ch) || isnum(ch);
    }

    Expression::token_t Expression::get_token(tokenizer_t *t, bool get)
    {
        // Pre-checks
        if (!get)
            return t->enType;

        // Skip whitespace
        char c = t->sUnget;
        t->sUnget = '\0';

        while (c == '\0')
        {
            c = *(t->pStr);
            if (c == '\0')
            {
                t->sText[0] = '\0';
                return t->enType   = TT_EOF;
            }

            // Update pointer
            t->pStr++;

            // Leave cycle if not a whitespace
            if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))
                c = '\0';
        }

        // Now analyze first character
        switch (c)
        {
            // Non-alpha
            case '(': // TT_LBRACE
                return t->enType   = TT_LBRACE;
            case ')': // TT_RBRACE
                return t->enType   = TT_RBRACE;
            case '+': // TT_ADD
                return t->enType   = TT_ADD;
            case '-': // TT_SUB
                return t->enType   = TT_SUB;
            case '*': // TT_MUL
                return t->enType   = TT_MUL;
            case '/': // TT_DIV
                return t->enType   = TT_DIV;
            case '%': // TT_MOD
                return t->enType   = TT_MOD;
            case '?': // TT_QUESTION
                return t->enType   = TT_QUESTION;
            case '&': // TT_AND
                if (*(t->pStr) == '&') // Accept both variants: & and &&
                    t->pStr ++;
                return t->enType   = TT_AND;

            case '|': // TT_OR
                if (*(t->pStr) == '|') // Accept both variants: | and ||
                    t->pStr ++;
                return t->enType   = TT_OR;

            case '!': // TT_NOT, TT_NOT_EQ
                if (*(t->pStr) == '=') // Check for !=
                {
                    t->pStr ++;
                    return t->enType   = TT_NOT_EQ;
                }
                return t->enType   = TT_NOT;

            case '^': // TT_XOR
                if (*(t->pStr) == '^') // Accept both variants: ^ and ^^
                    t->pStr ++;
                return t->enType   = TT_XOR;

            case '<': // TT_LESS, TT_LESS_EQ, TT_NOT_EQ
                c = *(t->pStr);
                if (c == '=') // <=
                {
                    t->pStr ++;
                    return t->enType    = TT_LESS_EQ;
                }
                else if (c == '>') // <>
                {
                    t->pStr ++;
                    return t->enType    = TT_NOT_EQ;
                }
                return t->enType    = TT_LESS;

            case '>': // TT_GREATER, TT_GREATER_EQ
                if (*(t->pStr) == '=') // Check for !=
                {
                    t->pStr ++;
                    return t->enType   = TT_GREATER_EQ;
                }
                return t->enType   = TT_GREATER;

            case '=': // TT_EQ
                if (*(t->pStr) == '=') // Accept both variants: = and ==
                    t->pStr ++;
                return t->enType   = TT_EQ;

            case ':': // TT_DOTS, TT_IDENTIFIER
            {
                // Check for next character
                c = *(t->pStr);
                if ((!isalpha(c)) && (c != '_'))
                    return t->enType    = TT_DOTS;

                size_t i=0;
                do
                {
                    if (i >= (EXPR_TOKEN_LENGTH-1))
                    {
                        t->sText[i] = '\0';
                        return t->enType    = TT_UNKNOWN;
                    }
                    t->sText[i++] = c;

                    c = *(++t->pStr);
                } while ((isalnum(c)) || (c == '_'));

                t->sText[i] = '\0';
                return t->enType    = TT_IDENTIFIER;
            }

            // Alpha
            case 'a': case 'A': // TT_AND, TT_ADD
            {
                const char *p = t->pStr;
                if ((p[0] == 'n') || (p[0] == 'N'))
                {
                    if (((p[1] == 'd') || (p[1] == 'D')) &&
                        (!isalpha(p[2]))) // AND
                    {
                        t->pStr            += 2;
                        return t->enType    = TT_AND;
                    }
                }
                else if ((p[0] == 'd') || (p[0] == 'D'))
                {
                    if (((p[1] == 'd') || (p[1] == 'D')) &&
                        (!isalpha(p[2]))) // ADD
                    {
                        t->pStr            += 2;
                        return t->enType    = TT_ADD;
                    }
                }
                return t->enType    = TT_UNKNOWN;
            }

            case 'b': case 'B': // TT_BAND, TT_BNOT, TT_BOR, TT_BXOR
            {
                const char *p = t->pStr;
                if ((p[0] == 'a') || (p[0] == 'A'))
                {
                    if (((p[1] == 'n') || (p[1] == 'N')) &&
                        ((p[2] == 'd') || (p[2] == 'D')) &&
                        (!isalpha(p[3])))
                    {
                        t->pStr            += 3;
                        return t->enType    = TT_BAND;
                    }
                }
                else if ((p[0] == 'n') || (p[0] == 'N'))
                {
                    if (((p[1] == 'o') || (p[1] == 'O')) &&
                        ((p[2] == 't') || (p[2] == 'T')) &&
                        (!isalpha(p[3])))
                    {
                        t->pStr            += 3;
                        return t->enType    = TT_BNOT;
                    }
                }
                else if ((p[0] == 'o') || (p[0] == 'O'))
                {
                    if (((p[1] == 'r') || (p[1] == 'R')) &&
                        (!isalpha(p[2])))
                    {
                        t->pStr            += 2;
                        return t->enType    = TT_BOR;
                    }
                }
                else if ((p[0] == 'x') || (p[0] == 'X'))
                {
                    if (((p[1] == 'o') || (p[1] == 'O')) &&
                        ((p[2] == 'r') || (p[2] == 'R')) &&
                        (!isalpha(p[3])))
                    {
                        t->pStr            += 3;
                        return t->enType    = TT_BXOR;
                    }
                }
                return t->enType    = TT_UNKNOWN;
            }

            case 'd': case 'D': // TT_DIV
            {
                const char *p = t->pStr;
                if ((p[0] == 'i') || (p[0] == 'I'))
                {
                    if (((p[1] == 'v') || (p[1] == 'V')) &&
                        (!isalpha(p[2])))
                    {
                        t->pStr            += 2;
                        return t->enType    = TT_DIV;
                    }
                }
                return t->enType    = TT_UNKNOWN;
            }

            case 'e': case 'E': // TT_EQ
            {
                const char *p = t->pStr;
                if ((p[0] == 'q') || (p[0] == 'Q'))
                {
                    if (!isalpha(p[1])) // EQ
                    {
                        t->pStr            += 1;
                        return t->enType    = TT_EQ;
                    }
                }

                return t->enType    = TT_UNKNOWN;
            }

            case 'f': case 'F': // FALSE
            {
                const char *p = t->pStr;
                if (((p[0] == 'A') || (p[0] == 'a')) &&
                    ((p[1] == 'L') || (p[1] == 'l')) &&
                    ((p[2] == 'S') || (p[2] == 's')) &&
                    ((p[3] == 'E') || (p[3] == 'e')) &&
                    (!isalpha(p[4])))
                {
                    t->pStr            += 4;
                    t->fValue   = 0.0f;
                    return t->enType    = TT_VALUE;
                }
                return t->enType    = TT_UNKNOWN;
            }

            case 'g': case 'G': // TT_GREATER, TT_GREATER_EQ
            {
                const char *p = t->pStr;
                if ((p[0] == 't') || (p[0] == 'T'))
                {
                    if (!isalpha(p[1])) // GT
                    {
                        t->pStr            += 1;
                        return t->enType    = TT_GREATER;
                    }
                }
                else if ((p[0] == 'e') || (p[0] == 'E'))
                {
                    if (!isalpha(p[1])) // GE
                    {
                        t->pStr            += 1;
                        return t->enType    = TT_GREATER_EQ;
                    }
                }

                return t->enType    = TT_UNKNOWN;
            }

            case 'i': case 'I': // TT_IADD, TT_ISUB, TT_IMUL, TT_IDIV, TT_IMOD
            {
                const char *p = t->pStr;
                if ((p[0] == 'a') || (p[0] == 'A'))
                {
                    if (((p[1] == 'd') || (p[1] == 'D')) &&
                        ((p[2] == 'd') || (p[2] == 'D')) &&
                        (!isalpha(p[3]))) // IADD
                    {
                        t->pStr            += 3;
                        return t->enType    = TT_IADD;
                    }
                }
                else if ((p[0] == 'd') || (p[0] == 'D'))
                {
                    if (((p[1] == 'i') || (p[1] == 'I')) &&
                        ((p[2] == 'v') || (p[2] == 'V')) &&
                        (!isalpha(p[3]))) // IDIV
                    {
                        t->pStr            += 3;
                        return t->enType    = TT_IDIV;
                    }
                }
                else if ((p[0] == 'e') || (p[0] == 'E'))
                {
                    if (((p[1] == 'q') || (p[1] == 'Q')) &&
                        (!isalpha(p[2]))) // IEQ
                    {
                        t->pStr            += 2;
                        return t->enType    = TT_IEQ;
                    }
                    else if (!isalpha(p[1])) // IE
                    {
                        t->pStr            += 1;
                        return t->enType    = TT_IEQ;
                    }
                }
                else if ((p[0] == 'g') || (p[0] == 'G'))
                {
                    if ((p[1] == 't') || (p[1] == 'T'))
                    {
                        if (!isalpha(p[2])) // IGT
                        {
                            t->pStr            += 2;
                            return t->enType    = TT_IGREATER;
                        }
                    }
                    else if ((p[1] == 'e') || (p[1] == 'E'))
                    {
                        if (!isalpha(p[2])) // IGE
                        {
                            t->pStr            += 2;
                            return t->enType    = TT_IGREATER_EQ;
                        }
                    }
                }
                else if ((p[0] == 'l') || (p[0] == 'L'))
                {
                    if ((p[1] == 't') || (p[1] == 'T'))
                    {
                        if (!isalpha(p[2])) // ILT
                        {
                            t->pStr            += 2;
                            return t->enType    = TT_ILESS;
                        }
                    }
                    else if ((p[1] == 'e') || (p[1] == 'E'))
                    {
                        if (!isalpha(p[2])) // ILE
                        {
                            t->pStr            += 2;
                            return t->enType    = TT_ILESS_EQ;
                        }
                    }
                }
                else if ((p[0] == 'm') || (p[0] == 'M'))
                {
                    if ((p[1] == 'o') || (p[1] == 'O'))
                    {
                        if (((p[2] == 'd') || (p[2] == 'D')) &&
                            (!isalpha(p[3]))) // IMOD
                        {
                            t->pStr            += 3;
                            return t->enType    = TT_MOD;
                        }
                    }
                    else if ((p[1] == 'u') || (p[1] == 'U'))
                    {
                        if (((p[2] == 'l') || (p[2] == 'L')) &&
                            (!isalpha(p[3]))) // IMUL
                        {
                            t->pStr            += 3;
                            return t->enType    = TT_IMUL;
                        }
                    }
                }
                else if ((p[0] == 'n') || (p[0] == 'N'))
                {
                    if ((p[1] == 'e') || (p[1] == 'E'))
                    {
                        if (!isalpha(p[2])) // INE
                        {
                            t->pStr            += 2;
                            return t->enType    = TT_INOT_EQ;
                        }
                    }
                    else if ((p[1] == 'g') || (p[1] == 'G'))
                    {
                        if (((p[2] == 't') || (p[2] == 'T')) &&
                            (!isalpha(p[3]))) // INGT
                        {
                            t->pStr            += 3;
                            return t->enType    = TT_ILESS_EQ;
                        }
                        else if (((p[2] == 'e') || (p[2] == 'E')) &&
                            (!isalpha(p[3]))) // INGE
                        {
                            t->pStr            += 3;
                            return t->enType    = TT_ILESS;
                        }
                    }
                    else if ((p[1] == 'l') || (p[1] == 'L'))
                    {
                        if (((p[2] == 't') || (p[2] == 'T')) &&
                            (!isalpha(p[3]))) // INLT
                        {
                            t->pStr            += 3;
                            return t->enType    = TT_IGREATER_EQ;
                        }
                        else if (((p[2] == 'e') || (p[1] == 'E')) &&
                            (!isalpha(p[3]))) // INLE
                        {
                            t->pStr            += 3;
                            return t->enType    = TT_IGREATER;
                        }
                    }
                }
                else if ((p[0] == 's') || (p[0] == 'S'))
                {
                    if (((p[1] == 'u') || (p[1] == 'U')) &&
                        ((p[2] == 'b') || (p[2] == 'B')) &&
                        (!isalpha(p[3]))) // ISUB
                    {
                        t->pStr            += 3;
                        return t->enType    = TT_ISUB;
                    }
                }

                return t->enType    = TT_UNKNOWN;
            }

            case 'l': case 'L': // TT_LESS, TT_LESS_EQ
            {
                const char *p = t->pStr;
                if ((p[0] == 't') || (p[0] == 'T'))
                {
                    if (!isalpha(p[1])) // LT
                    {
                        t->pStr            += 1;
                        return t->enType    = TT_LESS;
                    }
                }
                else if ((p[0] == 'e') || (p[0] == 'E'))
                {
                    if (!isalpha(p[1])) // LE
                    {
                        t->pStr            += 1;
                        return t->enType    = TT_LESS_EQ;
                    }
                }

                return t->enType    = TT_UNKNOWN;
            }

            case 'm': case 'M': // TT_MUL, TT_MOD
            {
                const char *p = t->pStr;
                if ((p[0] == 'o') || (p[0] == 'O'))
                {
                    if (((p[1] == 'd') || (p[1] == 'D')) &&
                        (!isalpha(p[2]))) // MOD
                    {
                        t->pStr            += 2;
                        return t->enType    = TT_MOD;
                    }
                }
                else if ((p[0] == 'u') || (p[0] == 'U'))
                {
                    if (((p[1] == 'l') || (p[1] == 'L')) &&
                        (!isalpha(p[2]))) // MUL
                    {
                        t->pStr            += 2;
                        return t->enType    = TT_MUL;
                    }
                }
                return t->enType    = TT_UNKNOWN;
            }

            case 'n': case 'N': // TT_NOT, TT_LESS, TT_GREATER, TT_LESS_EQ, TT_GREATER_EQ, TT_NOT_EQ
            {
                const char *p = t->pStr;
                if ((p[0] == 'o') || (p[0] == 'O'))
                {
                    if (((p[1] == 't') || (p[1] == 'T')) &&
                        (!isalpha(p[2]))) // NOT
                    {
                        t->pStr            += 2;
                        return t->enType    = TT_NOT;
                    }
                }
                else if ((p[0] == 'g') || (p[0] == 'G'))
                {
                    if (((p[1] == 't') || (p[1] == 'T')) &&
                        (!isalpha(p[2]))) // NGT
                    {
                        t->pStr            += 2;
                        return t->enType    = TT_LESS_EQ;
                    }
                    else if (((p[1] == 'e') || (p[1] == 'E')) &&
                        (!isalpha(p[2]))) // NGE
                    {
                        t->pStr            += 2;
                        return t->enType    = TT_LESS;
                    }
                }
                else if ((p[0] == 'l') || (p[0] == 'L'))
                {
                    if (((p[1] == 't') || (p[1] == 'T')) &&
                        (!isalpha(p[2]))) // NLT
                    {
                        t->pStr            += 2;
                        return t->enType    = TT_GREATER_EQ;
                    }
                    else if (((p[1] == 'e') || (p[1] == 'E')) &&
                        (!isalpha(p[2]))) // NLE
                    {
                        t->pStr            += 2;
                        return t->enType    = TT_GREATER;
                    }
                }
                else if ((p[0] == 'e') || (p[0] == 'E'))
                {
                    if (!isalpha(p[1])) // NE
                    {
                        t->pStr            += 1;
                        return t->enType    = TT_NOT_EQ;
                    }
                }
                return t->enType    = TT_UNKNOWN;
            }

            case 'o': case 'O': // TT_OR
            {
                const char *p = t->pStr;
                if (((p[0] == 'r') || (p[0] == 'R')) &&
                    (!isalpha(p[1])))
                {
                    t->pStr            += 1;
                    return t->enType    = TT_OR;
                }

                return t->enType    = TT_UNKNOWN;
            }

            case 's': case 'S': // TT_SUB
            {
                const char *p = t->pStr;
                if (((p[0] == 'u') || (p[0] == 'U')) &&
                    ((p[1] == 'b') || (p[1] == 'B')) &&
                    (!isalpha(p[2])))
                {
                    t->pStr            += 2;
                    return t->enType    = TT_SUB;
                }
                return t->enType    = TT_UNKNOWN;
            }

            case 't': case 'T': // TRUE
            {
                const char *p = t->pStr;
                if (((p[0] == 'R') || (p[0] == 'r')) &&
                    ((p[1] == 'U') || (p[1] == 'u')) &&
                    ((p[2] == 'E') || (p[2] == 'e')) &&
                    (!isalpha(p[3])))
                {
                    t->pStr            += 3;
                    t->fValue   = 1.0f;
                    return t->enType    = TT_VALUE;
                }
                return t->enType    = TT_UNKNOWN;
            }

            case 'x': case 'X': // TT_XOR
            {
                const char *p = t->pStr;
                if ((p[0] == 'o') || (p[0] == 'O'))
                {
                    if (((p[1] == 'r') || (p[1] == 'R')) &&
                        (!isalpha(p[2])))
                    {
                        t->pStr            += 2;
                        return t->enType    = TT_XOR;
                    }
                }
                return t->enType    = TT_UNKNOWN;
            }

            // Defaults
            default: // TT_VALUE
            {
                const char *p = &t->pStr[-1];

                // Try to parse float value
                char *endptr    = NULL;
                errno           = 0;
                t->fValue       = strtof(p, &endptr);
                if (errno != 0)
                    return t->enType    = TT_UNKNOWN;

                t->pStr     = endptr;
                return t->enType    = TT_VALUE;
            }
        }

        return t->enType    = TT_UNKNOWN;
    }

    float Expression::execute(binding_t *expr)
    {
        #define BINARY(key, op) case key: return execute(expr->sCalc.pLeft) op execute(expr->sCalc.pRight);
        #define INT_BINARY(key, op) case key: return long(execute(expr->sCalc.pLeft)) op long(execute(expr->sCalc.pRight));
        #define UNARY(key, op) case key: return op execute(expr->sCalc.pLeft);
        #define INT_UNARY(key, op) case key: return op long(execute(expr->sCalc.pLeft));

        if (expr == NULL)
            return 0.0f;
        switch (expr->enOp)
        {
            case OP_LOAD:
                return (expr->sLoad.pPort != NULL) ?
                    expr->sLoad.pPort->getValue() : expr->sLoad.fValue;

            BINARY(OP_ADD, +)
            BINARY(OP_SUB, -)
            UNARY(OP_SIGN, -)
            BINARY(OP_MUL, *)
            BINARY(OP_DIV, /)

            case OP_AND:
                if (execute(expr->sCalc.pLeft) < 0.5f)
                    return 0.0f;
                return (execute(expr->sCalc.pRight) >= 0.5f) ? 1.0f : 0.0f;

            case OP_OR:
                if (execute(expr->sCalc.pLeft) >= 0.5f)
                    return 1.0f;
                return (execute(expr->sCalc.pRight) >= 0.5f) ? 1.0f : 0.0f;

            case OP_NOT:
                return execute(expr->sCalc.pLeft) < 0.5f;

            case OP_TERNARY:
                return (execute(expr->sCalc.pCond) >= 0.5f) ?
                    execute(expr->sCalc.pLeft) :
                    execute(expr->sCalc.pRight);

            case OP_XOR:
            {
                bool a = execute(expr->sCalc.pLeft) >= 0.5f;
                bool b = execute(expr->sCalc.pRight) >= 0.5f;
                return (a ^ b) ? 1.0f : 0.0f;
            }

            // Bitwise operators
            INT_BINARY(OP_BAND, &);
            INT_BINARY(OP_BOR, |);
            INT_BINARY(OP_BXOR, ^);
            INT_UNARY(OP_BNOT, ~);

            // Integer arithmetics
            INT_BINARY(OP_IADD, +);
            INT_BINARY(OP_ISUB, -);
            INT_BINARY(OP_IMUL, *);
            INT_BINARY(OP_IDIV, /);
            INT_BINARY(OP_MOD, %);

            // Float comparisons
            BINARY(OP_LESS, <)
            BINARY(OP_GREATER, >)
            BINARY(OP_LESS_EQ, <=)
            BINARY(OP_GREATER_EQ, >=)
            BINARY(OP_NOT_EQ, !=)
            BINARY(OP_EQ, ==)

            // Integer comparisons
            INT_BINARY(OP_ILESS, <)
            INT_BINARY(OP_IGREATER, >)
            INT_BINARY(OP_ILESS_EQ, <=)
            INT_BINARY(OP_IGREATER_EQ, >=)
            INT_BINARY(OP_INOT_EQ, !=)
            INT_BINARY(OP_IEQ, ==)

            default:
                return 0.0f;
        }
        #undef BINARY
        #undef INT_BINARY
        #undef UNARY

        return 0.0f;
    }

    float Expression::evaluate()
    {
        return (pRoot != NULL) ? execute(pRoot) : 0.0f;
    }

    Expression::binding_t *Expression::parse_ternary(tokenizer_t *t, bool get)
    {
        // Parse condition part
        binding_t *cond = parse_xor(t, get);
        if (cond == NULL)
            return NULL;

        // Check token
        token_t tok = get_token(t, false);
        if (tok != TT_QUESTION)
            return cond;

        // Parse left part
        binding_t *left = parse_ternary(t, true);
        if (left == NULL)
        {
            destroy_data(cond);
            return NULL;
        }

        // Check token
        tok = get_token(t, false);
        if (tok != TT_DOTS)
            return cond;

        // Parse right part
        binding_t *right = parse_ternary(t, true);
        if (right == NULL)
        {
            destroy_data(cond);
            destroy_data(left);
            return NULL;
        }

        // Create binding between left and right
        binding_t *bind     = new binding_t;
        if (bind == NULL)
        {
            destroy_data(cond);
            destroy_data(left);
            destroy_data(right);
            return NULL;
        }
        bind->enOp          = OP_TERNARY;
        bind->sCalc.pLeft   = left;
        bind->sCalc.pRight  = right;
        bind->sCalc.pCond   = cond;
        return bind;
    }

    Expression::binding_t *Expression::parse_xor(tokenizer_t *t, bool get)
    {
        // Parse left part
        binding_t *left = parse_or(t, get);
        if (left == NULL)
            return NULL;

        // Check token
        token_t tok = get_token(t, false);
        if ((tok != TT_XOR) || (tok != TT_BXOR))
            return left;

        // Parse right part
        binding_t *right = parse_xor(t, true);
        if (right == NULL)
        {
            destroy_data(left);
            return NULL;
        }

        // Create binding between left and right
        binding_t *bind     = new binding_t;
        if (bind == NULL)
        {
            destroy_data(left);
            destroy_data(right);
            return NULL;
        }
        bind->enOp          = (tok == TT_XOR) ? OP_XOR : OP_BXOR;
        bind->sCalc.pLeft   = left;
        bind->sCalc.pRight  = right;
        return bind;
    }

    Expression::binding_t  *Expression::parse_or(tokenizer_t *t, bool get)
    {
        // Parse left part
        binding_t *left = parse_and(t, get);
        if (left == NULL)
            return NULL;

        // Check token
        token_t tok = get_token(t, false);
        if (tok != TT_OR)
            return left;

        // Parse right part
        binding_t *right = parse_or(t, true);
        if (right == NULL)
        {
            destroy_data(left);
            return NULL;
        }

        // Create binding between left and right
        binding_t *bind     = new binding_t;
        if (bind == NULL)
        {
            destroy_data(left);
            destroy_data(right);
            return NULL;
        }
        bind->enOp          = OP_OR;
        bind->sCalc.pLeft   = left;
        bind->sCalc.pRight  = right;
        return bind;
    }

    Expression::binding_t  *Expression::parse_and(tokenizer_t *t, bool get)
    {
        // Parse left part
        binding_t *left = parse_bit_or(t, get);
        if (left == NULL)
            return NULL;

        // Check token
        token_t tok = get_token(t, false);
        if (tok != TT_AND)
            return left;

        // Parse right part
        binding_t *right = parse_and(t, true);
        if (right == NULL)
        {
            destroy_data(left);
            return NULL;
        }

        // Create binding between left and right
        binding_t *bind     = new binding_t;
        if (bind == NULL)
        {
            destroy_data(left);
            destroy_data(right);
            return NULL;
        }
        bind->enOp          = OP_AND;
        bind->sCalc.pLeft   = left;
        bind->sCalc.pRight  = right;
        return bind;
    }

    Expression::binding_t  *Expression::parse_bit_or(tokenizer_t *t, bool get)
    {
        // Parse left part
        binding_t *left = parse_bit_xor(t, get);
        if (left == NULL)
            return NULL;

        // Check token
        token_t tok = get_token(t, false);
        if (tok != TT_BOR)
            return left;

        // Parse right part
        binding_t *right = parse_and(t, true);
        if (right == NULL)
        {
            destroy_data(left);
            return NULL;
        }

        // Create binding between left and right
        binding_t *bind     = new binding_t;
        if (bind == NULL)
        {
            destroy_data(left);
            destroy_data(right);
            return NULL;
        }
        bind->enOp          = OP_BOR;
        bind->sCalc.pLeft   = left;
        bind->sCalc.pRight  = right;
        return bind;
    }

    Expression::binding_t  *Expression::parse_bit_xor(tokenizer_t *t, bool get)
    {
        // Parse left part
        binding_t *left = parse_bit_and(t, get);
        if (left == NULL)
            return NULL;

        // Check token
        token_t tok = get_token(t, false);
        if (tok != TT_BXOR)
            return left;

        // Parse right part
        binding_t *right = parse_and(t, true);
        if (right == NULL)
        {
            destroy_data(left);
            return NULL;
        }

        // Create binding between left and right
        binding_t *bind     = new binding_t;
        if (bind == NULL)
        {
            destroy_data(left);
            destroy_data(right);
            return NULL;
        }
        bind->enOp          = OP_BXOR;
        bind->sCalc.pLeft   = left;
        bind->sCalc.pRight  = right;
        return bind;
    }

    Expression::binding_t  *Expression::parse_bit_and(tokenizer_t *t, bool get)
    {
        // Parse left part
        binding_t *left = parse_cmp(t, get);
        if (left == NULL)
            return NULL;

        // Check token
        token_t tok = get_token(t, false);
        if (tok != TT_BAND)
            return left;

        // Parse right part
        binding_t *right = parse_and(t, true);
        if (right == NULL)
        {
            destroy_data(left);
            return NULL;
        }

        // Create binding between left and right
        binding_t *bind     = new binding_t;
        if (bind == NULL)
        {
            destroy_data(left);
            destroy_data(right);
            return NULL;
        }
        bind->enOp          = OP_BAND;
        bind->sCalc.pLeft   = left;
        bind->sCalc.pRight  = right;
        return bind;
    }

    Expression::binding_t  *Expression::parse_cmp(tokenizer_t *t, bool get)
    {
        // Parse left part
        binding_t *left = parse_addsub(t, get);
        if (left == NULL)
            return NULL;

        // Check token
        token_t tok = get_token(t, false);
        switch (tok)
        {
            case TT_LESS:
            case TT_GREATER:
            case TT_LESS_EQ:
            case TT_GREATER_EQ:
            case TT_NOT_EQ:
            case TT_EQ:
            case TT_ILESS:
            case TT_IGREATER:
            case TT_ILESS_EQ:
            case TT_IGREATER_EQ:
            case TT_INOT_EQ:
            case TT_IEQ:
                break;
            default:
                return left;
        }

        // Parse right part
        binding_t *right = parse_cmp(t, true);
        if (right == NULL)
        {
            destroy_data(left);
            return NULL;
        }

        // Create binding between left and right
        binding_t *bind     = new binding_t;
        if (bind == NULL)
        {
            destroy_data(left);
            destroy_data(right);
            return NULL;
        }

        switch (tok)
        {
            case TT_LESS:           bind->enOp = OP_LESS; break;
            case TT_GREATER:        bind->enOp = OP_GREATER; break;
            case TT_LESS_EQ:        bind->enOp = OP_LESS_EQ; break;
            case TT_GREATER_EQ:     bind->enOp = OP_GREATER_EQ; break;
            case TT_NOT_EQ:         bind->enOp = OP_NOT_EQ; break;
            case TT_EQ:             bind->enOp = OP_EQ; break;
            case TT_ILESS:          bind->enOp = OP_ILESS; break;
            case TT_IGREATER:       bind->enOp = OP_IGREATER; break;
            case TT_ILESS_EQ:       bind->enOp = OP_ILESS_EQ; break;
            case TT_IGREATER_EQ:    bind->enOp = OP_IGREATER_EQ; break;
            case TT_INOT_EQ:        bind->enOp = OP_INOT_EQ; break;
            case TT_IEQ:            bind->enOp = OP_IEQ; break;
            default:
            {
                destroy_data(left);
                destroy_data(right);
                destroy_data(bind);
                return NULL;
            }
        }
        bind->sCalc.pLeft   = left;
        bind->sCalc.pRight  = right;
        return bind;
    }

    Expression::binding_t  *Expression::parse_addsub(tokenizer_t *t, bool get)
    {
        // Parse left part
        binding_t *left = parse_muldiv(t, get);
        if (left == NULL)
            return NULL;

        // Check token
        token_t tok = get_token(t, false);
        switch (tok)
        {
            case TT_ADD:
            case TT_SUB:
            case TT_IADD:
            case TT_ISUB:
                break;
            default:
                return left;
        }

        // Parse right part
        binding_t *right = parse_addsub(t, true);
        if (right == NULL)
        {
            destroy_data(left);
            return NULL;
        }

        // Create binding between left and right
        binding_t *bind     = new binding_t;
        if (bind == NULL)
        {
            destroy_data(left);
            destroy_data(right);
            return NULL;
        }

        switch (tok)
        {
            case TT_ADD:    bind->enOp  = OP_ADD;   break;
            case TT_SUB:    bind->enOp  = OP_SUB;   break;
            case TT_IADD:   bind->enOp  = OP_IADD;  break;
            case TT_ISUB:   bind->enOp  = OP_ISUB;  break;
            default:
                break;
        }

        bind->sCalc.pLeft   = left;
        bind->sCalc.pRight  = right;
        return bind;
    }

    Expression::binding_t  *Expression::parse_muldiv(tokenizer_t *t, bool get)
    {
        // Parse left part
        binding_t *left = parse_not(t, get);
        if (left == NULL)
            return NULL;

        // Check token
        token_t tok = get_token(t, false);
        switch (tok)
        {
            case TT_MUL:
            case TT_DIV:
            case TT_IMUL:
            case TT_IDIV:
            case TT_MOD:
                break;
            default:
                return left;
        }

        // Parse right part
        binding_t *right = parse_muldiv(t, true);
        if (right == NULL)
        {
            destroy_data(left);
            return NULL;
        }

        // Create binding between left and right
        binding_t *bind     = new binding_t;
        if (bind == NULL)
        {
            destroy_data(left);
            destroy_data(right);
            return NULL;
        }

        switch (tok)
        {
            case TT_MUL:    bind->enOp = OP_MUL;    break;
            case TT_DIV:    bind->enOp = OP_DIV;    break;
            case TT_IMUL:   bind->enOp = OP_IMUL;   break;
            case TT_IDIV:   bind->enOp = OP_IDIV;   break;
            case TT_MOD:    bind->enOp = OP_MOD;    break;
            default:
                break;
        }

        bind->sCalc.pLeft   = left;
        bind->sCalc.pRight  = right;
        return bind;
    }

    Expression::binding_t  *Expression::parse_not(tokenizer_t *t, bool get)
    {
        // Check token
        token_t tok = get_token(t, get);

        // Parse right part
        binding_t *right =
            ((tok == TT_NOT) || (tok == TT_BNOT)) ? parse_not(t, true) :
            parse_sign(t, false);
        if ((right == NULL) || (tok != TT_NOT) || (tok != TT_BNOT))
            return right;

        // Create binding between left and right
        binding_t *bind     = new binding_t;
        if (bind == NULL)
        {
            destroy_data(right);
            return NULL;
        }
        bind->enOp          = (tok == TT_NOT) ? OP_NOT : OP_BNOT;
        bind->sCalc.pLeft   = right;
        bind->sCalc.pRight  = NULL;
        return bind;
    }

    Expression::binding_t  *Expression::parse_sign(tokenizer_t *t, bool get)
    {
        // Check token
        token_t tok = get_token(t, get);

        // Parse right part
        binding_t *right = NULL;
        switch (tok)
        {
            case TT_ADD:
            case TT_SUB:
            case TT_IADD:
            case TT_ISUB:
                right = parse_sign(t, true);
                break;
            default:
                right = parse_primary(t, false);
                break;
        }
        if ((right == NULL) || (tok != TT_SUB))
            return right;

        // Create binding between left and right
        binding_t *bind     = new binding_t;
        if (bind == NULL)
        {
            destroy_data(right);
            return NULL;
        }
        bind->enOp          = OP_SIGN;
        bind->sCalc.pLeft   = right;
        bind->sCalc.pRight  = NULL;
        return bind;
    }

    Expression::binding_t  *Expression::parse_primary(tokenizer_t *t, bool get)
    {
        token_t tok = get_token(t, get);
        switch (tok)
        {
            case TT_IDENTIFIER:
            {
                binding_t *bind     = new binding_t;
                bind->enOp          = OP_LOAD;
                bind->sLoad.pPort   = pUI->port(t->sText);
                if (bind->sLoad.pPort != NULL)
                {
                    bind->sLoad.pPort->bind(this);
                    bind->sLoad.fValue  = bind->sLoad.pPort->getValue();
                }
                else
                    bind->sLoad.fValue  = 0.0f;
                get_token(t, true);
                return bind;
            }

            case TT_VALUE:
            {
                binding_t *bind     = new binding_t;
                bind->enOp          = OP_LOAD;
                bind->sLoad.pPort   = NULL;
                bind->sLoad.fValue  = t->fValue;
                get_token(t, true);
                return bind;
            }

            case TT_LBRACE:
            {
                binding_t *bind     = parse_expression(t, true);
                if (bind == NULL)
                    return bind;
                tok = get_token(t, false);
                if (tok == TT_RBRACE)
                {
                    get_token(t, true); // TODO: check that really needed
                    return bind;
                }

                destroy_data(bind);
                return NULL;
            }

            case TT_EOF:
                return NULL;

            default:
            case TT_UNKNOWN:
                return NULL;
        }
        return NULL;
    }

    bool Expression::parse(const char *expr)
    {
        // Drop previous data
        destroy_data(pRoot);
        pRoot       = NULL;
        if (expr == NULL)
            return true;

        char *saved_locale = setlocale(LC_NUMERIC, "C");

        // Initialize tokenizer
        tokenizer_t t;
        t.sText[0]  = '\0';
        t.fValue    = 0.0f;
        t.enType    = TT_UNKNOWN;
        t.sUnget    = '\0';
        t.pStr      = expr;

        // Parse expression
        pRoot       = parse_expression(&t, true);
        setlocale(LC_NUMERIC, saved_locale);
        if (pRoot == NULL)
            return false;

        if (get_token(&t, false) != TT_EOF)
        {
            destroy_data(pRoot);
            pRoot = NULL;
            return false;
        }

        return true;
    }
} /* namespace lsp */
