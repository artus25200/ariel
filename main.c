#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  T_RBRAC,
  T_PRINT
} tokentype;

typedef enum {
  O_GLUE,
  O_ADD,
  O_SUB,
  O_MUL,
  O_DIV,
  O_INTLIT,
  O_VAR,
  O_NEGATE,
  O_STRLIT,
  O_IDENT,
  O_PRINT
} ast_op;

int op_pr[] = {0, 10, 10, 20, 20, 0, 0, 30, 0, 0};

int op_priority(ast_op op) {
  int priority = op_pr[op];
  if (priority == 0) {
    exit(1);
  }
  return priority;
}

ast_op token_to_op(tokentype type) {
  switch (type) {
  case T_INTLIT:
    return O_INTLIT;
  case T_STRLIT:
    return O_STRLIT;
  case T_STAR:
    return O_MUL;
  case T_SLASH:
    return O_DIV;
  case T_PLUS:
    return O_ADD;
  case T_MINUS:
    return O_SUB;
  case T_IDENT:
    return O_IDENT;
  case T_PRINT:
    return O_PRINT;
  default:
    exit(1);
  }
}

typedef struct {
  tokentype type;
  union {
    int int_value;
    char *ident;
  };
} token;

typedef struct ast {
  ast_op op;
  struct ast *left;
  struct ast *middle;
  struct ast *right;
  union {
    double value;
    char *ident;
  };
} ast;

struct parser {
  int line;
  int i;
  int token_count;
  token *previous_token;
  token *current_token;
  token *next_token;
};
typedef enum { DOUBLE_RESULT, INT_RESULT, STRING_RESULT } resultType;
typedef struct {
  bool isThereReturn;
  union {
    double double_value;
    int int_value;
    char *string_value;
  };
  resultType resulttype;
  union {
    double double_return;
    int int_return;
    char *string_return;
  };
  resultType returntype;
} interpretResult;

char *input;
FILE *input_file;
int size;
int c = 0;

token *token_list;

void read_file(char *filename) {
  input_file = fopen(filename, "r");
  if (!input_file) {
    fprintf(stderr, "Failed to read file : %s\n", filename);
    exit(1);
  }
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

tokentype keyword(char *ident) {
  switch (*ident) {
  case 'p':
    if (strcmp(ident, "print") == 0)
      return T_PRINT;
  }
  return T_IDENT;
}

char *parse_identifier(char ch) {
  char *ident = malloc(sizeof(char) * MAX_IDENT_SIZE);
  int i = 0;
  while ((isalnum(ch) || '_' == ch) && i < MAX_IDENT_SIZE) {
    ident[i] = ch;
    ++i;
    ch = next();
  }
  previous();

  return ident;
}

int read_tokens(token *list) {
  c = 0;
  char ch;
  int i = 0;
  while ((ch = skip()) != -1) {
    token token;

    switch (ch) {
    case EOF:
      token.type = T_EOF;
      break;
    case ';':
      token.type = T_SEMI;
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
      } else if (isalnum(ch)) {
        token.ident = parse_identifier(ch);
        token.type = keyword(token.ident);
        break;
      }
      exit(1);
    }
    printf("[token %d", token.type);
    if (token.type == T_INTLIT) {
      printf(" value : %d", token.int_value);
    }
    printf("]\n");
    if (list != NULL) {
      list[i] = token;
    }
    ++i;
  }
  c = 0;
  return i;
}

ast *make_ast_ternary(ast_op op, ast *left, ast *middle, ast *right,
                      double value, char *ident) {
  ast *ast = malloc(sizeof(struct ast));
  ast->op = op;
  ast->left = left;
  ast->right = right;
  ast->middle = middle;
  if (op == O_INTLIT) {
    ast->value = value;
  } else if (op == O_VAR || op == O_STRLIT || op == O_IDENT) {
    ast->ident = malloc(sizeof(char) * strlen(ident) + 1);
    strcpy(ast->ident, ident);
  }
  return ast;
}

ast *make_ast(ast_op op, ast *left, ast *right, double value, char *ident) {
  return make_ast_ternary(op, left, NULL, right, value, ident);
}

ast *make_ast_unary(ast_op op, ast *left, double value, char *ident) {
  return make_ast(op, left, NULL, value, ident);
}

ast *make_ast_number(double number) {
  return make_ast_unary(O_INTLIT, NULL, number, NULL);
}

ast *make_ast_var(char *ident) { return make_ast_unary(O_VAR, NULL, 0, ident); }

ast *glue_ast(ast *tree) {
  tree->right = malloc(sizeof(ast));
  tree->right->op = O_GLUE;
  return tree->right;
}

struct parser *parser;
void next_token() {
  parser->previous_token = parser->current_token;
  parser->current_token = parser->next_token;
  if (token_list[parser->i].type != T_EOF) {
    parser->next_token = &token_list[(parser->i + 2)];
  }
}

void match(tokentype type, char *expected) {
  if (parser->current_token->type != type) {
    fprintf(stderr, "Expected token \"%s\"", expected);
    exit(1);
  }
  next_token();
}

void rpar() { match(T_RPAR, ")"); }
void rbrac() { match(T_RBRAC, "}"); }
void semi() { match(T_SEMI, ";"); }
ast *statement();
ast *binary_expression(int pp);
ast *unary_expression() {
  ast *node;
  switch (parser->current_token->type) {
  case T_MINUS:
    next_token();
    node = make_ast_unary(O_NEGATE, unary_expression(), 0, NULL);
    break;
  case T_INTLIT:
    node =
        make_ast_unary(O_INTLIT, NULL, parser->current_token->int_value, NULL);
    next_token();
    break;
  case T_STRLIT:
    node = make_ast_unary(O_STRLIT, NULL, 0, parser->current_token->ident);
    next_token();
    break;
  case T_LPAR:
    next_token();
    node = binary_expression(0);
    rpar();
    break;
  case T_LBRAC:
    next_token();
    node = statement();
    rbrac();
    break;
  case T_PRINT:
    next_token();
    node = binary_expression(0);
    node = make_ast_unary(O_PRINT, node, 0, NULL);
    break;
  default:
    fprintf(stderr, "Syntax Error\n");
    exit(1);
  }
  return node;
}

ast *binary_expression(int pp) {
  ast *left, *right;
  left = unary_expression();
  if (parser->current_token->type == T_SEMI ||
      parser->current_token->type == T_EOF ||
      parser->current_token->type == T_RPAR)
    return left;

  int token_type = parser->current_token->type;
  while (op_priority(token_type) > pp) {
    next_token();
    right = binary_expression(op_priority(token_type));
    left = make_ast(token_type, left, right, 0, NULL);

    token_type = parser->current_token->type;
    if (parser->current_token->type == T_SEMI ||
        parser->current_token->type == T_EOF ||
        parser->current_token->type == T_RPAR)
      return left;
  }
  return left;
}

ast *statement() {
  ast *main_tree = malloc(sizeof(ast));
  main_tree->op = O_GLUE;
  ast *current_tree = main_tree;
  while (parser->current_token->type != T_EOF || parser->current_token->type ||
         T_RBRAC) {
    current_tree->left = binary_expression(0);
    semi();
    current_tree = glue_ast(current_tree);
  }
  return main_tree;
}

void free_ast(ast *tree) {
  if (tree->left)
    free_ast(tree->left);
  if (tree->right)
    free_ast(tree->right);
  free(tree);
}

interpretResult interpret_ast(ast *tree) {
  interpretResult result;
  switch (tree->op) {
  case O_GLUE:
    result = interpret_ast(tree->left);
    if (result.isThereReturn)
      return result;
    if (!tree->right)
      break;
    result = interpret_ast(tree->right);
    if (result.isThereReturn)
      return result;
    break;
  case O_INTLIT:
    // TODO: add double support
    result.resulttype = INT_RESULT;
    result.int_value = (int)tree->value;
    break;
  case O_PRINT:
    result = interpret_ast(tree->left);
    switch (result.resulttype) {
    case DOUBLE_RESULT:
      printf("%f", result.double_value);
      break;
    case INT_RESULT:
      printf("%d", result.int_value);
      break;
    case STRING_RESULT:
      printf("%s", result.string_value);
      break;
    }
    result.isThereReturn = false;
  case O_ADD: {
    interpretResult resultleft = interpret_ast(tree->left);
    interpretResult resultright = interpret_ast(tree->right);
    resultType lefttype = resultleft.resulttype;
    resultType righttype = resultright.resulttype;
    if (righttype != STRING_RESULT && lefttype != STRING_RESULT) {
      double add_result = lefttype == INT_RESULT ? resultleft.int_value
                          : resultleft.double_value + righttype == INT_RESULT
                              ? resultright.int_value
                              : resultright.double_value;
      result.resulttype =
          (((int)add_result) == add_result) ? INT_RESULT : DOUBLE_RESULT;
      if (result.resulttype == INT_RESULT) {
        result.int_value = (int)add_result;
      } else {
        result.double_value = add_result;
      }
    } else if (righttype == STRING_RESULT && lefttype == STRING_RESULT) {
      // Concatenate strings
      char *string_result =
          malloc(sizeof(char) * (strlen(resultright.string_value) +
                                 strlen(resultleft.string_value) + 1));
      strcpy(string_result, resultleft.string_value);
      strcat(string_result, resultright.string_value);
      result.resulttype = STRING_RESULT;
      result.string_value = string_result;
    } else {
      // TODO: add type debugging
      fprintf(stderr, "Cannot add type ... and type ...\n");
      exit(1);
    }
    break;
  }
  }
  return result;
}

int main(int argc, char **argv) {
  if (argc < 2)
    return 1;
  read_file(argv[1]);
  int token_count = read_tokens(NULL);
  if (token_count < 2)
    exit(1);
  token_list = malloc(sizeof(token) * token_count);
  read_tokens(token_list);
  parser = malloc(sizeof(struct parser));
  parser->token_count = token_count;
  parser->current_token = &token_list[0];
  parser->next_token = &token_list[1];
  parser->i = 0;
  ast *tree = statement();
  interpret_ast(tree);
  free_ast(tree);
  return 0;
}
