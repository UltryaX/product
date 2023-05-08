#include "StdAfx.h"
#include "casm6502.h"


casm6502::~casm6502(void)
{
}

//文字列置換サブ
std::string &casm6502::replace(std::string &str,std::string src,std::string dst)
{
	std::string::size_type pos = 0;
	while( (pos = str.find(src, pos)) != std::string::npos )
	{
		str.replace(pos, src.size(), dst);
		pos += dst.size();
	}

	return str;
}


//トリム処理
void casm6502::trimleft(char **p)
{
	while(**p)
	{
		if (**p == ' ' || **p == '\t')
		{
			(*p)++;
		}
		else
			break;
	}
}

// , のまとめ処理
bool casm6502::checkcanma(char **p)
{
	(*p)++;
	trimleft(p);
	if (**p != ',')
	{
		printf("syntax error. line:%d [%s]\n",ln,*p);
		b_err = true;
		return false;
	}

	(*p)++;
	trimleft(p);

	return true;
}



//先頭文字がリテラルか
bool casm6502::checkreteral(char *p)
{
	bool b = false;
	switch(*p)
	{
		case '0':	case '1':	case '2':	case '3':	case '4':	case '5':	case '6':	case '7':	case '8':	case '9':
			b = true;
			break;
	}
	return b;
}



void casm6502::get_label(char **p,std::string &s,LABEL &l)
{
	//ラベル名を抽出
	const char *ss = *p;
	s.clear();
	l.dr = 0;
	l.d = 0;

	int fg = 1;
	do
	{
		switch(**p)
		{
		default:
			fg = 0;
			break;

			//ラベル名はリテラルの位置。現在のアドレッシングだと、リテラル+x等はない。[[xx]+y]はあるが、]である
			//つまりラベル名に+か-が来たら、修飾子だととらえてもよいだろう
		case '+':
		case '-':
			{
				char *x = (*p)+1;
				trimleft(&x);
				if (!checkreteral(x))
				{
					(*p)++;
					break;
				}

				l.d = get_reteral(&x);	//ここでまたラベルとかやるとおかしくなるが、無視する
				if (**p == '+')
					l.dr = 1;	//+方向
				else
					l.dr = 2;	//-方向

				**p = '\0';

				fg = 0;
				*p = x;
			}
			break;


		case '_':
		case '0':	case '1':	case '2':	case '3':	case '4':	
		case '5':	case '6':	case '7':	case '8':	case '9':
		case 'a':	case 'b':	case 'c':	case 'd':	case 'e':	case 'f':	case 'g':	case 'h':
		case 'i':	case 'j':	case 'k':	case 'l':	case 'm':	case 'n':	case 'o':	case 'p':
		case 'q':	case 'r':	case 's':	case 't':	case 'u':	case 'v':	case 'w':	case 'x':	case 'y':	case 'z':
		case 'A':	case 'B':	case 'C':	case 'D':	case 'E':	case 'F':	case 'G':	case 'H':
		case 'I':	case 'J':	case 'K':	case 'L':	case 'M':	case 'N':	case 'O':	case 'P':
		case 'Q':	case 'R':	case 'S':	case 'T':	case 'U':	case 'V':	case 'W':	case 'X':	case 'Y':	case 'Z':
			(*p)++;
			break;
		}
	}while(fg);
				
	char e = **p;
	**p = '\0';
	s = ss;
	**p = e;

	return;
}





//リテラル値の取得
unsigned long casm6502::get_reteral(char **p)
{
	int fg = 1;
	unsigned long ul = 0;
	char *x = NULL;

	if (!checkreteral(*p))
	{
		//ラベルの取得
		std::string s;
		LABEL l;
		get_label(p,s,l);

		//ラベルマップから探す
		if ( (il = label_map.find(s)) != label_map.end() )
		{
			ul = il->second;
			if (l.dr == 1)
				ul += l.d;
			else
				ul -= l.d;
		}
		else
		{
			if ( (ila = delay_absolute_address_map.find(s)) == delay_absolute_address_map.end() )
			{
				std::deque<LABEL> d;
				l.ad = ad+1;
				d.push_back(l);
				delay_absolute_address_map.insert( std::pair<std::string,std::deque<LABEL>>(s,d) );
			}
			else
			{
				l.ad = ad+1;
				ila->second.push_back(l);
			}

			//absolute用アドレッシング
			ul = 0xffff;
		}

	}
	else
	{

		do
		{
			switch(**p)
			{
			case ' ':
			case  '\t':
				(*p)++;
				break;

			case 'h':
				**p = '\0';
				if (x)
				{
					ul = strtoul(x,NULL,16);
				}
				(*p)++;
				fg = 0;
				break;

			case '0':	case '1':	case '2':	case '3':	case '4':
			case '5':	case '6':	case '7':	case '8':	case '9':
			case 'A':	case 'B':	case 'C':	case 'D':	case 'E':	case 'F':
			case 'a':	case 'b':	case 'c':	case 'd':	case 'e':	case 'f':
				if (!x)
				{
					x = *p;
				}
				(*p)++;
				break;

			default:
				if (x)
				{
					ul = strtol(x,p,10);
				}
				fg = 0;
				break;
			}

		}while(fg);

	}

	return ul;
}



//	1:イミディエイト		00h
//	2:ゼロページ			[00h]
//	3:ゼロページX			[x+00h]
//	4:アブソリュート		[1234h]
//	5:アブソリュートX		[x+1234h]
//	6:アブソリュートY		[y+1234h]
//	7:インダイレクトX		[[x+12h]]
//	8:インダイレクトY		[[12h]+y]
//	9:ゼロページY			[y+00h]
//
int casm6502::addressing(char **p,unsigned short *xx)
{
	int n = 0;	//none

	trimleft(p);

	if (**p == '[')
	{	//addressing
		(*p)++;
		trimleft(p);

		switch(**p)
		{
		default:	//zeropage & absolute
			*xx = (unsigned short)get_reteral(p);
			if (*xx < 0x100)
				n = 2;		//zeropage
			else
				n = 4;		//absolute
			trimleft(p);
			if (**p != ']')
			{
				printf("syntax error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			}
			break;

		case '[':	//indirect
			(*p)++;
			trimleft(p);
			if (**p == 'x')		//	(indirect,x)	[[x+12h]]
			{
				(*p)++;
				trimleft(p);
				if (**p != '+')
				{
					printf("syntax error. line:%d [%s]\n",ln,p);
					b_err = true;
					break;
				}
				(*p)++;
				trimleft(p);
				*xx = (unsigned short)get_reteral(p);
				trimleft(p);
				if (**p != ']')
				{
					printf("syntax error. line:%d [%s]\n",ln,*p);
					b_err = true;
					break;
				}
				trimleft(p);
				if (**p != ']')
				{
					printf("syntax error. line:%d [%s]\n",ln,*p);
					b_err = true;
					break;
				}
				n = 7;
			}
			else
			{
				//[[%02x]+y]
				*xx = (unsigned short)get_reteral(p);
				trimleft(p);
				if (**p != ']')
				{
					printf("syntax error. line:%d [%s]\n",ln,*p);
					b_err = true;
					break;
				}
				(*p)++;
				trimleft(p);
				if (**p != '+')
				{
					printf("syntax error. line:%d [%s]\n",ln,*p);
					b_err = true;
					break;
				}
				(*p)++;
				trimleft(p);
				if (**p != 'y')
				{
					printf("syntax error. line:%d [%s]\n",ln,*p);
					b_err = true;
					break;
				}
				(*p)++;
				trimleft(p);
				if (**p != ']')
				{
					printf("syntax error. line:%d [%s]\n",ln,*p);
					b_err = true;
					break;
				}
				n = 8;	//indirect y
			}
			break;

		case 'x':	//zeropage x / absolute x
			(*p)++;
			trimleft(p);
			if (**p != '+')
			{
				printf("syntax error. line:%d [%s]\n",ln,*p);
				b_err = true;
				break;
			}
			(*p)++;
			trimleft(p);
			*xx = (unsigned short)get_reteral(p);
			if (*xx < 0x100)
			{
				n = 3;	//zeropage x
			}
			else
			{
				n = 5;	//absolute x
			}
			trimleft(p);
			if (**p != ']')
			{
				printf("syntax error. line:%d [%s]\n",ln,*p);
				b_err = true;
				break;
			}
			break;
		case 'y':	//zeropage y / absolute y
			p++;
			trimleft(p);
			if (**p != '+')
			{
				printf("syntax error. line:%d [%s]\n",ln,*p);
				b_err = true;
				break;
			}
			(*p)++;
			trimleft(p);
			*xx = (unsigned short)get_reteral(p);
			if (*xx < 0x100)
			{
				n = 9;
			}
			else
			{
				n = 6;
			}
			break;
		}
	}
	else
	{
		switch(**p)
		{
		default:
			//immidiate
			*xx = (unsigned short)get_reteral(p);
			n = 1;
			break;
		case 'a':	n = 10;	break;
		case 'x':	n = 11;	break;
		case 'y':	n = 12;	break;
		case 's':	n = 13;	break;

		case 'o':	//offset指定
			{
				unsigned int offsetmode = 0;
				if (strncmp(*p,"offset_h",8) == 0)
				{
					offsetmode = 1;
					*p += 8;
				}
				else if (strncmp(*p,"offset+1",8) == 0)
				{
					offsetmode = 1;
					*p += 8;
				}
				else if (strncmp(*p,"offset",6) == 0)
				{
					offsetmode = 2;
					*p += 6;
				}


				if (offsetmode)
				{
					//ラベル検索
					trimleft(p);
					if (**p)
					{

						//ラベル名を抽出
						const char *s = *p;
						int fg = 1;
						do
						{
							switch(**p)
							{
							default:
								fg = 0;
								break;
							case '_':	case '-':
							case '0':	case '1':	case '2':	case '3':	case '4':	
							case '5':	case '6':	case '7':	case '8':	case '9':
							case 'a':	case 'b':	case 'c':	case 'd':	case 'e':	case 'f':	case 'g':	case 'h':
							case 'i':	case 'j':	case 'k':	case 'l':	case 'm':	case 'n':	case 'o':	case 'p':
							case 'q':	case 'r':	case 's':	case 't':	case 'u':	case 'v':	case 'w':	case 'x':	case 'y':	case 'z':
							case 'A':	case 'B':	case 'C':	case 'D':	case 'E':	case 'F':	case 'G':	case 'H':
							case 'I':	case 'J':	case 'K':	case 'L':	case 'M':	case 'N':	case 'O':	case 'P':
							case 'Q':	case 'R':	case 'S':	case 'T':	case 'U':	case 'V':	case 'W':	case 'X':	case 'Y':	case 'Z':
								(*p)++;
								break;
							}
						}while(fg);
				
						char e = **p;
						**p = '\0';

						unsigned long ul;

						//ラベルマップから探す
						if ( (il = label_map.find(s)) != label_map.end() )
						{
							ul = il->second;
						}
						else
						{
							if ( (io = delay_offset_map.find(s)) == delay_offset_map.end() )
							{
								std::deque< std::pair<unsigned long,unsigned char>> d;
								d.push_back( std::pair<unsigned long,unsigned char>(ad+1,offsetmode-1) );
								delay_offset_map.insert( std::pair<std::string,std::deque<std::pair<unsigned long,unsigned char>>>(s,d) );
							}
							else
							{
								io->second.push_back( std::pair<unsigned long,unsigned char>(ad+1,offsetmode-1) );
							}

							//absolute用アドレッシング
							ul = 0xffff;
						}

						**p = e;

						//offsetと+1で返す値を変更する
						if (offsetmode == 1)
							*xx = (unsigned char)(ul >> 8);
						else
							*xx = (unsigned char)(ul & 255);

						//offset値のimmidiateを返す
						n = 1;
					}
					break;
				}

			}

			printf("syntax error. line:%d [%s]\n",ln,*p);
			break;
		}
	}

	return n;
}


long casm6502::get_relative(char **p)
{
	long relative = 0;
	unsigned short xx;
	if (**p == '$')
	{
		unsigned long target_ad = ad;
		(*p)++;
		trimleft(p);
		if (**p == '+')
		{
			(*p)++;
			trimleft(p);
			xx = (unsigned short)get_reteral(p);
			target_ad += xx;
		}
		else if (**p == '-')
		{
			(*p)++;
			trimleft(p);
			xx = (unsigned short)get_reteral(p);
			target_ad -= xx;
		}
		relative = target_ad - ad - 2;
	}

	return relative;
}






bool casm6502::assemble(char *buf)
{
	char *p,*pp;

	int n;
	unsigned short xx;


	//ln++;
	if ((p = strrchr(buf,'\n')))	*p = '\0';
	p = buf;
	trimleft(&p);
	if (!*p)	return false;

	//行にコメント ; が入っていたらそこから後ろを解析しない
	if ( (pp = strchr(p,';')) )	*pp = '\0';

	//後ろのタブやスペースを除去する
	int len = strlen(buf);
	char *x = buf + len-1;
	while(len)
	{
		switch(*x)
		{
		default:
			len = 0;
			break;
		case ' ':
		case '\t':
			*x = '\0';
			len--;
			break;
		}
	}

	//マクロの記録
	if (b_macromode)
	{
		if (strstr(buf,"endm"))
		{
			b_macromode = false;
			macros.insert( std::pair<std::string,std::deque<std::string>>(macro_name,tmp_macro) );
			return false;
		}

		tmp_macro.push_back(p);
		return false;
	}



	//	"ラベル	db	xxxx"　とか、コロンなしのラベル記述を見つけたら処理する仕掛け
	pp = p;
	do
	{
		if (*pp == ' ' || *pp == '\t' || *pp == ':' || *pp == '\0')
		{
			break;
		}
		pp++;
	}while(*pp);
	if (*pp && *pp != ':')
	{
		char *ppp = pp;
		trimleft(&pp);
		if (*pp)
		{
			if (strncmp(pp,"db",2) == 0 || strncmp(pp,"dw",2) == 0 || strncmp(pp,"ds",2) == 0)
			{
				*ppp = ':';
			}
			else if (strncmp(pp,"macro",5) == 0)
			{
				b_macromode = true;
				tmp_macro.clear();
				*ppp = '\0';
				macro_name = p;
				return false;
			}
		}
	}


	//行に :　が入っていたらラベル登録
	if ( (pp = strchr(p,':')) )
	{
		*pp = '\0';
		if (!label_map.insert( std::pair<std::string,unsigned long>(p,ad) ).second)
		{
			printf("label unique error.[%s] line:%d\n",p,ln);
			b_err = true;
			return false;
		}

		printf("%s:\n",p);	//labelを表示

		//相対ジャンプ用のマップ
		if ( (ill = delay_branch_map.find(p)) != delay_branch_map.end() )
		{
			long relative;
			for (illl = ill->second.begin();illl != ill->second.end();++illl)
			{
				relative = ad - *illl;	//相対ジャンプは命令直後のアドレス（計算用）
				outbuf[*illl-1] = (unsigned char)relative;
				printf("\tlabel[%s] : [%04x] <- [%02x]\n",p,*illl-1,(unsigned char)relative);
			}

			delay_branch_map.erase(ill);
		}

		//絶対ジャンプ用のマップ
		if ( (ill = delay_absolute_branch_map.find(p)) != delay_absolute_branch_map.end() )
		{
			for (illl = ill->second.begin();illl != ill->second.end();++illl)
			{
				outbuf[*illl] = (unsigned char)(ad & 255);
				outbuf[*illl+1] = (unsigned char)(ad >> 8);
				printf("\tlabel[%s] : [%04x] <- [%04x]\n",p,*illl,ad);
			}

			delay_absolute_branch_map.erase(ill);
		}

		//アブソリュート値用のマップ
		if ( (ila = delay_absolute_address_map.find(p)) != delay_absolute_address_map.end() )
		{
			for (ilal = ila->second.begin();ilal != ila->second.end();++ilal)
			{
				unsigned long x = ad;
				if (ilal->dr == 1)
					x += ilal->d;
				else
					x -= ilal->d;
				outbuf[ilal->ad] = (unsigned char)(x & 255);
				outbuf[ilal->ad+1] = (unsigned char)(x >> 8);
				printf("\tlabel[%s] : [%04x] <- [%04x]\n",p,ilal->ad,x);
			}

			delay_absolute_address_map.erase(ila);
		}

		//イミディエイト値のマップ用
		if ( (io = delay_offset_map.find(p)) != delay_offset_map.end() )
		{
			for (ioo = io->second.begin();ioo != io->second.end();++ioo)
			{
				if (!ioo->second)
				{
					outbuf[ioo->first] = (unsigned char)(ad & 255);
					printf("\tlabel[%s] : [%04x] <- [%02x]\n",p,ioo->first,ad & 255);
				}
				else
				{
					outbuf[ioo->first] = (unsigned char)(ad >> 8);
					printf("\tlabel[%s] : [%04x] <- [%02x]\n",p,ioo->first,ad >> 8);
				}
			}

			delay_offset_map.erase(io);
		}

		p = pp+1;
		trimleft(&p);
		if (!*p)	return false;
	}


	//アセンブラ疑似命令
	if (*p == '.')
	{
		p++;
		do
		{
			if (!*p)
			{
				break;
			}

			//org	アドレス指定
			if (strncmp(p,"org",3) == 0)
			{
				p += 3;
				trimleft(&p);
				ad = get_reteral(&p);
				if (ad >= 524288)
				{
					printf("address size error. line:%d [%s]\n",ln,p);
					b_err = true;
				}
				return false;
			}

			//align	アライン調整
			if (strncmp(p,"align",5) == 0)
			{
				p += 5;
				trimleft(&p);
				if (!*p)
				{
					printf("syntax error. line:%d [%s]\n",ln,p);
					b_err = true;
					return false;
				}
				xx = get_reteral(&p);

				int n = ad % xx;
				ad += n ? n-xx : 0;

				return false;
			}

			//予約語がない場合、マクロはここで発動することにしよう

			//	.マクロ名で起動する
			std::string s;
			LABEL l;
			get_label(&p,s,l);

			std::map<std::string,std::deque<std::string>>::const_iterator i;
			if ((i = macros.find(s)) == macros.end())
			{
				//見つからない場合
				break;
			}

			//見つかった場合、パラメータ部分を取得する
			int mct = 0;
			char *xpt[16];
			char *pt;

			pt = strtok(p,", \t");
			while(pt)
			{
				xpt[mct++] = pt;
				if (mct > 9)	break;
				pt = strtok(NULL,", \t");
			}
			
			//マクロを置換しながらアセンブルする
			std::deque<std::string>::const_iterator j;
			char ws[64];
			char bufm[32768];
			int lp;

			for (j = i->second.begin();j != i->second.end();++j)
			{
				s = *j;

				for (lp = 0;lp < mct;lp++)
				{
					wsprintf(ws,"T%d",lp+1);
					replace(s,ws,xpt[lp]);
				}

				strncpy(bufm,s.c_str(),s.length()+1);

				assemble(bufm);

				//画面表示確認用
				if (o_ad < ad)
				{
					int lx = 0;

					printf("%04x\t",o_ad);
					for (;o_ad < ad;o_ad++,lx++)
					{
						printf("%02x ",outbuf[o_ad]);
					}
					for (;lx < 4;lx++)
					{
						printf("   ");
					}
					printf("%s\n",s.c_str());
				}
			}

			return false;
		}while(0);

		//ほかの予約語はなし
		printf("syntax error. line:%d [%s]\n",ln,p);
		b_err = true;
		return false;
	}


	//前回のアドレスを設定
	o_ad = ad;


	//何もない
	if (!*p)	return false;


	//命令語の解釈

	switch(*p)
	{
		//
		//不明
		//
	default:
		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;


		//aの命令語
	case 'a':
		//adc
		if ( strncmp(p,"adc",3) == 0 )
		{
			p += 3;
			trimleft(&p);
			if (*p != 'a')
			{
				printf("illegal error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			}
			if (!checkcanma(&p))	break;
			switch(addressing(&p,&xx))
			{
			default:
				printf("illegal error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			case 1:
				outbuf[ad++] = 0x69;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 2:
				outbuf[ad++] = 0x65;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 3:
				outbuf[ad++] = 0x75;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 4:
				outbuf[ad++] = 0x6d;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
				break;
			case 5:
				outbuf[ad++] = 0x7d;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
				break;
			case 6:
				outbuf[ad++] = 0x79;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
				break;
			case 7:
				outbuf[ad++] = 0x61;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 8:
				outbuf[ad++] = 0x71;	outbuf[ad++] = (unsigned char)xx;
				break;
			}
			break;
		}

		//and
		if ( strncmp(p,"and",3) == 0 )
		{
			p += 3;
			trimleft(&p);
			if (*p != 'a')
			{
				printf("illegal error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			}
			if (!checkcanma(&p))	break;
			switch(addressing(&p,&xx))
			{
			default:
				printf("illegal error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			case 1:
				outbuf[ad++] = 0x29;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 2:
				outbuf[ad++] = 0x25;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 3:
				outbuf[ad++] = 0x35;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 4:
				outbuf[ad++] = 0x2d;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
				break;
			case 5:
				outbuf[ad++] = 0x3d;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
				break;
			case 6:
				outbuf[ad++] = 0x39;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
				break;
			case 7:
				outbuf[ad++] = 0x21;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 8:
				outbuf[ad++] = 0x31;	outbuf[ad++] = (unsigned char)xx;
				break;
			}
			break;
		}

		break;

		//Bの命令語

	case 'b':

		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;


		//cの命令語
	case 'c':

		//cli(割り込み禁止)
		if ( strncmp(p,"cli",3) == 0 )
		{
			outbuf[ad++] = 0x78;
			break;
		}

		//clc	cフラグクリア
		if ( strncmp(p,"clc",3) == 0 )
		{
			outbuf[ad++] = 0x18;
			break;
		}

		//cld	bcdモードから通常モード復帰(string転送命令の方向指定ではない)	FCでは未実装
		if ( strncmp(p,"cld",3) == 0 )
		{
			outbuf[ad++] = 0xd8;
			break;
		}

		//clv	ovフラグクリア
		if ( strncmp(p,"clv",3) == 0 )
		{
			outbuf[ad++] = 0xb8;
			break;
		}

		//call命令
		if ( strncmp(p,"call",4) == 0 )
		{
			p += 4;
			trimleft(&p);
			if (!checkreteral(p))
			{
				if ( (il = label_map.find(p)) == label_map.end() )
				{
					if ( (ill = delay_absolute_branch_map.find(p)) == delay_absolute_branch_map.end() )
					{
						std::deque<unsigned long> d;
						d.push_back(ad+1);
						delay_absolute_branch_map.insert( std::pair<std::string,std::deque<unsigned long>>(p,d) );
					}
					else
					{
						ill->second.push_back(ad+1);
					}
					xx = 0;
				}
				else
				{
					xx = (unsigned short)il->second;
				}
			}
			else
			{
				xx = (unsigned short)get_reteral(&p);
			}
			outbuf[ad++] = 0x20;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
			break;
		}

		//cmp
		if ( strncmp(p,"cmp",3) == 0 )
		{
			p += 3;
			trimleft(&p);

			switch(*p)
			{
			case 'a':
				if (!checkcanma(&p))	break;
				switch(addressing(&p,&xx))
				{
				default:
					printf("illegal error. line:%d [%s]\n",ln,p);
					b_err = true;
					break;
				case 1:
					outbuf[ad++] = 0xc9;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 2:
					outbuf[ad++] = 0xc5;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 3:
					outbuf[ad++] = 0xd5;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 4:
					outbuf[ad++] = 0xcd;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
					break;
				case 5:
					outbuf[ad++] = 0xdd;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
					break;
				case 6:
					outbuf[ad++] = 0xd9;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
					break;
				case 7:
					outbuf[ad++] = 0xc1;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 8:
					outbuf[ad++] = 0xd1;	outbuf[ad++] = (unsigned char)xx;
					break;
				}
				break;
			case 'x':
				if (!checkcanma(&p))	break;
				switch(addressing(&p,&xx))
				{
				default:
					printf("illegal error. line:%d [%s]\n",ln,p);
					b_err = true;
					break;
				case 1:
					outbuf[ad++] = 0xe0;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 2:
					outbuf[ad++] = 0xe4;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 4:
					outbuf[ad++] = 0xec;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
					break;
				}
				break;
			case 'y':
				if (!checkcanma(&p))	break;
				switch(addressing(&p,&xx))
				{
				default:
					printf("illegal error. line:%d [%s]\n",ln,p);
					b_err = true;
					break;
				case 1:
					outbuf[ad++] = 0xc0;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 2:
					outbuf[ad++] = 0xc4;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 4:
					outbuf[ad++] = 0xcc;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
					break;
				}
				break;
			}

			break;
		}

		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;

		//dの命令語
	case 'd':
		if ( strncmp(p,"dec",3) == 0 )
		{
			p += 3;
			trimleft(&p);
			switch(*p)
			{
			default:
				printf("illegal error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			case '[':
				switch(addressing(&p,&xx))
				{
				case 2:
					outbuf[ad++] = 0xc6;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 3:
					outbuf[ad++] = 0xd6;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 4:
					outbuf[ad++] = 0xce;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
					break;
				case 5:
					outbuf[ad++] = 0xde;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
					break;
				}
				break;
			case 'x':
				outbuf[ad++] = 0xca;
				break;
			case 'y':
				outbuf[ad++] = 0x88;
				break;
			}

			break;
		}

		if ( strncmp(p,"db",2) == 0 )
		{
			p += 2;
			do
			{
				trimleft(&p);
				if (*p == '\'')
				{
					p++;
					if (!(pp = strchr(p,'\'')))
					{
						printf("syntax error. line:%d [%s]\n",ln,p);
						b_err = true;
						break;
					}

					for (;p < pp;p++)
					{
						outbuf[ad++] = *p;
					}
					p++;
				}
				else
				{
					if (!checkreteral(p))	break;
					outbuf[ad++] = (unsigned char)get_reteral(&p);
				}

				trimleft(&p);
				if (*p != ',')	break;

				p++;
			}while(1);
			break;
		}


		if ( strncmp(p,"dw",2) == 0 )
		{
			p += 2;
			trimleft(&p);

			do
			{
				ad--;	//ラベル登録時に内部で+1されるので(リテラル値なら影響ないけど)
				xx = (unsigned short)get_reteral(&p);
				ad++;
				outbuf[ad++] = (unsigned char)(xx & 255);
				outbuf[ad++] = (unsigned char)(xx >> 8);
				trimleft(&p);
				if (*p != ',')	break;

				p++;
				trimleft(&p);

			}while(1);
			break;
		}


		if ( strncmp(p,"ds",2) == 0 )
		{
			p += 2;
			do
			{
				trimleft(&p);
				if (*p == '\'')
				{
					p++;
					if (!(pp = strchr(p,'\'')))
					{
						printf("syntax error. line:%d [%s]\n",ln,p);
						b_err = true;
						break;
					}

					for (;p < pp;p++)
					{
						outbuf[ad++] = *p;
					}
					p++;
				}
				else
				{
					if (!checkreteral(p))	break;
					outbuf[ad++] = (unsigned char)get_reteral(&p);
				}

				trimleft(&p);
				if (*p != ',')	break;

				p++;
			}while(1);
			break;
		}


		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;

		//eの命令語
	case 'e':


		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;



		//iの命令語
	case 'i':

		//int
		if ( strncmp(p,"int",3) == 0 )
		{
			outbuf[ad++] = 0x00;
			break;
		}

		//iret
		if ( strncmp(p,"iret",4) == 0 )
		{
			outbuf[ad++] = 0x40;
			break;
		}

		//inc
		if ( strncmp(p,"inc",3) == 0 )
		{
			p += 3;
			trimleft(&p);
			switch(*p)
			{
			case '[':
				switch(addressing(&p,&xx))
				{
				case 2:
					outbuf[ad++] = 0xe6;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 3:
					outbuf[ad++] = 0xf6;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 4:
					outbuf[ad++] = 0xee;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
					break;
				case 5:
					outbuf[ad++] = 0xfe;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
					break;
				}
				break;
			case 'x':
				outbuf[ad++] = 0xe8;
				break;
			case 'y':
				outbuf[ad++] = 0xc8;
				break;
			}

			break;
		}


		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;

		//jの命令語
	case 'j':
		//jmp
		if ( strncmp(p,"jmp",3) == 0 )
		{
			p += 3;
			trimleft(&p);

			if (*p == '[')
			{
				p++;
				xx = (unsigned short)get_reteral(&p);
				trimleft(&p);
				if (*p != ']')
				{
					printf("syntax error. line:%d [%s]\n",ln,p);
					b_err = true;
					break;
				}
				outbuf[ad++] = 0x6c;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
			}
			else
			{
				if (checkreteral(p))
				{
					xx = (unsigned short)get_reteral(&p);
					outbuf[ad++] = 0x4c;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
				}
				else
				{
					if (*p == '$')
					{
						unsigned long target_ad = ad;

						p++;
						trimleft(&p);
						if (*p == '+')
						{
							p++;
							trimleft(&p);
							xx = (unsigned short)get_reteral(&p);
							target_ad += xx;
						}
						else if (*p == '-')
						{
							p++;
							trimleft(&p);
							xx = (unsigned short)get_reteral(&p);
							target_ad -= xx;
						}

						outbuf[ad++] = 0x4c;
						outbuf[ad++] = (unsigned char)((target_ad) & 255);
						outbuf[ad++] = (unsigned char)((target_ad) >> 8);
						break;
					}

					if ( (il = label_map.find(p)) == label_map.end() )
					{
						if ( (ill = delay_absolute_branch_map.find(p)) == delay_absolute_branch_map.end() )
						{
							std::deque<unsigned long> d;
							d.push_back(ad+1);
							delay_absolute_branch_map.insert( std::pair<std::string,std::deque<unsigned long>>(p,d) );
						}
						else
							ill->second.push_back(ad+1);

						outbuf[ad++] = 0x4c;
						outbuf[ad++] = 0;
						outbuf[ad++] = 0;
						break;
					}
					outbuf[ad++] = 0x4c;
					outbuf[ad++] = (unsigned char)(il->second & 255);
					outbuf[ad++] = (unsigned char)(il->second >> 8);
				}
			}
			break;
		}

		//jnz
		if ( strncmp(p,"jnz",3) == 0 )
		{
			p += 3;
			trimleft(&p);

			long relative = 0;
			if (*p == '$')
				relative = get_relative(&p);
			else
			{
				std::string s;
				LABEL l;
				get_label(&p,s,l);
				if ( (il = label_map.find(s)) == label_map.end() )
				{
					if ( (ill = delay_branch_map.find(s)) != delay_branch_map.end() )
						ill->second.push_back(ad);
					else
					{
						std::deque<unsigned long> d;
						d.push_back(ad+2);
						delay_branch_map.insert( std::pair<std::string,std::deque<unsigned long>>(s,d) );
					}
				}
				else
					relative = il->second - ad - 2;
			}
			outbuf[ad++] = 0xd0;
			outbuf[ad++] = (unsigned char)relative;
			break;
		}
		//jz
		if ( strncmp(p,"jz",2) == 0 )
		{
			p += 2;
			trimleft(&p);

			long relative = 0;
			if (*p == '$')
				relative = get_relative(&p);
			else
			{
				std::string s;
				LABEL l;
				get_label(&p,s,l);
				if ( (il = label_map.find(s)) == label_map.end() )
				{
					if ( (ill = delay_branch_map.find(s)) != delay_branch_map.end() )
						ill->second.push_back(ad);
					else
					{
						std::deque<unsigned long> d;
						d.push_back(ad+2);
						delay_branch_map.insert( std::pair<std::string,std::deque<unsigned long>>(s,d) );
					}
				}
				else
					relative = il->second - ad - 2;
			}
			outbuf[ad++] = 0xf0;
			outbuf[ad++] = (unsigned char)relative;
			break;
		}

		//jnc
		if ( strncmp(p,"jnc",3) == 0 )
		{
			p += 3;
			trimleft(&p);

			long relative = 0;
			if (*p == '$')
				relative = get_relative(&p);
			else
			{
				std::string s;
				LABEL l;
				get_label(&p,s,l);
				if ( (il = label_map.find(s)) == label_map.end() )
				{
					if ( (ill = delay_branch_map.find(s)) != delay_branch_map.end() )
						ill->second.push_back(ad);
					else
					{
						std::deque<unsigned long> d;
						d.push_back(ad+2);
						delay_branch_map.insert( std::pair<std::string,std::deque<unsigned long>>(s,d) );
					}
				}
				else
					relative = il->second - ad - 2;
			}
			outbuf[ad++] = 0x90;
			outbuf[ad++] = (unsigned char)relative;
			break;
		}
		//jc
		if ( strncmp(p,"jc",2) == 0 )
		{
			p += 2;
			trimleft(&p);

			long relative = 0;
			if (*p == '$')
				relative = get_relative(&p);
			else
			{
				std::string s;
				LABEL l;
				get_label(&p,s,l);
				if ( (il = label_map.find(s)) == label_map.end() )
				{
					if ( (ill = delay_branch_map.find(s)) != delay_branch_map.end() )
						ill->second.push_back(ad);
					else
					{
						std::deque<unsigned long> d;
						d.push_back(ad+2);
						delay_branch_map.insert( std::pair<std::string,std::deque<unsigned long>>(s,d) );
					}
				}
				else
					relative = il->second - ad - 2;
			}
			outbuf[ad++] = 0xb0;
			outbuf[ad++] = (unsigned char)relative;
			break;
		}

		//jns
		if ( strncmp(p,"jns",3) == 0 )
		{
			p += 3;
			trimleft(&p);

			long relative = 0;
			if (*p == '$')
				relative = get_relative(&p);
			else
			{
				std::string s;
				LABEL l;
				get_label(&p,s,l);
				if ( (il = label_map.find(s)) == label_map.end() )
				{
					if ( (ill = delay_branch_map.find(s)) != delay_branch_map.end() )
						ill->second.push_back(ad);
					else
					{
						std::deque<unsigned long> d;
						d.push_back(ad+2);
						delay_branch_map.insert( std::pair<std::string,std::deque<unsigned long>>(s,d) );
					}
				}
				else
					relative = il->second - ad - 2;
			}
			outbuf[ad++] = 0x10;
			outbuf[ad++] = (unsigned char)relative;
			break;
		}
		//js
		if ( strncmp(p,"js",2) == 0 )
		{
			p += 2;
			trimleft(&p);

			long relative = 0;
			if (*p == '$')
				relative = get_relative(&p);
			else
			{
				std::string s;
				LABEL l;
				get_label(&p,s,l);
				if ( (il = label_map.find(s)) == label_map.end() )
				{
					if ( (ill = delay_branch_map.find(s)) != delay_branch_map.end() )
						ill->second.push_back(ad);
					else
					{
						std::deque<unsigned long> d;
						d.push_back(ad+2);
						delay_branch_map.insert( std::pair<std::string,std::deque<unsigned long>>(s,d) );
					}
				}
				else
					relative = il->second - ad - 2;
			}
			outbuf[ad++] = 0x30;
			outbuf[ad++] = (unsigned char)relative;
			break;
		}


		//jno
		if ( strncmp(p,"jno",3) == 0 )
		{
			p += 3;
			trimleft(&p);

			long relative = 0;
			if (*p == '$')
				relative = get_relative(&p);
			else
			{
				std::string s;
				LABEL l;
				get_label(&p,s,l);
				if ( (il = label_map.find(s)) == label_map.end() )
				{
					if ( (ill = delay_branch_map.find(s)) != delay_branch_map.end() )
						ill->second.push_back(ad);
					else
					{
						std::deque<unsigned long> d;
						d.push_back(ad+2);
						delay_branch_map.insert( std::pair<std::string,std::deque<unsigned long>>(s,d) );
					}
				}
				else
					relative = il->second - ad - 2;
			}
			outbuf[ad++] = 0x50;
			outbuf[ad++] = (unsigned char)relative;
			break;
		}
		//jo
		if ( strncmp(p,"jo",2) == 0 )
		{
			p += 2;
			trimleft(&p);

			long relative = 0;
			if (*p == '$')
				relative = get_relative(&p);
			else
			{
				std::string s;
				LABEL l;
				get_label(&p,s,l);
				if ( (il = label_map.find(s)) == label_map.end() )
				{
					if ( (ill = delay_branch_map.find(s)) != delay_branch_map.end() )
						ill->second.push_back(ad);
					else
					{
						std::deque<unsigned long> d;
						d.push_back(ad+2);
						delay_branch_map.insert( std::pair<std::string,std::deque<unsigned long>>(s,d) );
					}
				}
				else
					relative = il->second - ad - 2;
			}
			outbuf[ad++] = 0x70;
			outbuf[ad++] = (unsigned char)relative;
			break;
		}

		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;

		//lの命令語

	case 'l':


		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;




		//mの命令語
	case 'm':
		if ( strncmp(p,"mov",3) == 0 )
		{
			p += 3;
			trimleft(&p);

			switch(*p)
			{
			case 'a':
				if (!checkcanma(&p))	break;
				switch(addressing(&p,&xx))
				{
				default:	break;
				case 1:	//immidiate
					outbuf[ad++] = 0xa9;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 2:	//zeropage
					outbuf[ad++] = 0xa5;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 3:	//zeropage x
					outbuf[ad++] = 0xb5;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 4:	//absolute
					outbuf[ad++] = 0xad;
					outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
					break;
				case 5:	//absolute x
					outbuf[ad++] = 0xbd;
					outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
					break;
				case 6:	//absolute y
					outbuf[ad++] = 0xb9;
					outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
					break;
				case 7:	//indirect x
					outbuf[ad++] = 0xa1;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 8:	//absolute y
					outbuf[ad++] = 0xb1;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 11:	// a,x
					outbuf[ad++] = 0x8a;
					break;
				case 12:	// a,y
					outbuf[ad++] = 0x98;
					break;
				}
				break;

			case 'x':
				if (!checkcanma(&p))	break;
				switch(addressing(&p,&xx))
				{
				default:	b_err = true;	break;
				case 1:	//immidiate
					outbuf[ad++] = 0xa2;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 2:	//zeropage
					outbuf[ad++] = 0xa6;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 9:	//zeropage y
					outbuf[ad++] = 0xb6;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 4:	//absolute
					outbuf[ad++] = 0xae;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
					break;
				case 6:	//absolute y
					outbuf[ad++] = 0xbe;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
					break;
				case 10:	//x,a
					outbuf[ad++] = 0xaa;
					break;
				case 13:	//x,s
					outbuf[ad++] = 0xba;
					break;
				}
				break;

			case 'y':
				if (!checkcanma(&p))	break;
				switch(addressing(&p,&xx))
				{
				default:	b_err = true;	break;
				case 1:	//immidiate
					outbuf[ad++] = 0xa0;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 2:	//zeropage
					outbuf[ad++] = 0xa4;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 3:	//zeropage x
					outbuf[ad++] = 0xb4;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 4:	//absolute
					outbuf[ad++] = 0xac;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
					break;
				case 5:	//absolute x
					outbuf[ad++] = 0xbc;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
					break;
				case 10:	//y,a
					outbuf[ad++] = 0xa8;
					break;
				}
				break;

			case 's':
				p++;
				if (*p != 'p')
				{
					printf("syntax error. line:%d [%s]\n",ln,p);
					b_err = true;
					break;
				}
				if (!checkcanma(&p))	break;
				trimleft(&p);
				if (*p != 'x')
				{
					printf("syntax error. line:%d [%s]\n",ln,p);
					b_err = true;
					break;
				}
				outbuf[ad++] = 0x9a;
				break;

				//sta系
			case '[':
				n = addressing(&p,&xx);
				if (!n)
				{
					printf("syntax error. line:%d [%s]\n",ln,p);
					b_err = true;
					break;
				}
				if (!checkcanma(&p))	break;
				trimleft(&p);
				switch(*p)
				{
				case 'a':
					switch(n)
					{
					default:
						printf("syntax error. line:%d [%s]\n",ln,p);
						b_err = true;
						break;
					case 2:
						outbuf[ad++] = 0x85;	outbuf[ad++] = (unsigned char)xx;
						break;
					case 3:
						outbuf[ad++] = 0x95;	outbuf[ad++] = (unsigned char)xx;
						break;
					case 4:
						outbuf[ad++] = 0x8d;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
						break;
					case 5:
						outbuf[ad++] = 0x9d;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
						break;
					case 6:
						outbuf[ad++] = 0x99;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
						break;
					case 7:
						outbuf[ad++] = 0x81;	outbuf[ad++] = (unsigned char)xx;
						break;
					case 8:
						outbuf[ad++] = 0x91;	outbuf[ad++] = (unsigned char)xx;
						break;
					}
					break;
				case 'x':
					switch(n)
					{
					default:
						printf("syntax error. line:%d [%s]\n",ln,p);
						b_err = true;
						break;
					case 2:
						outbuf[ad++] = 0x86;	outbuf[ad++] = (unsigned char)xx;
						break;
					case 9:
						outbuf[ad++] = 0x96;	outbuf[ad++] = (unsigned char)xx;
						break;
					case 4:
						outbuf[ad++] = 0x8e;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
						break;
					}
					break;
				case 'y':
					switch(n)
					{
					default:
						printf("syntax error. line:%d [%s]\n",ln,p);
						b_err = true;
						break;
					case 2:
						outbuf[ad++] = 0x84;	outbuf[ad++] = (unsigned char)xx;
						break;
					case 3:
						outbuf[ad++] = 0x94;	outbuf[ad++] = (unsigned char)xx;
						break;
					case 4:
						outbuf[ad++] = 0x8c;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
						break;
					}
					break;
				}
				break;

			}

			break;
		}


		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;
			

		//nの命令語
	case 'n':
		if ( strncmp(p,"nop",3) == 0 )
		{
			outbuf[ad++] = 0xea;
			break;
		}

		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;



		//oの命令語

	case 'o':
		if ( strncmp(p,"or",2) == 0 )
		{
			p += 2;
			trimleft(&p);
			if (*p != 'a')
			{
				printf("illegal error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			}
			if (!checkcanma(&p))	break;
			switch(addressing(&p,&xx))
			{
			default:
				printf("illegal error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			case 1:
				outbuf[ad++] = 0x09;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 2:
				outbuf[ad++] = 0x05;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 3:
				outbuf[ad++] = 0x15;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 4:
				outbuf[ad++] = 0x0d;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
				break;
			case 5:
				outbuf[ad++] = 0x1d;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
				break;
			case 6:
				outbuf[ad++] = 0x19;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
				break;
			case 7:
				outbuf[ad++] = 0x01;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 8:
				outbuf[ad++] = 0x11;	outbuf[ad++] = (unsigned char)xx;
				break;
			}
			break;
		}

		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;


		//pの命令語
	case 'p':
		//push
		if ( strncmp(p,"push",4) == 0 )
		{
			p += 4;
			trimleft(&p);
			switch(*p)
			{
			case 'a':
				outbuf[ad++] = 0x48;
				break;
			case 'p':
				outbuf[ad++] = 0x08;
				break;
			}
			break;
		}

		//pop
		if ( strncmp(p,"pop",3) == 0 )
		{
			p += 3;
			trimleft(&p);
			switch(*p)
			{
			case 'a':
				outbuf[ad++] = 0x68;
				break;
			case 'p':
				outbuf[ad++] = 0x28;
				break;
			}
			break;
		}

		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;



		//rの命令語

	case 'r':
		//ret
		if ( strncmp(p,"ret",3) == 0 )
		{
			outbuf[ad++] = 0x60;
			break;
		}

		//rcl
		if ( strncmp(p,"rcl",3) == 0 )
		{
			p += 3;
			trimleft(&p);
			switch(*p)
			{
			case 'a':
				outbuf[ad++] = 0x2a;
				break;
			case '[':
				switch(addressing(&p,&xx))
				{
				default:
					printf("illegal error. line:%d [%s]\n",ln,p);
					b_err = true;
					break;
				case 2:
					outbuf[ad++] = 0x26;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 3:
					outbuf[ad++] = 0x36;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 4:
					outbuf[ad++] = 0x2e;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
					break;
				case 5:
					outbuf[ad++] = 0x3e;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
					break;
				}
				break;
			}
			break;
		}

		//rcr
		if ( strncmp(p,"rcr",3) == 0 )
		{
			p += 3;
			trimleft(&p);
			switch(*p)
			{
			case 'a':
				outbuf[ad++] = 0x6a;
				break;
			case '[':
				switch(addressing(&p,&xx))
				{
				default:
					printf("illegal error. line:%d [%s]\n",ln,p);
					b_err = true;
					break;
				case 2:
					outbuf[ad++] = 0x66;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 3:
					outbuf[ad++] = 0x76;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 4:
					outbuf[ad++] = 0x6e;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
					break;
				case 5:
					outbuf[ad++] = 0x7e;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
					break;
				}
				break;
			}
			break;
		}

		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;



		//sの命令語

	case 's':

		//sed	bcdモードへ変更　FCでは未実装
		if ( strncmp(p,"sed",3) == 0 )
		{
			outbuf[ad++] = 0xf8;
			break;
		}

		//stc cフラグon
		if ( strncmp(p,"stc",3) == 0 )
		{
			outbuf[ad++] = 0x38;
			break;
		}

		//sti(割り込み許可)
		if ( strncmp(p,"sti",3) == 0 )
		{
			outbuf[ad++] = 0x58;
			break;
		}

		//sbb
		if ( strncmp(p,"sbb",3) == 0 )
		{
			p += 3;
			trimleft(&p);
			if (*p != 'a')
			{
				printf("illegal error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			}
			if (!checkcanma(&p))	break;
			switch(addressing(&p,&xx))
			{
			default:
				printf("illegal error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			case 1:
				outbuf[ad++] = 0xe9;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 2:
				outbuf[ad++] = 0xe5;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 3:
				outbuf[ad++] = 0xf5;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 4:
				outbuf[ad++] = 0xed;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
				break;
			case 5:
				outbuf[ad++] = 0xfd;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
				break;
			case 6:
				outbuf[ad++] = 0xf9;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
				break;
			case 7:
				outbuf[ad++] = 0xe1;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 8:
				outbuf[ad++] = 0xf1;	outbuf[ad++] = (unsigned char)xx;
				break;
			}
			break;
		}

		//shl
		if ( strncmp(p,"shl",3) == 0 )
		{
			p += 3;
			trimleft(&p);
			switch(*p)
			{
			case 'a':
				outbuf[ad++] = 0x0a;
				break;

			case '[':
				switch(addressing(&p,&xx))
				{
				default:
					printf("illegal error. line:%d [%s]\n",ln,p);
					b_err = true;
					break;
				case 2:
					outbuf[ad++] = 0x06;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 3:
					outbuf[ad++] = 0x16;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 4:
					outbuf[ad++] = 0x0e;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
					break;
				case 5:
					outbuf[ad++] = 0x1e;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
					break;
				}
				break;
			}
			break;
		}

		//shr
		if ( strncmp(p,"shr",3) == 0 )
		{
			p += 3;
			trimleft(&p);
			switch(*p)
			{
			case 'a':
				outbuf[ad++] = 0x4a;
				break;

			case '[':
				switch(addressing(&p,&xx))
				{
				default:
					printf("illegal error. line:%d [%s]\n",ln,p);
					b_err = true;
					break;
				case 2:
					outbuf[ad++] = 0x46;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 3:
					outbuf[ad++] = 0x56;	outbuf[ad++] = (unsigned char)xx;
					break;
				case 4:
					outbuf[ad++] = 0x4e;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
					break;
				case 5:
					outbuf[ad++] = 0x5e;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
					break;
				}
				break;
			}
			break;
		}


		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;


		//tの命令語

	case 't':
		//test
		if ( strncmp(p,"test",4) == 0 )
		{
			p += 4;
			trimleft(&p);
			if (*p != 'a')
			{
				printf("illegal error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			}
			if (!checkcanma(&p))	break;
			switch(addressing(&p,&xx))
			{
			default:
				printf("illegal error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			case 2:
				outbuf[ad++] = 0x24;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 4:
				outbuf[ad++] = 0x2c;	outbuf[ad++] = xx & 255;	outbuf[ad++] = xx >> 8;
				break;
			}
			break;
		}

		printf("unknown mnemonic line:%d [%s] [%s]\n",ln,p,buf);
		b_err = true;
		break;


		//xの命令語

	case 'x':
		//xor
		if ( strncmp(p,"xor",3) == 0 )
		{
			p += 3;
			trimleft(&p);
			if (*p != 'a')
			{
				printf("illegal error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			}
			if (!checkcanma(&p))	break;
			switch(addressing(&p,&xx))
			{
			default:
				printf("illegal error. line:%d [%s]\n",ln,p);
				b_err = true;
				break;
			case 1:
				outbuf[ad++] = 0x49;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 2:
				outbuf[ad++] = 0x45;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 3:
				outbuf[ad++] = 0x55;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 4:
				outbuf[ad++] = 0x4d;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
				break;
			case 5:
				outbuf[ad++] = 0x5d;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
				break;
			case 6:
				outbuf[ad++] = 0x59;	outbuf[ad++] = (unsigned char)(xx & 255);	outbuf[ad++] = (unsigned char)(xx >> 8);
				break;
			case 7:
				outbuf[ad++] = 0x41;	outbuf[ad++] = (unsigned char)xx;
				break;
			case 8:
				outbuf[ad++] = 0x51;	outbuf[ad++] = (unsigned char)xx;
				break;
			}
			break;
		}


		printf("unknown mnemonic line:%d [%s]\n",ln,p);
		b_err = true;
		break;
	}


	//オーバーフロー
	if (ad > 524288+1)
	{
		printf("error: assemble size overflow.\n");
		b_err = true;
	}

	if (ad > max_ad)	max_ad = ad;


	return true;
}


void casm6502::warnings()
{
	//消されていないdelayがある場合警告
	if ( !delay_branch_map.empty() )
	{
		printf("warning! delay_branch_map\n");
		for (ill = delay_branch_map.begin();ill != delay_branch_map.end();++ill)
		{
			printf("\t[%s] not used!\n",ill->first.c_str());
		}
	}
	if ( !delay_absolute_branch_map.empty() )
	{
		printf("warning! delay_absolute_branch_map\n");
		for (ill = delay_absolute_branch_map.begin();ill != delay_absolute_branch_map.end();++ill)
		{
			printf("\t[%s] not used!\n",ill->first.c_str());
		}
	}
	if ( !delay_absolute_address_map.empty() )
	{
		printf("warning! delay_absolute_address_map\n");
		for (ila = delay_absolute_address_map.begin();ila != delay_absolute_address_map.end();++ila)
		{
			printf("\t[%s] not used!\n",ila->first.c_str());
		}
	}
	if ( !delay_offset_map.empty() )
	{
		printf("warning! delay_offset_map\n");
		for (io = delay_offset_map.begin();io != delay_offset_map.end();++io)
		{
			printf("\t[%s] not used!\n",io->first.c_str());
		}
	}
}








