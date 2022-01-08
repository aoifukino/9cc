#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv){
	if(argc != 2){
		fprintf(stderr,"引数の個数が正しくありません\n");
		return 1;
	}

	char *p = argv[1];

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");
	//strtol:文字列を整数に変換 数値を読み込んだ後、第2引数のポインタを
	//アップデートして、次の文字を指すように値を更新する。
	printf("	mov rax, %ld\n",strtol(p,&p,10));

	while(*p){
		if(*p == '+'){
			p++;//+ → 20
			printf("	add rax, %ld\n", strtol(p, &p, 10));
			continue;
		}

		if(*p == '-'){
			p++;//- →　4
			printf("	sub rax, %ld\n", strtol(p, &p, 10));
			continue;
		}

		fprintf(stderr,"予期しない文字です: '%c'\n",*p);
		return 1;
	}

	printf("	ret\n");
	return 0;
}