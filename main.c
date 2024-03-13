#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_IDENT_SIZE 50

typedef enum {
  T_EOF,
  T_UNKNOWN,
  T_INTLIT,
  T_STRLIT,
  T_STAR,
  T_SLASH,
  T_PLUS,
  T_MINUS,
  T_IDENT,
  T_SEMI,
  T_LPAR,
  T_RPAR,
  T_LBRAC,
  T_RBRAC
} tokentype;

typedef struct {
  tokentype type;
  union {
    int int_value;
    char ident[50];
  };
} token;

char *input;
FILE *input_file;
int size;
int c = 0;

token *token_list;

void read_file(char *filename) {
  input_file = fopen(filename, "r");
  fseek(input_file, 0, SEEK_END);
  size = ftell(input_file);
  fseek(input_file, 0, SEEK_SET);
  input = malloc(sizeof(char) * (size + 1));
  fread(input, size, 1, input_file);
}

char next() {
  if (c > size - 1)
    return -1;
  char ch = input[c];
  ++c;
  return ch;
}

char previous() {
  --c;
  return input[c];
}

char skip() {
  char ch;
  do {
    ch = next();
  } while (ch == ' ' || ch == '\n' || ch == '\t');
  return ch;
}

double parse_int(char ch) {
  int value = 0;
  while (isdigit(ch)) {
    value *= 10;
    value += ch - '0';
    ch = next();
  }
  previous();
  return value;
}

int read_tokens(token *list) {
  c = 0;
  char ch;
  int i = 0;
  while ((ch = skip()) != -1) {
    token token;

    switch (ch) {
    case '\0':
      token.type = T_EOF;
      break;
    case '+':
      token.type = T_PLUS;
      break;
    case '-':
      token.type = T_MINUS;
      break;
    case '*':
      token.type = T_STAR;
      break;
    case '/':
      token.type = T_SLASH;
      break;
    default:
      if (isdigit(ch)) {
        token.type = T_INTLIT;
        token.int_value = parse_int(ch);
        break;
      }
      exit(1);
    }
    if (list != NULL) {
      list[i] = token;
    }
    ++i;
  }
  c = 0;
  return i;
}

int main(int argc, char **argv) {
  if (argc < 2)
    return 1;
  read_file(argv[1]);
  int token_count = read_tokens(NULL);
  token_list = malloc(sizeof(token) * token_count);
  read_tokens(token_list);
  for (int i = 0; i < token_count; ++i) {
    printf("%d ", token_list[i].type);
    if (token_list[i].type == T_INTLIT)
      printf("(%d) ", token_list[i].int_value);
  }
  return 0;
}
