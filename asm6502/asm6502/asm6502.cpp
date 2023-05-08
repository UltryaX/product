// asm6502.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//


//
//	2019/6/16	v1.0
//	2019/6/22	v1.1	���낢��C��
//		 6/24	v1.2	�I�t�Z�b�g���Z�Ȃǂ�ǉ�
//		 6/25	v1.3	���΃W�����v�p$��ǉ��B1�s�T�u���[�`����
//		 6/27	v1.4	�}�N���ǉ�,align�ǉ�
//		7/26	v1.5	�I�v�V�����Ƃ��ǉ��Amax 512KB�A�A�h���X�T�C�Y�G���[�Ƃ��ǉ�
//
//	�c��
//		o		l02+1���̑Ή�
//		o		1�s�T�u���[�`����
//		o		���΃W�����v���߂� $ �̎g�p
//		o		db 0,1,'abcdef',0	�Ƃ��̍���
//		o		�}�N��
//
//				include�Ń\�[�X�}�[�W
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

			case 0:	//option�l����
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

				//�����Ȃ��Ȃ�A���̓t�@�C�����@������1��ڂ���
				if (opt.inputfile.empty())
				{
					opt.inputfile = argv[lp];
					continue;
				}

				//�G���[
				printf("invalid option : [No.%d:%s]\n",lp,argv[1]);
				b_err = true;
				break;

				//�l�̕ۑ�
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
				"x86�� 6502 assembler v 1.5\n"
				"Copyright ultrya, rev 19/07/26\n\n"
				"usage: asm6502 assemblefile [option]\n"
				"\n"
				"option: -offset <offset value>  ...  �A�Z���u�������o�C�i���̏����o���I�t�Z�b�g�ʒu���w�肵�܂�\n"
				"        -size <size value>      ...  �A�Z���u�������o�C�i���̃T�C�Y�𖾎��I�Ɏw�肵�܂�\n"
				"        -output <filename>      ...  �A�Z���u�������o�C�i���̏����o���t�@�C�������w�肵�܂�\n"
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
		//�\���p�ɃR�s�[
		strcpy(buf2,buf);
		if ((p = strrchr(buf2,'\n')))	*p = '\0';

		a.ln++;

		//�A�Z���u��
		if (a.assemble(buf))
		{
			//��ʕ\���m�F�p
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

		//�G���[���������Ă����璆�f����
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
			//�����o���T�C�Y�v�Z
			unsigned long sz = !opt.size ? a.max_ad : opt.size;

			if (opt.offset)
			{
				//�w�肳�ꂽ�I�t�Z�b�g���A�ő�A�h���X�����傫��
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

			//�t�@�C���������o��
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


