// Json parser library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// First lets focus on parsing the valid inputs

/* The possible states */
enum json_state
{
    OBJECT_S,
    ARRAY_S,
    KEY_S,
    VALUE_S
};

typedef struct stack
{
    enum json_state *stack;
    size_t stack_size;
    size_t stack_cap;
} stack;

void init_stack(stack *s)
{
    s->stack = malloc(sizeof(enum json_state) * 10);
    s->stack_cap = 10;
    s->stack_size = 0;
}

enum json_state stack_peek(stack *s)
{
    return s->stack[s->stack_size - 1];
}

enum json_state stack_pop(stack *s)
{
    --(s->stack_size);
    return s->stack[(s->stack_size)];
}

void stack_push(stack *s, enum json_state elem)
{
    s->stack[s->stack_size] = elem;
    s->stack_size++;
    if (s->stack_size == s->stack_cap)
    {
        enum json_state *ptr = malloc(sizeof(enum json_state) * s->stack_cap * 2);
        memcpy((void *)ptr, (void *)s->stack, s->stack_size);
        free(s->stack);
        s->stack = ptr;
    }
}

/* The possible tokens */
enum json_token_state
{
    START,
    WHITESPACE,
    KEY,
    COLON,
    VALUE,
    COMMA,
    ARRAY,
    NUMBER,
    STRING,
    TRUE,          /* true */
    FALSE,         /* false */
    NULLVAL,       /* null */
    QUOTE,         /* " */
    RSOLIDUS,      /* \ */
    SOLIDUS,       /* / */
    CHAR,          /* alphabet */
    HEX_A,
    HEX_B,
    HEX_C,
    HEX_D,
    HEX_E,
    HEX_F,
    BACKSPACE,       /* b */
    FORM_FEED,       /* f */
    LINE_FEED,       /* n */
    CARRIAGE_RETURN, /* r */
    HORIZONTAL_TAB,  /* t */
    U_HEX,           /* u */
    ZERO,            /* 0 */
    DIGIT,           /* 1-9 */
    MINUS,           /* - */
    PLUS,            /* + */
    DOT,             /* . */
};

typedef struct json_parser
{
    enum json_token_state state;
} json_parser;

struct stack s;

int json_parse(json_parser *jp, const char json[], const size_t size)
{
    size_t cursor = 0;

    while (cursor++ < size )
    {
        switch (json[cursor])
        {
            
            case '{':
                // Start of object parsing
                stack_push(&s, OBJECT_S);
                printf("Pushed to stack OBJECT\n");
                jp->state = START;
                break;
            case '}':
                // End of object parsing
                if (OBJECT_S != stack_pop(&s)) {
                    // error
                    printf("The last object wasn't an object\n");
                    return -1;
                }
                break;
            case '[':
                stack_push(&s, ARRAY_S);
                jp->state = ARRAY_S;
                break;
            case ']':
                if (ARRAY_S!= stack_pop(&s)) {
                    printf("The last object wasn't an array\n");
                    return -1;
                }
            /* Whitespace characters are treated the same besides */
            case '\t':
            case '\r':
            case '\n':
                switch (jp->state)
                {
                    case STRING:
                        printf("NO whitespace other than space in a string\n");
                        return -1;
                }
                break;
            case ' ':
                switch(jp->state)
                {
                    case STRING:
                        break;
                    default:
                        break;
                }
                break;
            case '/':
            case ',':
                switch(jp->state)
                {
                    case ARRAY:
                    // We are still parsing an array, next char is a whitespace so go ahead
                        break;
                    case VALUE:
                        break;
                    default:
                        printf("Error with ,\n");
                        return -1;
                }
            case ':':
                jp->state = COLON;
                break;
            case '\"':
                switch (jp->state)
                {
                    case OBJECT_START:
                        jp->state = KEY;
                        break;
                    case KEY:
                        jp->state = WHITESPACE;
                        break;
                    case COLON:
                        jp->state = VALUE;
                        break;
                    case STRING:
                        break;
                }

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '-':
                switch (jp->state)
                {
                    case COLON:
                        stack_push(&s, VALUE);
                        jp->state = NUMBER;
                        break;
                }
            case '+':
            case '.':
            case 'a':
            case 'b':
            case 'e':
            case 'E':
            case 'f':
            case 'l':
            case 'n':
            case 'r':
            case 's':
            default:
            break;
        }
        printf("Char: %c Jp_state: %d Cursor: %d\n", json[cursor], jp->state, cursor);
    }
    return 0;
}

int main(void)
{
    char test_str[500] = "{ \"name\" : \"balazs\", \"name\" : \"lili\" }";
    printf("Len %d\n", strlen(test_str));

    json_parser jp;
    memset(&jp, 0, sizeof(json_parser));

    int res = json_parse(&jp, test_str, strlen(test_str));

    switch (res)
    {
        case 0:
            printf("Successful parsing\n");
            break;
        default:
            printf("Unsuccessful parsing\n");
            break;
    }

    return EXIT_SUCCESS;
}
