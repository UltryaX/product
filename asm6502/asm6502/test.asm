		;zeropage�p�̃A�h���b�V���O����Ȃ�A�ŏ��ɒ�`���邱��
		;����ȊO�Ȃ�A�u�\�����[�g�ɂȂ�


			;
			;	�}�N����`
			;

setoffset	macro
			mov a,offset_h T2			;���
			mov [T1],a
			mov a,offset T2				;����
			mov [T1+1],a
			endm

setword		macro
			mov a,T2
			mov [T1],a
			mov a,T3
			mov [T1+1],a
			endm

setscreen	macro
			mov a,T1
			mov [2006h],a
			mov a,T2
			mov [2006h],a
			endm

			;
			;	�[���y�[�W��`�G���A
			;

			.org 0

l00			db	0
l01			db	0

l02			db	0	;�\�����b�Z�[�W�A�h���X
l03			db	0
l04			db	0	;�\����VRAM�A�h���X
l05			db	0

l06			db	0	;�\���\���̐擪�A�h���X
l07			db	0

l08			db	0,0,0,0, 0,0,0,0		;8-15
sqreg		db	0,0						;16-17

joypad		db	0,0						;18
vrc6prg		db	0,0
dumpmemad	db	0,0

work		db	0,0,0,0,0,0,0,0
vwait		db	0,0
drawreq		db	0,0

			;
			;	�J�n
			;

			.org 0e800h


startup:
			;���荞�݋֎~�A�X�^�b�N�|�C���^������
			cli
			mov x,0ffh
			mov sp,x

			;vblank��҂�
wait1:		mov a,[2002h]
			jns wait1
wait2:		mov a,[2002h]
			jns wait2

			;PPU�������i��ʃI�t�j
			mov a,0
			mov [2000h],a
			mov [2001h],a

			;rp2a03 apu������
			mov a,0
			mov [4015h],a

			;vrc6������
			mov a,20h
			mov [0b003h],a

			mov a,0
			mov [0d000h],a
			mov a,2
			mov [0d001h],a
			mov a,1
			mov [0d002h],a
			mov a,3
			mov [0d003h],a

			mov a,4
			mov [0e000h],a
			mov a,6
			mov [0e001h],a
			mov a,5
			mov [0e002h],a
			mov a,7
			mov [0e003h],a

			;memory������
			mov x,0
			mov a,x
memclr:
			mov [x+00h],a
			mov [x+100h],a
			mov [x+200h],a
			mov [x+300h],a
			mov [x+400h],a
			mov [x+500h],a
			mov [x+600h],a
			mov [x+700h],a
			mov [x+6000h],a
			mov [x+6100h],a
			mov [x+7e00h],a
			mov [x+7f00h],a
			inc x
			jnz memclr

			;vrc6 prgmap������
			mov a,0
			mov [8000h],a
			mov [0c000h],a
			mov [vrc6prg],a
			mov [vrc6prg+1],a

			;vrc6sound������
			mov a,0
			mov [9003h],a		;�����l�Ƃ���0������Ƃ��� https://wiki.nesdev.com/w/index.php/VRC6_audio

			mov [9000h],a		;���`�����l�����I�t�A�{�����[���I�t��
			mov [0a000h],a
			mov [9001h],a
			mov [0a001h],a
			mov [0b001h],a
			mov [9002h],a
			mov [0a002h],a
			mov [0b002h],a



			;�`��A�h���X��BG�p���b�g�ɐݒ�
			.setscreen 3fh,0

			;�p���b�g�ݒ�
			mov x,0
			mov y,10h
loop1:
			mov a,[x+palette]
			mov [2007h],a
			inc x
			dec y
			jnz loop1


			;BG�̃l�[���e�[�u�����N���A
			.setscreen 20h,0

			;960�񃋁[�v
			.setword l00,0c0h,3
			mov x,0
loop3:
			mov a,x
			mov [2007h],a
			
			mov a,[l00]
			jnz lx2
			mov a,[l01]
			jz loop3end
			dec [l01]

lx2:
			dec [l00]
			jmp loop3

	
loop3end:

			;�����e�[�u����������
			.setscreen 23h,0c0h
			
			mov x,40h
			mov a,0
loop4:		
			mov [2007h],a
			dec x
			jnz loop4



			;
			;	demo�f�[�^����
			;
			mov a,9fh
			mov [8],a
			mov a,0abh
			mov [9],a
			mov a,03fh
			mov [10],a
			mov a,0f2h
			mov [11],a

			;
			;	sq1�̃f�[�^��\������
			;	

			.setoffset	l06,sq1mes
			call blockprint

			.setoffset	l06,registermes
			call blockprint

			.setword sqreg,8,0
			call statusprint


			;
			;	���W�X�^�\��
			;

			.setoffset	l06,soundmes
			call blockprint


			;
			;	��ʕ\��
			;

screen_on:		
			;vblank wait��������
			mov a,0
			mov [vwait],a
			mov [vwait+1],a

			;�X�N���[���l��0�ɐݒ�
			mov a,[2002h]		;�X�N���[�����W�X�^�������ݏ���������
			mov a,0
			mov [2005h],a
			mov [2005h],a

			;��ʕ\���J�n
			mov a,88h
			mov [2000h],a
			mov a,1eh
			mov [2001h],a

			;�T�E���h�n�o�b�t�@������
			mov a,0
			mov [work+1],a
			mov [work+2],a
			mov [work+3],a
			mov [work+4],a
			mov [work+5],a
			mov [work+6],a
			mov [work+7],a
			

			;���荞�݋����������AVBLANK��NMI�����Ȃ̂Ŋ��荞�܂��
			;sti

			jmp $

			;�������[�v	VBLANK�҂�
;waitvblank:
;			mov a,[vwait+1]
;			jz waitvblank
;			call mainloop
;			mov a,0
;			mov [vwait+1],a
;			jmp waitvblank

			;
			;	���荞�݂ŏ������郁�C�����[�v
			;
callvblank:
			push a
			mov a,x
			push a
			mov a,y
			push a

			;�p�b�h���͂̊��x����������
			mov a,[vwait+1]
			jz m0
			dec [vwait+1]
			jmp m1

m0:
			;�p�b�h����
			mov a,1
			mov [4016h],a
			mov a,0
			mov [4016h],a

			mov x,8
getpad:
			mov a,[4016h]
			shr a,1
			rcl [joypad+1],1
			dec x
			jnz getpad

			mov a,[joypad+1]
			or a,[joypad]
			mov [joypad],a

			mov a,8
			mov [vwait+1],a


m1:
			;
			;	15�t���[����1�񂵂��������Ȃ�
			;
			mov a,[vwait]
			jz m2
			dec [vwait]

			;
			;	��ʕ`�惊�N�G�X�g���������番�����ď������Ă���
			;

			mov a,[drawreq]
			jz m1c

			cmp a,1
			jnz m1a

			.setword sqreg,8,0
			call statusprint
			inc [drawreq]
			jmp m1b

	m1a:
			.setoffset	l06,soundmes
			call blockprint

			mov a,0
			mov [drawreq],a


			;
			;	�`�悵�����ʈʒu��␳����
			;
	m1b:
			mov a,0
			mov [2005h],a
			mov [2005h],a

	m1c:
			pop a
			mov y,a
			pop a
			mov x,a
			pop a
			iret

m2:			mov a,15
			mov [vwait],a


			;
			;
			;	�������C��
			;
			;

mainloop:
			rcr [joypad],1		;�E
			jnc p1
			inc [work+2]
			mov a,[work+2]
			and a,7
			mov [work+2],a
			jmp p9
	p1:		rcr [joypad],1		;��
			jnc p2
			dec [work+2]
			mov a,[work+2]
			and a,7
			mov [work+2],a
			jmp p9
	p2:		rcr [joypad],1		;��
			jnc p3
			inc [work+3]
			mov a,[work+3]
			and a,3
			mov [work+3],a
			jmp p9
	p3:		rcr [joypad],1		;��
			jnc p4
			dec [work+3]
			mov a,[work+3]
			and a,3
			mov [work+3],a
			jmp p9

	p4:		rcr [joypad],1		;start
			rcr [joypad],1		;select
			rcr [joypad],1		;B
			jnc p5

			mov a,80h
			mov [work+5],a
			mov a,[work+2]
			mov [work+6],a

			mov a,[work+6]
			jz $+6
			shr [work+5],1
			dec [work+6]
			jnz $-4

			;�ړI�̃r�b�g��ύX����
			mov a,[work+3]
			jnz p41					;register 0
			mov a,[work+5]
			xor a,[l08]
			mov [l08],a
			jmp p44
	p41:	cmp a,1
			jnz p42
			mov a,[work+5]
			xor a,[l08+1]
			mov [l08+1],a
			jmp p44
	p42:	cmp a,2
			jnz p43
			mov a,[work+5]
			xor a,[l08+2]
			mov [l08+2],a
			jmp p44
	p43:	mov a,[work+5]
			xor a,[l08+3]
			mov [l08+3],a

			;�ύX�����̂ŉ�ʂ̍X�V�v����������
	p44:	mov a,1
			mov [drawreq],a
			jmp p9


	p5:		rcr [joypad],1		;A
			jnc p9

			mov a,[l08]			;�T�E���h���W�X�^��ݒ�
			mov [4000h],a
			mov a,[l08+1]
			mov [4001h],a
			mov a,[l08+2]
			mov [4002h],a
			mov a,[l08+3]
			mov [4003h],a

			mov a,1				;������ݒ�
			mov [4015h],a

	p9:		
			;���ݑI������Ă��郌�W�X�^��_�ł�����
			call sounddisp
			;work	2	x	0-7bit
			;work	3	y	reg0-3
			;work	4	brink
			inc [work+4]
			mov a,[work+4]
			shr a,1
			jnc p10

			mov a,[work+3]
			mov x,a
			mov a,[x+selreg]
			clc
			adc a,[work+2]
			push a
			mov a,22h
			mov [2006h],a
			pop a
			mov [2006h],a

			mov a,32
			mov [2007h],a

	p10:
			;
			;	�X�N���[���ʒu�͖��񏑂��Ă���
			;
			mov a,0
			mov [2005h],a
			mov [2005h],a

			;
			;	�p�b�h���͂̓N���A���Ă���
			;
			;mov a,0
			mov [joypad],a

			pop a
			mov y,a
			pop a
			mov x,a
			pop a
			iret

			;
			;	�T�E���h�p��ʕ\��
			;
sounddisp:
			mov x,0
			.setscreen 22h,27h
			call sds
			inc x
			.setscreen 22h,67h
			call sds
			inc x
			.setscreen 22h,0a7h
			call sds
			inc x
			.setscreen 22h,0e7h

	sds:	mov a,[x+l08]
			mov [work+1],a
			mov y,8
	sds1:
			shl [work+1],1
			jnc sds2
			mov a,31h
			jmp sds3
	sds2:	mov a,30h
	sds3:	mov [2007h],a
			dec y
			jnz sds1
			ret





			;
			;
			;	�}�b�s���O�����p�������_���v
			;
			;

dumpmem:	
			;
			;	dump mem 8000h�`
			;
			
			.setword l04,23h,00h
			.setoffset  l02,mesa0
			call printmessage
			mov a,[vrc6prg]
			call hexprint2

			.setscreen 23h,20h
			mov a,4							;�ǂ���vrc6�́Avrc6���W�X�^��Read�ł��A�N�Z�X����Ƃ��낢�남�����Ȃ��Ƃ��N����炵��
			mov [dumpmemad],a
			mov a,80h
			mov [dumpmemad+1],a
			call dumpmem2

			;
			;	dump mem c000h�`
			;
			mov a,40h
			mov [l05],a
			.setoffset  l02,mesa1
			call printmessage
			mov a,[vrc6prg+1]
			call hexprint2

			.setscreen 23h,60h
			mov a,0c0h
			mov [dumpmemad+1],a

dumpmem2:
			mov a,12
			mov [work],a
			mov y,0
du6:
			mov a,[[dumpmemad]+y]
			push a

			shr a,1
			shr a,1
			shr a,1
			shr a,1
			and a,15
			mov x,a
			mov a,[x+mes02]
			mov [2007h],a

			pop a
			and a,15
			mov x,a
			mov a,[x+mes02]
			mov [2007h],a

			inc y
			dec [work]
			jnz du6
			ret



			;
			;	���b�Z�[�W�\���\���̂̓��e��\������
			;

blockprint:
			mov y,0
			mov a,[[l06]+y]		;���b�Z�[�W���擾
			inc y
sq1s1:
			push a

			mov a,[[l06]+y]		;���b�Z�[�W������A�h���X�̎擾
			mov [l02],a
			inc y
			mov a,[[l06]+y]
			mov [l03],a
			inc y
			mov a,[[l06]+y]		;��ʕ\���A�h���X�̎擾	2006h�ɓ����A�h���X�͋t�ɃX�g�A����
			mov [l05],a
			inc y
			mov a,[[l06]+y]
			mov [l04],a
			inc y

			mov a,y
			push a
			call printmessage
			pop a
			mov y,a

			pop a
			stc
			sbb a,1
			jnz sq1s1

			return

			;
			;���݂̐ݒ�l�����ƂɁA�e�ݒ�̏�Ԃ���������
			;

statusprint:
			;�{�����[��
			.setword l04,20h,0aah
			mov y,0
			mov a,[[sqreg]+y]
			and a,15
			call hexprint

			;�G���x���[�v
			mov a,0cah
			mov [l05],a
			mov y,0
			mov a,[[sqreg]+y]
			and a,10h
			call edprint

			;�L�[�I�t�܂��̓G���x���[�v���[�v
			mov a,0eah
			mov [l05],a
			mov y,0
			mov a,[[sqreg]+y]
			and a,20h
			call edprint

			;�f���[�e�B��
			.setword l04,21h,0ah
			.setoffset	l06,sqduty
			mov y,0
			mov a,[[sqreg]+y]
			rcl a,1
			rcl a,1
			rcl a,1
			and a,3
			call tableprint

			;�X�C�[�v�L���t���O
			mov a,2ah
			mov [l05],a
			mov y,1
			mov a,[[sqreg]+y]
			rcl a,1
			rcl a,1
			and a,1
			call edprint

			;�X�C�[�v��(shift)
			mov a,4ah
			mov [l05],a
			mov y,1
			mov a,[[sqreg]+y]
			and a,7
			call hexprint

			;�X�C�[�v�ω����x(period)
			mov a,6ah
			mov [l05],a
			mov y,1
			mov a,[[sqreg]+y]
			shr a,1
			shr a,1
			shr a,1
			shr a,1
			and a,7
			call hexprint

			;�X�C�[�v����(negate)
			mov a,8ah
			mov [l05],a
			.setoffset l06,swdir
			mov y,1
			mov a,[[sqreg]+y]
			shr a,1
			shr a,1
			shr a,1
			and a,1
			call tableprint

			;���g��
			mov a,0aah
			mov [l05],a
			mov y,3
			mov a,[[sqreg]+y]
			and a,7
			call hexprint
			mov y,2
			mov a,[[sqreg]+y]
			call hexprint2

			;�L�[�I�t�J�E���g�l
			mov a,0cah
			mov [l05],a
			mov y,3
			mov a,[[sqreg]+y]
			shr a,1
			shr a,1
			shr a,1
			call hexprint



			;
			;	register�\��
			;

			.setword l04,20h,0dah
			mov y,0
			mov a,[[sqreg]+y]
			call hexprint

			.setword l04,20h,0fah
			mov y,1
			mov a,[[sqreg]+y]
			call hexprint

			.setword l04,21h,1ah
			mov y,2
			mov a,[[sqreg]+y]
			call hexprint

			.setword l04,21h,3ah
			mov y,3
			mov a,[[sqreg]+y]
			call hexprint

			ret



			;
			;	���b�Z�[�W�\��
			;
printmessage:
			.setscreen [l04],[l05]

			mov y,0
prm1:
			mov a,[[l02]+y]
			cmp a,024h
			jz prm2

			mov [2007h],a
			inc y
			jmp prm1

prm2:
			ret



			;
			;	�E�F�C�g
			;
;screen_wait:
;			mov a,16			;NES�ɂ͂��̋@�\�͂Ȃ��Ƃ����\�@�܂菈���͕K�v�Ȃ�
;;wait3a:	test a,[2002h]
;;			jnz wait3a
;wait4:		mov a,[2002h]		;�ǂ߂�VBLANK�t���O�̓N���A�����
;			jns wait4
;			ret


hexprint:
			push a
			.setscreen [l04],[l05]
			pop a

hexprint2:
			push a
			shr a,1
			shr a,1
			shr a,1
			shr a,1
			mov x,a
			mov a,[x+mes02]
			mov [2007h],a
			pop a
			and a,15
			mov x,a
			mov a,[x+mes02]
			mov [2007h],a
			ret

edprint:
			jnz edp2
			.setoffset l02,mes01
			jmp printmessage
	edp2:	.setoffset l02,mes00
			jmp printmessage

tableprint:
			shl a,1
			mov y,a

			mov a,[[6]+y]
			mov [l02],a
			inc y
			mov a,[[6]+y]
			mov [l03],a
			jmp printmessage



			;
			;	�f�[�^�G���A
			;		0xf000-03�ӂ�ɂ�IRQ�֘A��I/O���}�b�s���O����Ă���͗l

			.org 0f010h

palette		db 0fh,00h,10h,20h
			db 0fh,06h,16h,26h
			db 0fh,08h,18h,28h
			db 0fh,0ah,1ah,2ah


sq1mes		db	12
			dw	message,2040h
			dw	mes20,2081h
			dw	mes21,20a2h
			dw	mes22,20c2h
			dw	mes23,20e2h
			dw	mes24,2102h
			dw	mes25,2122h
			dw	mes26,2142h
			dw	mes27,2162h
			dw	mes28,2182h
			dw	mes29,21a2h
			dw	mes2a,21c2h

sqduty		dw	mes30
			dw	mes31
			dw	mes32
			dw	mes33

swdir		dw	mes34
			dw	mes35


registermes	db	5
			dw	mes90,20b4h
			dw	mes91,20d4h
			dw	mes92,20f4h
			dw	mes93,2114h
			dw	mes94,2134h

soundmes	db	4
			dw	mess00,2200h
			dw	mess01,2240h
			dw	mess02,2280h
			dw	mess03,22c0h



			;.align 16


message		ds	'RP2A03 Sound Tester ver 0.1$'

mes20		ds	'Square1$'
mes21		ds	'Volume $'
mes22		ds	'Envelop$'
mes23		ds	'KeyOff $'
mes24		ds	'Duty   $'

mes25		ds	'Sweep  $'
mes26		ds	'SwShift$'
mes27		ds	'SPeriod$'
mes28		ds	'SNagate$'

mes29		ds	'Freq   $'
mes2a		ds	'KOffCt $'

mes00		ds	'enable $'
mes01		ds	'disable$'

mes02		ds	'0123456789ABCDEF'			;���ɂ����̂ő啶���ɕύX

mes30		ds	'12.5%  $'
mes31		ds	'25%    $'
mes32		ds	'50%    $'
mes33		ds	'75%    $'

mes34		ds	'up     $'
mes35		ds	'down   $'



mes90		ds	'Register$'
mes91		ds	'4000h$'
mes92		ds	'4001h$'
mes93		ds	'4002h$'
mes94		ds	'4003h$'


mess00		ds	'4000h  DDLERRRR$'
mess01		ds	'4001h  FSSSHRRR$'
mess02		ds	'4002h  FFFFFFFF$'
mess03		ds	'4003h  CCCCCFFF$'
selreg		db	27h,67h,0a7h,0e7h			


mesa0		ds	'8000h  $'
mesa1		ds	'c000h  $'


			;
			;reset vector
			;

			.org 0fffah

			dw	callvblank			;vblank���荞��(NMI����)
			dw	startup				;���Z�b�g�J�n�A�h���X
			dw	startup				;IRQ,NMI���荞��
