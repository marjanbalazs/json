// Json parser library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// First lets focus on parsing the valid inputs

/*
typedef struct logger {
    bool open;
    FILE *log_file;
} logger;

bool open_logger(logger *lg)
{
    if ( !lg->open ) {
        lg->log_file = fopen("json_log.txt", "rw+");
        if (lg->log_file != NULL ) {
            lg->open = TRUE;
        }
    }
    return lg->open;
}

void log(logger *lg, char* str, size_t len)
{
    fwrite(str, 1, lg->log_file, len);
}
*/

/* The possible states */
enum stack_state {
    STACK_OBJECT,
    STACK_ARRAY,
    STACK_KEY,
    STACK_VALUE,
    STACK_NONE = -1
};

typedef struct stack {
    enum stack_state *stack;
    size_t stack_size;
    size_t stack_cap;
} stack;

void init_stack(stack *s)
{
    s->stack = malloc(sizeof(enum stack_state) * 10);
    s->stack_cap = 10;
    s->stack_size = 0;
}

enum stack_state stack_peek(stack *s)
{
    if ( s->stack_size != 0) {
        return s->stack[s->stack_size - 1];
    } else {
        return STACK_NONE;
    }
}

enum stack_state stack_pop(stack *s)
{
    --(s->stack_size);
    return s->stack[(s->stack_size)];
}

void stack_push(stack *s, enum stack_state elem)
{
    s->stack[s->stack_size] = elem;
    s->stack_size++;
    if (s->stack_size == s->stack_cap) {
        enum stack_state *ptr = malloc(sizeof(enum stack_state) * s->stack_cap * 2);
        memcpy((void *)ptr, (void *)s->stack, s->stack_size);
        free(s->stack);
        s->stack = ptr;
    }
}

/* The possible tokens */
enum json_token_state {
    START = 0,
    WHITESPACE = 1,
    KEY = 2,
    COLON = 3,
    COMMA = 4,
    ARRAY = 5,
    VALUE_STRING = 6,
    VALUE_NUMBER = 7,
    T,
    TR,
    TRU,
    F,
    FA,
    FAL,
    FALS,
    N,
    NU,
    NUL
};

typedef struct json_parser {
    enum json_token_state state;
} json_parser;

struct stack s;

int json_parse(json_parser *jp, const char json[], const size_t size)
{
    size_t cursor = 0;
    while (cursor < size )
    {
        switch (json[cursor])
        {
            case '{':
                // Start of object parsing
                stack_push(&s, STACK_OBJECT);
                printf("OBJECT pushed to stack.\n");
                jp->state = START;
                break;
            case '}':
                // End of object parsing
                if (STACK_OBJECT != stack_pop(&s)) {
                    printf("The last stack object wasn't an OBJECT\n");
                    return -1;
                } else {
                    printf("OBJECT popped\n");
                }
                break;
            case '[':
                stack_push(&s, STACK_ARRAY);
                jp->state = STACK_ARRAY;
                break;
            case ']':
                if (STACK_ARRAY != stack_pop(&s)) {
                    printf("The last object wasn't an ARRAY\n");
                    return -1;
                }
                break;
            case '\t':
            case '\r':
            case '\n':
                switch (jp->state) 
                {
                    case VALUE_STRING:
                        printf("NO whitespace other than space in a string\n");
                        return -1;
                    default:
                        break;
                }
                break;
            case ' ':
                break;
            case '/':
            case ',':
                switch(jp->state)
                {
                    case ARRAY:
                        // We are still parsing an array, next char is a whitespace so go ahead
                        break;
                    case START:
                        jp->state = START;
                        break;
                    default:
                        printf("Error with ,\n");
                        return -1;
                }
                break;
            case ':':
                jp->state = COLON;
                break;
            case '\"':
                switch (jp->state)
                {
                    case START:
                        // Start of KEY
                        jp->state = KEY;
                        break;
                    case KEY:
                        // End of KEY
                        jp->state = WHITESPACE;
                        break;
                    case COLON:
                        // Start of VALUE_STRING
                        jp->state = VALUE_STRING;
                        break;
                    case VALUE_STRING:
                        jp->state = START;
                        break;
                    default:
                        break;
                }
                break;
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
                        stack_push(&s, STACK_VALUE);
                        jp->state = VALUE_NUMBER;
                        break;
                    default:
                        break;
                }
                break;
            case '+':
            case '.':
            case 't':
                switch (jp->state)
                {
                case COLON:
                    jp->state = T;
                    break;
                
                default:
                    break;
                }
                break;
            case 'r':
                switch (jp->state)
                {
                case T:
                    jp->state = TR;
                    break;
                default:
                    break;
                }
                break;
            case 'u':
                switch (jp->state)
                {
                case N:
                    jp->state = NU;
                    break;
                case TR:
                    jp->state = TRU;
                    break;
                default:
                    break;
                }
                break;
            case 'e':
                switch (jp->state)
                {
                case TRU:
                    // True found
                    break;
                case FALS:
                    /// False found;
                    break;
                default:
                    break;
                }
                break;
            case 'f':
            case 'a':
            case 'l':
                switch (jp->state)
                {
                case NU:
                    jp->state = NUL;
                    break;
                case NUL:
                    // We found a NULL
                    break;
                case FA:
                    jp->state = FAL;
                    break;
                default:
                    break;
                }
                break;
            case 's':
                switch (jp->state)
                {
                case FAL:
                    jp->state = FALS;
                    break;
                default:
                    break;
                }
                break;
            case 'n':
                switch (jp->state)
                {
                case COLON:
                    jp->state = N;
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
        }
        printf("Char: %c TokenState: %d StackState: %d Cursor: %ld\n", json[cursor], jp->state, stack_peek(&s), cursor);
        cursor++;
    }
    return 0;
}

int main(void)
{
    char test_str[500] = "{\"name\" : \"balazs\", \"name\" : \"lili puha\"}";
    printf("Len %ld\n", strlen(test_str));

    json_parser jp;
    memset(&jp, 0, sizeof(json_parser));
    init_stack(&s);

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
