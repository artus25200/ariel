#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_IDENT_SIZE 50
#define MAX_STR_SIZE 2000

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
  T_COMMA,
  // KEYWORDS
  T_PRINT,
  T_NEWLINE
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
  O_TUPLE, // arg1, arg2, arg3
  O_PRINT
} ast_op;

int op_pr[] = {0, 10, 10, 20, 20, 0, 0, 30, 0, 0, 5, 0};

int op_priority(ast_op op) {
  int priority = op_pr[op];
  if (priority == 0) {
    fprintf(stderr, "Wrong operand, op : %d\n", op);
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
  case T_COMMA:
    return O_TUPLE;
  default:
    fprintf(stderr, "Token is not an operand : %d\n", type);
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
typedef enum { NUMBER, STRING, TUPLE } resultType;
typedef struct interpretResult {
  bool isReturn;
  resultType type;
  union {
    double number;
    char *string;
  };
  ast *tuple_left;
  ast *tuple_right;
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
  if (ch == '/') {
    if ((ch = next()) == '/') {
      while ((ch = next()) != '\n') {
      }
      ch = skip();
    } else {
      previous();
      ch = '/';
    }
  }
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
  case 'n':
    if (strcmp(ident, "nl") == 0 || strcmp(ident, "newline") == 0)
      return T_NEWLINE;
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
  char ident[MAX_IDENT_SIZE];
  char string[MAX_STR_SIZE];
  while ((ch = skip()) != -1) {
    token token;

    switch (ch) {
    case EOF:
      token.type = T_EOF;
      break;
    case ';':
      token.type = T_SEMI;
      break;
    case ',':
      token.type = T_COMMA;
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
    case '(':
      token.type = T_LPAR;
      break;
    case ')':
      token.type = T_RPAR;
      break;
    case '{':
      token.type = T_LBRAC;
      break;
    case '}':
      token.type = T_RBRAC;
      break;
    case '"': {
      int ic = 0;
      while ((ch = next()) != '"' && ch != EOF) {
        if (ic >= MAX_STR_SIZE - 1) {
          fprintf(stderr, "Max string size (2000) exceeded.\n");
          exit(1);
        }
        string[ic] = ch;
        ++ic;
      }
      string[ic] = '\0';
      token.type = T_STRLIT;
      token.ident = malloc(sizeof(char) * (strlen(string) + 1));
      strcpy(token.ident, string);
      break;
    }
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
      fprintf(stderr, "Unknow token : %c", ch);
      exit(1);
    }
    /* printf("[token %d", token.type); */
    /* if (token.type == T_INTLIT) { */
    /*   printf(" value : %d", token.int_value); */
    /* } */
    /* printf("]\n"); */
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
  parser->i++;
  parser->previous_token = parser->current_token;
  parser->current_token = parser->next_token;
  if (parser->current_token->type != T_EOF) {
    parser->next_token = &token_list[(parser->i + 1)];
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
void lbrac() { match(T_LBRAC, "{"); }
void semi() { match(T_SEMI, ";"); }
void comma() { match(T_COMMA, ","); }

ast *compound_statement(bool is_top_level);
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
  case T_NEWLINE:
    node = make_ast_unary(O_STRLIT, NULL, 0, "\n");
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
    node = compound_statement(false);
    break;
  case T_PRINT:
    next_token();
    node = binary_expression(0);
    node = make_ast_unary(O_PRINT, node, 0, NULL);
    while (parser->current_token->type != T_SEMI &&
           parser->current_token->type != T_EOF) {
      match(T_COMMA, ",");
      node = make_ast(O_GLUE, node,
                      make_ast_unary(O_PRINT, binary_expression(0), 0, NULL), 0,
                      NULL);
    }
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
  if (parser->current_token->type == T_RBRAC)
    semi(); // will throw an error

  int token_type = parser->current_token->type;
  while (op_priority(token_to_op(token_type)) > pp) {
    next_token();
    right = binary_expression(op_priority(token_to_op(token_type)));
    left = make_ast(token_to_op(token_type), left, right, 0, NULL);

    token_type = parser->current_token->type;
    if (parser->current_token->type == T_SEMI ||
        parser->current_token->type == T_EOF ||
        parser->current_token->type == T_RPAR)
      return left;
    if (parser->current_token->type == T_RBRAC)
      semi(); // will throw an error
  }
  return left;
}

ast *statement() {
  ast *tree;
  tree = binary_expression(0);
  return tree;
}

ast *compound_statement(bool is_top_level) {
  if (!is_top_level)
    lbrac();

  ast *main_tree = NULL;
  ast *right = NULL;
  while (1) {
    if (parser->current_token->type == T_EOF ||
        parser->current_token->type == T_RBRAC)
      break;
    right = statement();
    semi();
    if (right != NULL) {
      if (main_tree == NULL)
        main_tree = right;
      else
        main_tree = make_ast(O_GLUE, main_tree, right, 0, NULL);
    }
  }
  if (!is_top_level)
    rbrac();
  return main_tree;
}

void free_ast(ast *tree) {
  if (tree->left)
    free_ast(tree->left);
  if (tree->right)
    free_ast(tree->right);
  free(tree);
}
void print_result(interpretResult result);
interpretResult interpret_ast(ast *tree) {
  interpretResult result;
  result.isReturn = false;
  if (tree == NULL)
    return result;
  interpretResult resultleft = interpret_ast(tree->left);
  interpretResult resultright = interpret_ast(tree->right);
  resultType lefttype = resultleft.type;
  resultType righttype = resultright.type;
  bool sub = false;
  switch (tree->op) {
  case O_GLUE:
    if (resultleft.isReturn)
      return resultleft;
    if (resultright.isReturn)
      return resultright;
    break;
  case O_INTLIT:
    result.type = NUMBER;
    result.number = tree->value;
    break;
  case O_STRLIT:
    result.type = STRING;
    result.string = tree->ident;
    break;
  case O_TUPLE:
    result.type = TUPLE;
    result.tuple_left = tree->left;
    result.tuple_right = tree->right;
    break;
  case O_PRINT:
    print_result(resultleft);
    break;
  case O_SUB:
    sub = true;
  case O_ADD: {
    if (lefttype == righttype && lefttype == NUMBER) {
      if (sub)
        result.number = resultleft.number - resultright.number;
      else
        result.number = resultleft.number + resultright.number;
      result.type = NUMBER;
      result.isReturn = false;
    } else if (righttype == lefttype && lefttype == STRING && sub == false) {
      // Concatenate strings
      char *string_result =
          malloc(sizeof(char) *
                 (strlen(resultright.string) + strlen(resultleft.string) + 1));
      strcpy(string_result, resultleft.string);
      strcat(string_result, resultright.string);
      result.type = STRING;
      result.string = string_result;
    } else {
      // TODO: add type debugging
      fprintf(stderr, "Cannot add type ... and type ...\n");
      exit(1);
    }
    break;
  }
  case O_MUL: {
    if (lefttype == righttype && lefttype == NUMBER) {
      result.type = NUMBER;
      result.number = resultleft.number * resultright.number;
      result.isReturn = false;
      break;
    } else if ((lefttype == NUMBER && righttype == STRING) ||
               (lefttype == STRING && righttype == NUMBER)) {
      result.type = STRING;
      char *string =
          lefttype == NUMBER ? resultright.string : resultleft.string;
      double times =
          lefttype == NUMBER ? resultleft.number : resultright.number;
      if (times < 0) {
        fprintf(stderr,
                "WARN : Cannot Multiply a string by a negative number !\n");
        result.string = "";
        break;
      }
      if (times != (int)times)
        fprintf(stderr,
                "WARN : Cannot multiply a string by %g, this number will "
                "automatically be converted to %d\n",
                times, (int)times);
      times = (int)times;
      char *result_string = malloc(sizeof(char) * (strlen(string) * times + 1));
      result_string[0] = '\0';
      for (int i = 0; i < times; ++i) {
        strcat(result_string, string);
      }
      result.string = result_string;
    } else {
      // TODO: add type debugging
      fprintf(stderr, "Cannot add type ... and type ...\n");
      exit(1);
    }
    break;
  }
  case O_DIV:
    if (lefttype != NUMBER || righttype != NUMBER) {
      // TODO: add type debugging
      fprintf(stderr, "Cannot divide type ... by type ...\n");
      exit(1);
    }
    result.type = NUMBER;
    result.number = resultleft.number / resultright.number;
    break;
  case O_NEGATE:
    result = resultleft;
    result.number = -result.number;
    break;

  default:
    fprintf(stderr, "WARN : op not implemented, skipped.\n");
  }
  return result;
}

void print_result(interpretResult result) {
  switch (result.type) {
  case TUPLE:
    print_result(interpret_ast(result.tuple_left));
    print_result(interpret_ast(result.tuple_right));
    break;
  case NUMBER:
    printf("%g", result.number);
    break;
  case STRING: {
    int len = strlen(result.string);
    for (int i = 0; i < len; ++i) {
      switch (result.string[i]) {
      case '\\':
        ++i;
        switch (result.string[i]) {
        case '\\':
          putchar('\\');
          break;
        case 'n':
          putchar('\n');
          break;
        case 't':
          putchar('\t');
          break;
        default:
          fprintf(stderr, " WARN : Unrecognized escape sequence : \\%c\n",
                  result.string[i]);
        }
        break;
      default:
        putchar(result.string[i]);
        break;
      }
    }
    return;
  }
  }
}
void print_ast(ast *tree, int depth) {
  if (tree == NULL)
    return;
  for (int i = 0; i < depth; ++i) {
    printf("  ");
  }
  printf("%d\n", tree->op);
  print_ast(tree->left, depth + 1);
  print_ast(tree->right, depth + 1);
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
  ast *tree = compound_statement(true);
  /* print_ast(tree, 0); */
  interpret_ast(tree);
  free_ast(tree);
  return 0;
}
