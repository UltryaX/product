#pragma once
class casm6502
{
public:
	casm6502(void)
	{
		b_err = false;
		ln = 0;
		ad = 0;
		b_macromode = false;
	};
	~casm6502(void);

	void clear()
	{
		label_map.clear();
		delay_branch_map.clear();
		delay_absolute_branch_map.clear();
		delay_absolute_address_map.clear();
		delay_offset_map.clear();
		tmp_macro.clear();
		macros.clear();
	};


public:
	bool b_err;
	int ln;
	unsigned long ad;
	unsigned long o_ad;
	unsigned char *outbuf;

protected:

	std::map< std::string,unsigned long > label_map;
	std::map< std::string,unsigned long >::const_iterator il;

	std::map< std::string,std::deque<unsigned long> > delay_branch_map;
	std::map< std::string,std::deque<unsigned long> > delay_absolute_branch_map;
	std::map< std::string,std::deque<unsigned long> >::iterator ill;
	std::deque<unsigned long>::const_iterator illl;

	struct LABEL {
		LABEL()
		{
			ad = 0;
			d = 0;
			dr = 0;
		};
		unsigned long ad;
		unsigned long d;
		unsigned char dr;
	};
	std::map< std::string,std::deque<LABEL> > delay_absolute_address_map;
	std::map< std::string,std::deque<LABEL> >::iterator ila;
	std::deque<LABEL>::const_iterator ilal;

	std::map< std::string,std::deque<std::pair<unsigned long,unsigned char>>> delay_offset_map;
	std::map< std::string,std::deque<std::pair<unsigned long,unsigned char>>>::iterator io;
	std::deque< std::pair<unsigned long,unsigned char>>::iterator ioo;


	bool b_macromode;
	std::deque<std::string> tmp_macro;
	std::string macro_name;
	std::map<std::string,std::deque<std::string>> macros;




public:
	bool assemble(char *buf);
	void casm6502::warnings();



protected:

	//文字列置換処理
	std::string &replace(std::string &str,std::string src,std::string dst);

	//トリム処理
	void trimleft(char **p);

	// , のまとめ処理
	bool checkcanma(char **p);

	//先頭文字がリテラルか
	bool checkreteral(char *p);

	//ラベルの取得
	void get_label(char **p,std::string &s,LABEL &l);

	//リテラル値の取得
	unsigned long get_reteral(char **p);

	//相対アドレス情報の取得
	long get_relative(char **p); 



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
	int addressing(char **p,unsigned short *xx);






};

