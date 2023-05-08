#include "StdAfx.h"
#include "cda6502.h"


cda6502::cda6502(void)
{
	p = NULL;
	ad = 0;
}


cda6502::~cda6502(void)
{
}

int cda6502::da()
{
	unsigned char *op = p;
	char codes[64];


	switch(*p++)
	{
	default:
		sprintf(codes,"???");
		break;

		//0x00

	case 0x00:	//BRK impl
		sprintf(codes,"irq");
		break;
	case 0x01:	//ORA X,ind
		x = *p++;
		sprintf(codes,"or\ta,[[x+%02x]]",x);
		break;
	case 0x05:	//ORA zpg
		x = *p++;
		sprintf(codes,"or\ta,[%02x]",x);
		break;
	case 0x06:	//ASL zpg
		x = *p++;
		sprintf(codes,"shl\t[%02x],1",x);
		break;
	case 0x08:	//PHP impl
		sprintf(codes,"pushf");
		break;
	case 0x09:	//ORA #
		x = *p++;
		sprintf(codes,"or\ta,%02x",x);
		break;
	case 0x0a:	//ASL A
		sprintf(codes,"shl\ta,1");
		break;
	case 0x0d:	//ORA abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"or\ta,[%04x]",xx);
		break;
	case 0x0e:	//ASL abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"shl\t[%04x],1",xx);
		break;

		//0x10
	case 0x10:	//BPL rel
		x = *p++;
		rad = (ad+2) + (signed char)x;
		sprintf(codes,"jns\t%04x",rad);
		break;
	case 0x11:	//ORA ind,Y
		x = *p++;
		sprintf(codes,"or\ta,[[%02x]+y]",x);
		break;
	case 0x15:	//ORA zpg,X
		x = *p++;
		sprintf(codes,"or\ta,[x+%02x]",x);
		break;
	case 0x16:	//ASL zpg,X
		x = *p++;
		sprintf(codes,"shl\t[x+%02x],1",x);
		break;
	case 0x18:	//CLC impl
		sprintf(codes,"clc");	//clear carry flag
		break;
	case 0x19:	//ORA abs,Y
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"or\ta,[y+%04x]",xx);
		break;
	case 0x1d:	//ORA abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"or\ta,[x+%04x]",xx);
		break;
	case 0x1e:	//ASL abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"shl\t[x+%04x],1",xx);
		break;

		//0x20

	case 0x20:	//JSR abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"call\t%04x",xx);
		break;
	case 0x21:	//AND X,ind
		x = *p++;
		sprintf(codes,"and\ta,[[x+%02x]]",x);
		break;
	case 0x24:	//BIT zpg
		x = *p++;
		sprintf(codes,"test\ta,[%02x] (N,V,Z)",x);		//zpg側のbitはNへ7 bit6はVフラグへコピー
		break;
	case 0x25:	//AND zpg
		x = *p++;
		sprintf(codes,"and\ta,[%02x]",x);
		break;
	case 0x26:	//ROL zpg
		x = *p++;
		sprintf(codes,"rol\t[%02x],1",x);
		break;
	case 0x28:	//PLP impl
		sprintf(codes,"popf");
		break;
	case 0x29:	//AND #
		x = *p++;
		sprintf(codes,"and\ta,%02x",x);
		break;
	case 0x2a:	//ROL A
		sprintf(codes,"rol\ta,1");
		break;
	case 0x2c:	//BIT abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"test\ta,[%04x]",xx);
		break;
	case 0x2d:	//AND abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"and\ta,[%04x]",xx);
		break;
	case 0x2e:	//ROL abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"rol\t[%04x],1",xx);
		break;

		//0x30

	case 0x30:	//BMI rel
		x = *p++;
		rad = (ad+2) + (signed char)x;
		sprintf(codes,"js\t%04x",rad);
		break;
	case 0x31:	//AND ind,Y
		x = *p++;
		sprintf(codes,"and\ta,[[%02x]+y]",x);
		break;
	case 0x35:	//AND zpg,X
		x = *p++;
		sprintf(codes,"and\ta,[x+%02x]",x);
		break;
	case 0x36:	//ROL zpg,X
		x = *p++;
		sprintf(codes,"rol\t[x+%02x],1",x);
		break;
	case 0x38:	//SEC impl
		sprintf(codes,"stc");		//set carry flag
		break;
	case 0x39:	//AND abs,Y
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"and\ta,[y+%04x]",xx);
		break;
	case 0x3d:	//AND abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"and\ta,[x+%04x]",xx);
		break;
	case 0x3e:	//ROL abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"rol\t[x+%04x],1",xx);
		break;

		//0x40

	case 0x40:	//RTI impl
		sprintf(codes,"iret");
		break;
	case 0x41:	//EOR X,ind
		x = *p++;
		sprintf(codes,"xor\ta,[[x+%02x]]",x);
		break;
	case 0x45:	//EOR zpg
		x = *p++;
		sprintf(codes,"xor\ta,[%02x]",x);
		break;
	case 0x46:	//LSR zpg
		x = *p++;
		sprintf(codes,"shr\t[%02x],1",x);
		break;
	case 0x48:	//PHA impl
		sprintf(codes,"push\ta");
		break;
	case 0x49:	//EOR #
		x = *p++;
		sprintf(codes,"xor\ta,%02x",x);
		break;
	case 0x4a:	//LSR A
		sprintf(codes,"shr\ta,1");
		break;
	case 0x4c:	//JMP abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"jmp\t%04x",xx);
		break;
	case 0x4d:	//EOR abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"xor\ta,[%04x]",xx);
		break;
	case 0x4e:	//LSR abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"shr\t[%04x],1",xx);
		break;

		//0x50

	case 0x50:	//BVC rel			jmp none overflow flag
		x = *p++;
		rad = (ad+2) + (signed char)x;
		sprintf(codes,"jno\t%04x",rad);
		break;
	case 0x51:	//EOR ind,Y
		x = *p++;
		sprintf(codes,"xor\ta,[[%02x]+y]",x);
		break;
	case 0x55:	//EOR zpg,X
		x = *p++;
		sprintf(codes,"xor\ta,[x+%02x]",x);
		break;
	case 0x56:	//LSR zpg,X
		x = *p++;
		sprintf(codes,"shr\t[x+%02x],1",x);
		break;
	case 0x58:	//CLI impl			clear interrupt mask flag (interrupt enable)
		sprintf(codes,"sti");
		break;
	case 0x59:	//EOR abs,Y
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"xor\ta,[y+%04x]",xx);
		break;
	case 0x5d:	//EOR abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"xor\ta,[x+%04x]",xx);
		break;
	case 0x5e:	//LSR abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"shr\t[x+%04x],1",xx);
		break;

		//0x60

	case 0x60:	//RTS impl
		sprintf(codes,"ret");
		break;
	case 0x61:	//ADC X,ind
		x = *p++;
		sprintf(codes,"adc\ta,[[x+%02x]]",x);
		break;
	case 0x65:	//ADC zpg
		x = *p++;
		sprintf(codes,"adc\ta,[%02x]",x);
		break;
	case 0x66:	//ROR zpg
		x = *p++;
		sprintf(codes,"ror\t[%02x],1",x);
		break;
	case 0x68:	//PLA impl
		sprintf(codes,"pop\ta");
		break;
	case 0x69:	//ADC #
		x = *p++;
		sprintf(codes,"adc\ta,%02x",x);
		break;
	case 0x6a:	//ROR A
		sprintf(codes,"ror\ta,1");
		break;
	case 0x6c:	//JMP ind
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"jmp\t[%04x]",xx);
		break;
	case 0x6d:	//ADC abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"adc\ta,[%04x]",xx);
		break;
	case 0x6e:	//ROR abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"ror\t[%04x],1",xx);
		break;

		//0x70

	case 0x70:	//BVS rel
		x = *p++;
		rad = (ad+2) + (signed char)x;
		sprintf(codes,"jeo\t%04x",rad);			//jmp overflow flag
		break;
	case 0x71:	//ADC ind,Y
		x = *p++;
		sprintf(codes,"adc\ta,[[%02x]+y]",x);
		break;
	case 0x75:	//ADC zpg,X
		x = *p++;
		sprintf(codes,"adc\ta,[x+%02x]",x);
		break;
	case 0x76:	//ROR zpg,X
		x = *p++;
		sprintf(codes,"ror\t[x+%02x],1",x);
		break;
	case 0x78:	//SEI impl
		sprintf(codes,"cli");					//set interrput mask flag	cli相当（割り込み禁止）
		break;
	case 0x79:	//ADC abs,Y
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"adc\ta,[y+%04x]",xx);
		break;
	case 0x7d:	//ADC abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"adc\ta,[x+%04x]",xx);
		break;
	case 0x7e:	//ROR abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"ror\t[x+%04x],1",xx);
		break;

		//0x80

	case 0x81:	//STA X,ind
		x = *p++;
		sprintf(codes,"mov\t[[x+%02x]],a",x);
		break;
	case 0x84:	//STY zpg
		x = *p++;
		sprintf(codes,"mov\t[%02x],y",x);
		break;
	case 0x85:	//STA zpg
		x = *p++;
		sprintf(codes,"mov\t[%02x],a",x);
		break;
	case 0x86:	//STX zpg
		x = *p++;
		sprintf(codes,"mov\t[%02x],x",x);
		break;
	case 0x88:	//DEY impl
		sprintf(codes,"dec\ty");
		break;
	case 0x8a:	//TXA impl
		sprintf(codes,"mov\ta,x");
		break;
	case 0x8c:	//STY abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"mov\t[%04x],y",xx);
		break;
	case 0x8d:	//STA abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"mov\t[%04x],a",xx);
		break;
	case 0x8e:	//STX abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"mov\t[%04x],x",xx);
		break;

		//0x90

	case 0x90:	//BCC rel
		x = *p++;
		rad = (ad+2) + (signed char)x;
		sprintf(codes,"jnc\t%04x",rad);			//jmp no carry flag
		break;
	case 0x91:	//STA ind,Y
		x = *p++;
		sprintf(codes,"mov\t[[%02x]+y],a",x);
		break;
	case 0x94:	//STY zpg,X
		x = *p++;
		sprintf(codes,"mov\t[x+%02x],x",x);
		break;
	case 0x95:	//STA zpg,X
		x = *p++;
		sprintf(codes,"mov\t[x+%02x],a",x);
		break;
	case 0x96:	//STX zpg,Y
		x = *p++;
		sprintf(codes,"mov\t[y+%02x],x",x);
		break;
	case 0x98:	//TYA impl
		sprintf(codes,"mov\ta,y");
		break;
	case 0x99:	//STA abs,Y
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"mov\t[y+%04x],a",xx);
		break;
	case 0x9a:	//TXS impl
		sprintf(codes,"mov\tsp,x");
		break;
	case 0x9d:	//STA abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"mov\t[x+%04x],a",xx);
		break;

		//0xa0

	case 0xa0:	//LDY #
		x = *p++;
		sprintf(codes,"mov\ty,%02x",x);
		break;
	case 0xa1:	//LDA X,ind
		x = *p++;
		sprintf(codes,"mov\ta,[[x+%02x]]",x);
		break;
	case 0xa2:	//LDX #
		x = *p++;
		sprintf(codes,"mov\tx,%02x",x);
		break;
	case 0xa4:	//LDY zpg
		x = *p++;
		sprintf(codes,"mov\ty,[%02x]",x);
		break;
	case 0xa5:	//LDA zpg
		x = *p++;
		sprintf(codes,"mov\ta,[%02x]",x);
		break;
	case 0xa6:	//LDX zpg
		x = *p++;
		sprintf(codes,"mov\tx,[%02x]",x);
		break;
	case 0xa8:	//TAY impl
		sprintf(codes,"mov\ty,a");
		break;
	case 0xa9:	//LDA #
		x = *p++;
		sprintf(codes,"mov\ta,%02x",x);
		break;
	case 0xaa:	//TAX impl
		sprintf(codes,"mov\tx,a");
		break;
	case 0xac:	//LDY abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"mov\ty,[%04x]",xx);
		break;
	case 0xad:	//LDA abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"mov\ta,[%04x]",xx);
		break;
	case 0xae:	//LDX abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"mov\tx,[%04x]",xx);
		break;

		//0xb0

	case 0xb0:	//BCS rel
		x = *p++;
		rad = (ad+2) + (signed char)x;
		sprintf(codes,"jc\t%04x",rad);			//jmp carry flag
		break;
	case 0xb1:	//LDA ind,Y
		x = *p++;
		sprintf(codes,"mov\ta,[[%02x]+y]",x);
		break;
	case 0xb4:	//LDY zpg,X
		x = *p++;
		sprintf(codes,"mov\ty,[x+%02x]",x);
		break;
	case 0xb5:	//LDA zpg,X
		x = *p++;
		sprintf(codes,"mov\ta,[x+%02x]",x);
		break;
	case 0xb6:	//LDX zpg,Y
		x = *p++;
		sprintf(codes,"mov\tx,[x+%02x]",x);
		break;
	case 0xb8:	//CLV impl
		sprintf(codes,"clv");					//clear overflow flag
		break;
	case 0xb9:	//LDA abs,Y
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"mov\ta,[y+%04x]",xx);
		break;
	case 0xba:	//TSX impl
		sprintf(codes,"mov\tx,sp");
		break;
	case 0xbc:	//LDY abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"mov\ty,[x+%04x]",xx);
		break;
	case 0xbd:	//LDA abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"mov\ta,[x+%04x]",xx);
		break;
	case 0xbe:	//LDX abs,Y
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"mov\tx,[y+%04x]",xx);
		break;

		//0xc0

	case 0xc0:	//CPY #
		x = *p++;
		sprintf(codes,"cmp\ty,%02x",x);
		break;
	case 0xc1:	//CMP X,ind
		x = *p++;
		sprintf(codes,"cmp\ta,[[x+%02x]]",x);
		break;
	case 0xc4:	//CPY zpg
		x = *p++;
		sprintf(codes,"cmp\ty,[%02x]",x);
		break;
	case 0xc5:	//CMP zpg
		x = *p++;
		sprintf(codes,"cmp\ta,[%02x]",x);
		break;
	case 0xc6:	//DEC zpg
		x = *p++;
		sprintf(codes,"dec\t[%02x]",x);
		break;
	case 0xc8:	//INY impl
		sprintf(codes,"inc\ty");
		break;
	case 0xc9:	//CMP #
		x = *p++;
		sprintf(codes,"cmp\ta,%02x",x);
		break;
	case 0xca:	//DEX impl
		sprintf(codes,"dec\tx");
		break;
	case 0xcc:	//CPY abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"cmp\ty,[%04x]",xx);
		break;
	case 0xcd:	//CMP abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"cmp\ta,[%04x]",xx);
		break;
	case 0xce:	//DEC abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"dec\t[%04x]",xx);
		break;

		//0xd0

	case 0xd0:	//BNE rel
		x = *p++;
		rad = (ad+2) + (signed char)x;
		sprintf(codes,"jnz\t%04x",rad);
		break;
	case 0xd1:	//CMP ind,Y
		x = *p++;
		sprintf(codes,"cmp\ta,[[%02x]+y]",x);
		break;
	case 0xd5:	//CMP zpg,X
		x = *p++;
		sprintf(codes,"mov\ta,[x+%02x]",x);
		break;
	case 0xd6:	//DEC zpg,X
		x = *p++;
		sprintf(codes,"dec\t[x+%02x]",x);
		break;
	case 0xd8:	//CLD impl
		sprintf(codes,"cld	(not implement)");		//bcd -> normal mode
		break;
	case 0xd9:	//CMP abs,Y
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"cmp\ta,[y+%04x]",xx);
		break;
	case 0xdd:	//CMP abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"cmp\ta,[x+%04x]",xx);
		break;
	case 0xde:	//DEC abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"dec\t[x+%04x]",xx);
		break;

		//0xe0

	case 0xe0:	//CPX #
		x = *p++;
		sprintf(codes,"cmp\tx,%02x",x);
		break;
	case 0xe1:	//SBC X,ind
		x = *p++;
		sprintf(codes,"sbb\tx,[[x+%02x]]",x);
		break;
	case 0xe4:	//CPX zpg
		x = *p++;
		sprintf(codes,"cmp\tx,[%02x]",x);
		break;
	case 0xe5:	//SBC zpg
		x = *p++;
		sprintf(codes,"sbb\ta,[%02x]",x);
		break;
	case 0xe6:	//INC zpg
		x = *p++;
		sprintf(codes,"inc\t[%02x]",x);
		break;
	case 0xe8:	//INX impl
		sprintf(codes,"inc\tx");
		break;
	case 0xe9:	//SBC #
		x = *p++;
		sprintf(codes,"sbb\ta,%02x",x);
		break;
	case 0xea:	//NOP impl
		sprintf(codes,"nop");
		break;
	case 0xec:	//CPX abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"cmp\tx,[%04x]",xx);
		break;
	case 0xed:	//SBC abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"sbb\ta,[%04x]",xx);
		break;
	case 0xee:	//INC abs
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"inc\t[%04x]",xx);
		break;

		//0xf0

	case 0xf0:	//BEQ rel
		x = *p++;
		rad = (ad+2) + (signed char)x;
		sprintf(codes,"jz\t%04x",rad);
		break;
	case 0xf1:	//SBC ind,Y
		x = *p++;
		sprintf(codes,"sbb\ta,[[%02x]+y]",x);
		break;
	case 0xf5:	//SBC zpg,X
		x = *p++;
		sprintf(codes,"sbb\ta,[x+%02x]",x);
		break;
	case 0xf6:	//INC zpg,X
		x = *p++;
		sprintf(codes,"inc\t[x+%02x]",x);
		break;
	case 0xf8:	//SED impl
		sprintf(codes,"sed	(not implement)");		//normal mode -> bcd mode
		break;
	case 0xf9:	//SBC abs,Y
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"sbb\ta,[y+%04x]",xx);
		break;
	case 0xfd:	//SBC abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"sbb\ta,[x+%04x]",xx);
		break;
	case 0xfe:	//INC abs,X
		xx = (*(p+1) << 8) | *p;	p += 2;
		sprintf(codes,"inc\t[x+%04x]",xx);
		break;

	}


	//今回の進行バイト数
	int n = (p-op);

	//アドレスを表示
	printf("%04x:",ad);
	//バイトを表示
	int lp;
	for (lp = 0;lp < n;lp++)
	{
		printf("%02x ",*op++);
	}
	for (;lp < 4;lp++)
	{
		printf("   ");
	}
	//ニーモニック表示
	printf("%s\n",codes);

	//次のアドレスへ
	ad += n;

	return 0;
}