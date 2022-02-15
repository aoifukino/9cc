#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

//トークンの種別
typedef enum{
	TK_RESERVED, //記号                         0
	TK_NUM,      //整数のトークン                1
	TK_EOF,      //入力の終わりを表すトークン	2
} TokenKind;      //定義と同時に変数を宣言

typedef struct Token Token;

//トークン型
struct Token{
	TokenKind kind; //トークンの型
	Token *next;    //次の入力トークン
	int val;      //数値(整数トークンであれば)
	char *str;    //トークン文字列
};

//Input program
char *user_input;

//現在着目しているトークン
Token *token;



//抽象構文木のノードの種類
typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

//抽象構文木のノードの型
struct Node {
	NodeKind kind;
	Node *lhs; //左辺(left-hand side)
	Node *rhs; //右辺(right-hand side)
	int val;   //kindがND_NUMの場合のみ使う
};


//エラーを報告するための関数
//printfと同じ引数を取る
void error(char *fmt,...){
	//可変長引数
	va_list ap;
	//可変長引数を1個の変数にまとめる
	va_start(ap,fmt);

	vfprintf(stderr,fmt,ap);
	exit(1);
}

//Reports an error location and exit
void error_at(char *loc,char *fmt,...){
	va_list ap;
	va_start(ap,fmt);
	
	int pos = loc - user_input;
	fprintf(stderr,"%s\n",user_input);
	fprintf(stderr,"%*s",pos,"");
	fprintf(stderr,"^ ");
	vfprintf(stderr,fmt,ap);
	fprintf(stderr,"\n");
	exit(1);
}

//次のトークンが期待している記号のときには、トークンを1つ読み進めて
//真を返す。それ以外の時は偽を返す。
bool consume(char op){
	if(token -> kind != TK_RESERVED || token -> str[0] != op)
		return false;
	token = token->next;
	return true;
}

//次のトークンが期待している記号のときには、トークンを1つ読み進める
//それ以外の場合にはエラーを報告する。
void expect(char op){
	if(token -> kind != TK_RESERVED || token -> str[0] != op)
		error_at(token->str,"'%c'ではありません",op);
	token = token->next;
}

//次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
//それ以外の場合にはエラーを報告する。
int expect_number(){
	if(token->kind != TK_NUM)
		error_at(token->str,"数ではではありません");
	//printf(" next:%p",token->next);
	int val = token->val;
	token = token->next; //次の入力トークンをtokenに入れる
	return val;
}

bool at_eof(){
	//printf("token_kind:%d",token->kind);
	return token->kind == TK_EOF;
}

//新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind,Token *cur,char *str){
	//Tokenのサイズ文メモリを確保(0で初期化)
	Token *tok = calloc(1,sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok; //nextにtokのアドレスを追加
	return tok;//次のTokenの情報を格納するポインタを返す
}

//入力文字列pをトークナイズしてそれを返す
//Tokenを宣言し、入力された値を1つづつ連結(それぞれに)
Token *tokenize(){
	char *p = user_input;
	Token head;
	head.next = NULL;
	Token *cur = &head; //headの値をcurで参照

	while (*p){
		//空白文字をスキップ
		if(isspace(*p)){ //空白ならtrue
			p++;
			continue;
		}

		if(strchr("+-*/()",*p)){
			//cur->nextを設定して、それをcurに代入している。
			cur = new_token(TK_RESERVED,cur,p++);
			continue;
		}

		if(isdigit(*p)){ //数字ならtrue
			cur = new_token(TK_NUM,cur,p);
			cur->val = strtol(p,&p,10);
			continue;
		}

		error_at(p,"数ではありません");
	}
	
	new_token(TK_EOF,cur,p);
	return head.next;
}

Node *new_node(NodeKind kind,Node *lhs,Node *rhs){
	Node *node = calloc(1,sizeof(Node));
	node -> kind = kind;
	node -> lhs = lhs;
	node -> rhs = rhs;
	return node;
}

Node *new_node_num(int val){
	Node *node = calloc(1,sizeof(Node));
	node -> kind = ND_NUM;
	node -> val = val;
	return node;
}

Node *expr();
Node *mul();
Node *primary();

// expr = mul ("+" mul | "-" mul)*
Node *expr(){
	Node *node = mul();

	for(;;){
		if(consume('+')){
			node = new_node(ND_ADD,node,mul());
		}else if(consume('-')){
			node = new_node(ND_SUB,node,mul());
		}else
			return node;
	}
}

// mul = primary("*" primary | "/" primary)*
Node *mul(){
	Node *node = primary();

	for(;;){
		if(consume('*')){
			node = new_node(ND_MUL,node,primary());
		}else if(consume('/')){
			node = new_node(ND_DIV,node,primary());
		}else
			return node;
	}
}

// primary = "(" expr ")" | num
Node *primary(){
	//次のトークンが"("なら,"("expr")"のはず
	if(consume('(')){
		Node *node = expr();
		expect(')');
		return node;
	}

	return new_node_num(expect_number());
}

void gen(Node *node) {
	if (node->kind == ND_NUM) {
		printf("  push %d\n", node->val);
    		return;
  	}

 	gen(node->lhs);
  	gen(node->rhs);

  	printf("  pop rdi\n");
  	printf("  pop rax\n");

  	switch (node->kind) {
  	case ND_ADD:
    		printf("  add rax, rdi\n");
    		break;
  	case ND_SUB:
    		printf("  sub rax, rdi\n");
    		break;
  	case ND_MUL:
    		printf("  imul rax, rdi\n");
    		break;
  	case ND_DIV:
    		printf("  cqo\n");
    		printf("  idiv rdi\n");
		break;
	}

	printf("  push rax\n");
}

int main(int argc,char **argv){
	if(argc != 2){
		fprintf(stderr,"引数の個数が正しくありません\n");
		return 1;
	}
	
	//トークナイズする
	user_input = argv[1];
	token = tokenize();
	Node *node = expr();

	//アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	//抽象構文木を下りながらコード生成
	gen(node);

	/*

	//式の最初は数でなければならないので、それをチェックして
	//最初のmov命令を出力
	printf("	mov rax, %d\n",expect_number());

	//'+<数>'あるいは'-<数>'というトークンの並びを消費しつつ
	//アセンブリを出力
	while(!at_eof()){
		if(consume('+')){  //+であればconsum内で1つトークンを読み進めてtrueを返す
			printf("	add rax, %d\n", expect_number());
			continue;
		}

		expect('-');
		printf("	sub rax, %d\n", expect_number());
	}
	*/

	printf("	pop rax\n");
	printf("	ret\n");

	return 0;
}