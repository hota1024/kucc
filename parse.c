# include "kucc.h"

// エラーの箇所を報告する
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");

  exit(1);
}

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");

  exit(1);
}

// 次のトークンが期待している記号のときには、とーくんを1つよみ勧めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op)
{
  if (
    token->kind != TK_RESERVED ||
    strlen(op) != token->len ||
    memcmp(token->str, op, token->len)
  )
  {
    return false;
  }

  token = token->next;

  return true;
}

// 次のトークンが期待している記号のときには、とーくんを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op)
{
  if (
    token->kind != TK_RESERVED ||
    strlen(op) != token->len ||
    memcmp(token->str, op, token->len)
  )
  {
    error_at(token->str, "'%c'ではありません", op);
  }

  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number()
{
  if (token->kind != TK_NUM)
  {
    error_at(token->str, "数ではありません");
  }

  int val = token->val;
  token = token->next;

  return val;
}

// 次のトークンがEOFなら真を、でなければ偽を返す。
bool at_eof()
{
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる。
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;

  cur->next = tok;

  return tok;
}

bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

// 入力文字列pをトークナイズしてそれを返す。
Token *tokenize(char *p)
{
  Token head;
  head.next = NULL;

  Token *cur = &head;

  while (*p)
  {
    if (isspace(*p))
    {
      p++;

      continue;
    }

    if ('a' <= *p && *p <= 'z')
    {
      cur = new_token(TK_IDENT, cur, p++);
      cur->len = 1;

      continue;
    }

    if (
      startswith(p, "==") ||
      startswith(p, "!=") ||
      startswith(p, "<=") ||
      startswith(p, ">=")
    )
    {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;

      continue;
    }

    if (strchr("+-*/()<>", *p))
    {
      cur = new_token(TK_RESERVED, cur, p++, 1);

      continue;
    }

    if (isdigit(*p))
    {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;

      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);

  return head.next;
}

// 新しいノードを作成する。
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;

  return node;
}

// 整数ノードを作成する。
Node *new_node_num(int val)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;

  return node;
}

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// 式をパースする。
Node *expr()
{
  return equality();
}

Node *equality()
{
  Node *node = relational();

  for (;;)
  {
    if (consume("=="))
    {
      node = new_node(ND_EQ, node, relational());
    }
    else if (consume("!="))
    {
      node = new_node(ND_NE, node, relational());
    }
    else
    {
      return node;
    }
  }
}

Node *relational()
{
  Node *node = add();

  for (;;)
  {
    if (consume("<"))
    {
      node = new_node(ND_LT, node, add());
    }
    else if (consume("<="))
    {
      node = new_node(ND_LE, node, add());
    }
    else if (consume(">"))
    {
      node = new_node(ND_LT, add(), node);
    }
    else if (consume(">="))
    {
      node = new_node(ND_LE, add(), node);
    }
    else
    {
      return node;
    }
  }
}

// 足し算と引き算をパースする
Node *add()
{
  Node *node = mul();

  for (;;)
  {
    if (consume("+"))
    {
      node = new_node(ND_ADD, node, mul());
    } else if (consume("-"))
    {
      node = new_node(ND_SUB, node, mul());
    } else
    {
      return node;
    }
  }
}

// 掛け算とわり算をパースする。
Node *mul()
{
  Node *node = unary();

  for (;;)
  {
    if (consume("*"))
    {
      node = new_node(ND_MUL, node, unary());
    } else if (consume("/"))
    {
      node = new_node(ND_DIV, node, unary());
    } else
    {
      return node;
    }
  }
}

Node *unary()
{
  if (consume("+"))
  {
    return primary();
  }

  if (consume("-"))
  {
    return new_node(ND_SUB, new_node_num(0), primary());
  }

  return primary();
}

// グループ化と整数をパースする。
Node *primary()
{
  if (consume("("))
  {
    Node *node = expr();
    expect(")");

    return node;
  }

  return new_node_num(expect_number());
}
