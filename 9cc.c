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
	int val;
	char *str;
};

//現在着目しているトークン
Token *token;

//エラーを報告するための関数
//printfと同じ引数を取る
void error(char *fmt,...){
	va_list ap;
	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
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
		error("'%c'ではありません",op);
	token = token->next;
}

//次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
//それ以外の場合にはエラーを報告する。
int expect_number(){
	if(token->kind != TK_NUM)
		error("数ではありません");
	//printf("kind=%d ",token->kind);
	//printf("val=%d",token->val);
	printf(" next:%p",token->next);
	int val = token->val;
	token = token->next; //次の入力トークンをtokenに入れる
	return val;
}

bool at_eof(){
	printf("token_kind:%d",token->kind);
	return token->kind == TK_EOF;
}

//新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind,Token *cur,char *str){
	//Tokenのサイズ文メモリを確保(0で初期化)
	Token *tok = calloc(1,sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok; //nextにtokのアドレスを追加
	return tok;
}

//入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p){
	Token head;
	head.next = NULL;
	Token *cur = &head; //headの値をcurで参照

	while (*p){
		//空白文字をスキップ
		if(isspace(*p)){ //空白ならtrue
			p++;
			continue;
		}

		if(*p == '+' || *p == '-'){
			cur = new_token(TK_RESERVED,cur,p++);
			continue;
		}

		if(isdigit(*p)){ //数字ならtrue
			cur = new_token(TK_NUM,cur,p);
			cur->val = strtol(p,&p,10);
			continue;
		}

		error("トークンナイズできません");
	}
	
	new_token(TK_EOF,cur,p);
	return head.next;
}

int main(int argc,char **argv){
	if(argc != 2){
		fprintf(stderr,"引数の個数が正しくありません\n");
		return 1;
	}

	char *p = argv[1];

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	//トークナイズする
	token = tokenize(argv[1]);

	//アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf("global main\n");
	printf("main:\n");

	//式の最初は数でなければならないので、それをチェックして
	//最初のmov命令を出力
	printf("	mov rax, %d\n",expect_number());

	//'+<数>'あるいは'-<数>'というトークンの並びを消費しつつ
	//アセンブリを出力
	while(!at_eof()){
		if(consume('+')){
			p++;
			printf("	add rax, %d\n", expect_number());
			continue;
		}

		expect('-');
		printf("	sub rax, %d\n", expect_number());
	}

	printf("	ret\n");

	return 0;
}