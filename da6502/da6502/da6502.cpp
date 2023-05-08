// da6502.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "cda6502.h"
#include "casm6502.h"


bool get_address(const char *p,unsigned long &r,unsigned char &ct);


int main(int argc, char* argv[])
{
	char filename[256];
	strcpy(filename,"noname.bin");	//初期設定ファイル名

	unsigned char filedat[262144];
	unsigned long loadsize = 0;

	if (argc > 1)
	{
		strcpy(filename,argv[1]);

		FILE *fr = fopen(filename,"rb");
		if (!fr)
		{
			return 0;
		}
		loadsize = fread(filedat,1,262144,fr);
		fclose(fr);
	}



	casm6502 a;
	a.outbuf = filedat;
	cda6502 x;
	x.p = filedat;

	unsigned long dump_ad,edit_ad,unassemble_ad,assemble_ad;
	dump_ad = edit_ad = unassemble_ad = assemble_ad = 0;

	bool loop_fg = true;
	int lp,lp2;

	unsigned char d,c;
	unsigned char dat[32768];
	char buf[32768];
	unsigned long rd = 0,wr;
	char *p;
	int f;
	bool b_history = false;


	do
	{
		printf(">");
		
		rd = 0;
		do
		{
			scanf("%c",&c);
			
			if (c == '\n')
			{
				buf[rd++] = '\0';
				break;
			}
			else
			{
				buf[rd++] = c;
			}

		}while(1);
		if (buf[0] == 0x0a || buf[0] == 0x0d || buf[0] == 0x00)
		{
			continue;
		}

		p = buf+1;
		while(*p)
		{
			if (*p == ' ' || *p == '\t')
			{
				p++;
				break;
			}
			if (*p == '\n')
			{
				p = NULL;
				break;
			}
			p++;
		}

		//そのうち末尾側から処理するようにしないといけない
		//もしくは先頭にダミーいれて偶数にするようにするとか

		d = 0;
		wr = 1;
		f = 0;
		while(p && *p)
		{
			switch(*p)
			{
			case 0x0a:
			case 0x0d:
				if (f & 1)
				{
					f = 0;
					dat[wr++] = d;
				}
				*p = '\0';
				p = NULL;
				break;
			case ' ':
			case '\t':
				if (f & 1)
				{
					f = 0;
					dat[wr++] = d;
					if (wr > 32767)	p = NULL;
				}
				break;
			case '0':	case '1':	case '2':	case '3':	case '4':
			case '5':	case '6':	case '7':	case '8':	case '9':
				if (f & 1)
				{
					d = (d << 4) | *p-'0';
					f = 0;
					dat[wr++] = d;
				}
				else
				{
					d = *p - '0';
					f = 1;
				}
				if (wr > 32767)	p = NULL;
				break;
			case 'a':	case 'b':	case 'c':	case 'd':	case 'e':	case 'f':
				if (f & 1)
				{
					d = (d << 4) | *p-'a' + 10;
					f = 0;
					dat[wr++] = d;
				}
				else
				{
					d = *p - 'a' + 10;
					f = 1;
				}
				if (wr > 32767)	p = NULL;
				break;
			case 'A':	case 'B':	case 'C':	case 'D':	case 'E':	case 'F':
				if (f & 1)
				{
					d = (d << 4) | *p-'A' + 10;
					f = 0;
					dat[wr++] = d;
				}
				else
				{
					d = *p - 'A' + 10;
					f = 1;
				}
				if (wr > 32767)	p = NULL;
				break;
			}
			if (p)	p++;
		}
		if (f & 1)
		{
			dat[wr++] = d;
		}


		rd = 0;
		switch(buf[0])
		{
		default:
			printf("invalid command [%c]\n",buf[0]);
			rd = 0;
			break;

		case 'a':	//assemble
			if (buf[1] != 'c')
			{
				a.clear();
			}
			else
			{
				if (buf[1] != '\0')	//アドレス省略時は前回までの累積アドレスを使用する
				{
					unsigned char ct;
					get_address(buf+2,assemble_ad,ct);
				}

				a.ad = assemble_ad;
				a.o_ad = a.ad;

				do
				{
					//アセンブル入力モード
					printf("%05x\t",a.ad);

					fgets(buf,255,stdin);

					//何もなければ終了
					if (buf[0] == '\0' || buf[0] == '\n')
					{
						break;
					}

					a.assemble(buf);

					if (a.o_ad < a.ad)
					{
						int lx = 0;

						printf("%05x\t",a.o_ad);
						for (;a.o_ad < a.ad;a.o_ad++,lx++)
						{
							printf("%02x ",filedat[a.o_ad]);
						}
						for (;lx < 4;lx++)
						{
							printf("   ");
						}
						printf("%s",buf);
					}

				}while(1);

				assemble_ad = a.ad;
			}
			break;



		case 'q':	//quit
			printf("exit!\n");
			loop_fg = false;
			break;

		case 'n':	//filename変更
			if (buf[1] == '\0' || (buf[1] != ' ' && buf[1] != '\t'))
			{
				printf("illegal command request.\n");
				break;
			}
			{
				char *p = buf+2;
				do
				{
					if (*p == ' ' || *p == '\t')
					{
						p++;
						continue;
					}
					break;
				}while(1);

				//ファイル名を保存
				strcpy(filename,p);
				printf("set filename [%s]\n",filename);
			}
			break;

		case 'l':	//load
			if (buf[1] != '\0')
			{
				if (buf[1] != ' ' && buf[1] != '\t')
				{
					printf("illegal command request.\n");
					break;
				}
				char *p = buf+2;
				do
				{
					if (*p == ' ' || *p == '\t')
					{
						p++;
						continue;
					}
					break;
				}while(1);

				//ファイル名を保存
				strcpy(filename,p);
			}
			{
				//ロード
				FILE *fr = fopen(filename,"rb");
				if (!fr)
				{
					printf("file open error.\n");
					break;
				}
				loadsize = fread(filedat,1,262144,fr);
				fclose(fr);
				printf("load file comp.[%s][%d(%08x)]\n",filename,loadsize,loadsize);
			}
			break;
		case 'w':	//save & wait
			if (buf[1] != '\0')
			{
				if (buf[1] != ' ' && buf[1] != '\t')
				{
					printf("illegal command request.\n");
					break;
				}
				char *p = buf+2;
				do
				{
					if (*p == ' ' || *p == '\t')
					{
						p++;
						continue;
					}
					break;
				}while(1);

				//ファイル名を保存
				strcpy(filename,p);
			}
			//オリジナルはcxレジスタのサイズだけど、ロードしたサイズにする
			if (loadsize > 0)
			{
				//セーブ
				FILE *fw = fopen(filename,"wb");
				if (!fw)
				{
					printf("file open error.\n");
					break;
				}
				if (loadsize > 1048576)
					fwrite(filedat,1,1048576,fw);
				else
					fwrite(filedat,1,loadsize,fw);
				fclose(fw);
				printf("save file comp.[%s]\n",filename);
			}
			break;


		case 'h':	//history
			b_history ^= 1;
			if (b_history)
				printf("history on\n");
			else
				printf("history off\n");
			break;



		case 'u':	//un assemble表示
			{
				unsigned long e_unassemble_ad = 0;

				if (buf[1] != '\0')	//アドレス省略時は前回までの累積アドレスを使用する
				{
					unsigned char ct;
					get_address(buf+2,unassemble_ad,ct);

					x.p = filedat + unassemble_ad;
					x.ad = unassemble_ad;

					char *p = buf+2+ct+1;
					if (*p != '\0')
					{
						get_address(p,e_unassemble_ad,ct);
					}
				}

				if (e_unassemble_ad)
				{
					while(x.ad < e_unassemble_ad)
					{
						x.da();
					}
				}
				else
				{
					for (lp = 0;lp < 8;lp++)
					{
						x.da();
					}
				}
			}
			break;

		case 'd':	//dump & display io
			{
				//dump
				char cbuf[32];
				cbuf[16] = '\0';

				unsigned long e_dump_ad = 0;
				if (buf[1] != '\0')	//アドレス省略時は前回までの累積アドレスを使用する
				{
					unsigned char ct;
					get_address(buf+2,dump_ad,ct);

					char *p = buf+2+ct+1;
					if (*p != '\0')
					{
						get_address(p,e_dump_ad,ct);
					}

				}

				if (e_dump_ad)
				{
					while(dump_ad < e_dump_ad)
					{
						printf("%05x ",dump_ad);
						for (lp2 = 0;lp2 < 16;lp2++)
						{
							if (filedat[dump_ad+lp2] < 0x20)
								cbuf[lp2] = '.';
							else
								cbuf[lp2] = filedat[dump_ad+lp2];
							if (lp2 == 7)
								printf("%02x-",filedat[dump_ad+lp2]);
							else
								printf("%02x ",filedat[dump_ad+lp2]);
						}
						printf("%s\n",cbuf);
						dump_ad += 16;
					}
				}
				else
				{
					for (lp = 0;lp < 8;lp++)
					{
						printf("%05x ",dump_ad);
						for (lp2 = 0;lp2 < 16;lp2++)
						{
							if (filedat[dump_ad+lp2] < 0x20)
								cbuf[lp2] = '.';
							else
								cbuf[lp2] = filedat[dump_ad+lp2];
							if (lp2 == 7)
								printf("%02x-",filedat[dump_ad+lp2]);
							else
								printf("%02x ",filedat[dump_ad+lp2]);
						}
						printf("%s\n",cbuf);
						dump_ad += 16;
					}
				}
			}
			break;

		case 'e':	//enter		アドレス指定が絶対アドレスだと、今見えているアドレスじゃない
			{
				char *xpt[64];
				lp = 0;
				if (buf[1] == ' ')
				{
					char *p = strtok(buf+2," \t");
					while(p)
					{
						xpt[lp++] = p;
						if (lp > 63)	break;
						p = strtok(NULL," \t");
					}
					if (lp > 0)
					{
						unsigned char ct;
						get_address(xpt[0],edit_ad,ct);
					}
				}
				//データ指定
				if (lp > 1)
				{
					for (lp2 = 1;lp2 < lp;lp2++)
					{
						filedat[edit_ad++] = strtol(xpt[lp2],NULL,16);
					}
					break;
				}
				
				//データ入力モード
				printf("%05x ",edit_ad);
				printf("%02x-",filedat[edit_ad]);

				unsigned char d = 0,fg = 0;
				unsigned int c;
				do
				{
					c = getch();
					putc(c,stdout);
			
					if (c == '\n' || c == '\r')
					{
						printf("\n");
						break;
					}

					if (c == 0x08)	//bs
					{
						putc(' ',stdout);
						putc(0x08,stdout);
						if (fg)	fg--;
						continue;
					}

					//スキップ
					if (c == ' ')
					{
						fg = 3;
					}
					else
					{
						d <<= 4;
						if (c >= '0' && c <= '9')
						{
							d += c-'0';
						}
						else if (c >= 'A' && c <= 'F')
						{
							d += c-'A'+10;
						}
						else if (c >= 'a' && c <= 'f')
						{
							d += c-'a'+10;
						}
						else
						{
							printf("?\n%05x %02x-",edit_ad,filedat[edit_ad]);
							fg = 0;
							continue;
						}
						fg++;
					}

					if (fg > 1)
					{
						if (fg < 3)	filedat[edit_ad] = d;

						if ((edit_ad & 7) == 7)
						{
							printf("\n%05x",edit_ad+1);
						}

						edit_ad++;
						printf(" %02x-",filedat[edit_ad]);

						d = 0;
						fg = 0;
					}

				}while(1);

			}
			break;



			//move command			src dst size
		case 'm':
			{
				unsigned long sad,dad,msz;
				unsigned char ct;
				if (!get_address(buf+2,sad,ct))
				{
					printf("invalid command\n");
					break;
				}
				char *p = buf+2+ct+1;
				if (*p == '\0')
				{
					printf("invalid command\n");
					break;
				}
				if (!get_address(p,dad,ct))
				{
					printf("invalid command\n");
					break;
				}
				p = p+ct+1;
				if (*p == '\0')
				{
					printf("invalid command\n");
					break;
				}
				if (!get_address(p,msz,ct))
				{
					printf("invalid command\n");
					break;
				}

				while(msz--)
				{
					filedat[dad++] = filedat[sad++];
				}

			}
			break;




			//fill & debug fm register set
		case 'f':
			switch(buf[1])
			{
			case '\0':	//required 2char
				printf("plz ext char!\n");
				break;
			case ' ':	//通常のfillコマンド
			case '\t':
				{
					unsigned long sad,ead,fillchar;
					unsigned char ct;
					if (!get_address(buf+2,sad,ct))
					{
						printf("invalid command\n");
						break;
					}
					char *p = buf+2+ct+1;
					if (*p == '\0')
					{
						printf("invalid command\n");
						break;
					}
					if (!get_address(p,ead,ct))
					{
						printf("invalid command\n");
						break;
					}
					p = p+ct+1;
					if (*p == '\0')
					{
						printf("invalid command\n");
						break;
					}
					if (!get_address(p,fillchar,ct))
					{
						printf("invalid command\n");
						break;
					}

					if (sad > ead)
					{
						printf("address error.\n");
					}

					//
					while(sad <= ead)
					{
						filedat[sad++] = fillchar;
					}

				}
				break;
			case 's':	//file size 変更
				{
					unsigned long fsz;
					unsigned char ct;
					if (!get_address(buf+2,fsz,ct))
					{
						printf("invalid file size\n");
						break;
					}
					loadsize = fsz;
					printf("set filesize -> [%d]\n",loadsize);
				}
				break;
			}
			break;

		}


	}while(loop_fg);


	return 0;
}




//アドレス取得サブ
bool get_address(const char *p,unsigned long &r,unsigned char &ct)
{
	unsigned char fg = 1;
	unsigned char b = 0;
	ct = 0;
	r = 0;

	bool res = true;

	do
	{
		switch(*p)
		{
		default:	//わからない文字ならエラー
			res = false;
		case ' ':	//知ってる文字でセパレーターなら終了
		case '\t':
			if (ct == 0)	//先頭ならトリム
			{
				p++;
				continue;
			}

		case '\n':
		case '\r':
		case '\0':
			fg = 0;
			break;

		case '0':	case '1':	case '2':	case '3':	case '4':
		case '5':	case '6':	case '7':	case '8':	case '9':
			r = r*16 + *p-'0';
			ct++;
			break;
		case 'a':	case 'b':	case 'c':	case 'd':	case 'e':	case 'f':
			b = *p;
			r = r*16 + *p-'a'+10;
			ct++;
			break;
		case 'A':	case 'B':	case 'C':	case 'D':	case 'E':	case 'F':
			b = *p;
			r = r*16 + *p-'A'+10;
			ct++;
			break;
		}

		p++;
	}while(fg);

	return res;
}

