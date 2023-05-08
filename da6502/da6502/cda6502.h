#pragma once
class cda6502
{
public:
	cda6502(void);
	~cda6502(void);


public:
	unsigned short ad,rad;
	unsigned char *p;
	unsigned char x;
	unsigned short xx;

public:
	int da();



};

