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

	//������u������
	std::string &replace(std::string &str,std::string src,std::string dst);

	//�g��������
	void trimleft(char **p);

	// , �̂܂Ƃߏ���
	bool checkcanma(char **p);

	//�擪���������e������
	bool checkreteral(char *p);

	//���x���̎擾
	void get_label(char **p,std::string &s,LABEL &l);

	//���e�����l�̎擾
	unsigned long get_reteral(char **p);

	//���΃A�h���X���̎擾
	long get_relative(char **p); 



	//	1:�C�~�f�B�G�C�g		00h
	//	2:�[���y�[�W			[00h]
	//	3:�[���y�[�WX			[x+00h]
	//	4:�A�u�\�����[�g		[1234h]
	//	5:�A�u�\�����[�gX		[x+1234h]
	//	6:�A�u�\�����[�gY		[y+1234h]
	//	7:�C���_�C���N�gX		[[x+12h]]
	//	8:�C���_�C���N�gY		[[12h]+y]
	//	9:�[���y�[�WY			[y+00h]
	//
	int addressing(char **p,unsigned short *xx);






};

