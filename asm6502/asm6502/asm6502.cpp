// asm6502.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//


//
//	2019/6/16	v1.0
//	2019/6/22	v1.1	いろいろ修正
//		 6/24	v1.2	オフセット加算などを追加
//		 6/25	v1.3	相対ジャンプ用$を追加。1行サブルーチン化
//		 6/27	v1.4	マクロ追加,align追加
//		7/26	v1.5	オプションとか追加、max 512KB、アドレスサイズエラーとか追加
//
//	残り
//		o		l02+1等の対応
//		o		1行サブルーチン化
//		o		相対ジャンプ命令で $ の使用
//		o		db 0,1,'abcdef',0	とかの混在
//		o		マクロ
//
//				includeでソースマージ
//
//


#include "stdafx.h"
#include "casm6502.h"


struct OPT
{
	OPT()
	{
		offset = 0;
		size = 0;
		outputfile = "dump.bin";
	};

	std::string inputfile;
	std::string outputfile;

	unsigned long offset;
	unsigned long size;

};




int main(int argc, char* argv[])
{

	OPT opt;
	{
		bool b_err = false;

		int lp,seq = 0;
		for (lp = 1;lp < argc;lp++)
		{
			switch(seq)
			{
			default:
				b_err = true;
				break;

			case 0:	//option値検査
				if (strcmp(argv[lp],"-offset") == 0)
				{
					seq = 1;
					continue;
				}
				if (strcmp(argv[lp],"-size") == 0)
				{
					seq = 2;
					continue;
				}
				if (strcmp(argv[lp],"-output") == 0)
				{
					seq = 3;
					continue;
				}

				//何もないなら、入力ファイル名　ただし1回目だけ
				if (opt.inputfile.empty())
				{
					opt.inputfile = argv[lp];
					continue;
				}

				//エラー
				printf("invalid option : [No.%d:%s]\n",lp,argv[1]);
				b_err = true;
				break;

				//値の保存
			case 1:
				opt.offset = strtoul(argv[lp],NULL,10);
				break;
			case 2:
				opt.size = strtoul(argv[lp],NULL,10);
				break;
			case 3:
				opt.outputfile = argv[lp];
				break;
			}
		}

		if (b_err || opt.inputfile.empty())
		{
			printf(
				"x86風 6502 assembler v 1.5\n"
				"Copyright ultrya, rev 19/07/26\n\n"
				"usage: asm6502 assemblefile [option]\n"
				"\n"
				"option: -offset <offset value>  ...  アセンブルしたバイナリの書き出しオフセット位置を指定します\n"
				"        -size <size value>      ...  アセンブルしたバイナリのサイズを明示的に指定します\n"
				"        -output <filename>      ...  アセンブルしたバイナリの書き出しファイル名を指定します\n"
				);

			return 0;
		}

	}



	FILE *fr = fopen(opt.inputfile.c_str(),"r");
	if (!fr)
	{
		return 0;
	}


	casm6502 a;

	char buf[32768],buf2[32768];

	a.ad = 0;
	a.o_ad = 0;
	a.ln = 0;
	a.b_err = false;
	

	unsigned char *outbuf = new unsigned char[524288];
	memset(outbuf,0,524288);
	a.outbuf = outbuf;
	char *p;

	while(fgets(buf,32767,fr))
	{
		//表示用にコピー
		strcpy(buf2,buf);
		if ((p = strrchr(buf2,'\n')))	*p = '\0';

		a.ln++;

		//アセンブル
		if (a.assemble(buf))
		{
			//画面表示確認用
			if (a.o_ad < a.ad)
			{
				int lx = 0;

				printf("%04x\t",a.o_ad);
				for (;a.o_ad < a.ad;a.o_ad++,lx++)
				{
					printf("%02x ",outbuf[a.o_ad]);
				}
				for (;lx < 4;lx++)
				{
					printf("   ");
				}
				printf("%s\n",buf2);
			}
		}

		//エラーが発生していたら中断する
		if (a.b_err)	break;
	}

	fclose(fr);

	printf("\n");


	if (!a.b_err)
	{
		printf("assemble warnings -----------------------------------------\n");
		a.warnings();

		do
		{
			//書き出しサイズ計算
			unsigned long sz = !opt.size ? a.max_ad : opt.size;

			if (opt.offset)
			{
				//指定されたオフセットが、最大アドレスよりも大きい
				if (a.max_ad < opt.offset)
				{
					printf("write offset over.\n");
					break;
				}

				sz -= opt.offset;

				if (opt.offset + sz >= 524288)
				{
					printf("writing size overflow. [%d]\n",a.max_ad+opt.size);
					break;
				}
			}

			//ファイルを書き出す
			FILE *fw = fopen(opt.outputfile.c_str(),"wb");
			if (fw)
			{
				fwrite(outbuf + opt.offset,
					1,
					sz,
					fw);
				fclose(fw);
			}

		}while(0);

	}

	delete[] outbuf;

	return 0;
}


