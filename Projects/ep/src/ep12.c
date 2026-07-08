/****************************************

TAVOLA DEGLI STATI DEL PROGRAMMATORE DI EPROM

input  E*(pin20)   Vcc(pin28)  Vpp(pin22)     mode  ack shift

  0  |	  0		5	0		0    0    1
  1  |    0    		5    	0    		0    0    0
  2  |	  5    		5    	0    		0    1    0
  3  |    5    		5    	0    		0    0    0
  4  |    0    		5    	12   		0    0    1
  5  |    0    		5    	12   		0    0    0
  6  |    5    		5    	0    		1    1    0
  7  |    5    		5    	0    		1    0    0
  8  |    0    		5.5  	12   		0    0    1
  9  |    0    		5.5  	12   		0    0    0
  A  |    0    		5.5  	12   		0    0    1
  B  |    0    		5.5  	12   		0    0    0
  C  |    5    		5.5  	12   		0    1    0
  D  |    5    		5.5  	12   		0    0    0
  E  |    5    		5.5  	12   		1    1    0
  F  |    5    		5.5  	12   		1    0    0

****************************************/

#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <mem.h>

#define BACKGROUND	BLUE
#define TEXT		LIGHTCYAN

#define DATA_OUT	0x378
#define DATA_IN		0x379
#define CONTROL		0x37A

#define read_lsb()		outp(CONTROL,0x1)
#define read_msb()		outp(CONTROL,0x0)
#define read_3()		outp(CONTROL,0x3)
#define read_4()		outp(CONTROL,0x2)
#define read_lready()		outp(CONTROL,0x7)
#define read_lset()		outp(CONTROL,0x6)
#define pro_b()			outp(CONTROL,0x8)	//0xa
#define pro_a()			outp(CONTROL,0x9)	//0xb
#define pro()			outp(CONTROL,0xa)
#define pro_inhibit() 		outp(CONTROL,0xc)	//0xd (con ack basso)
#define pro_init()		outp(CONTROL,0xd)
#define pro_lset()		outp(CONTROL,0xe)
#define pro_lready()		outp(CONTROL,0xf)

#define size_byte	((unsigned long)size/8*1024)

#define read8253ck() (~(inp(0x40) | (inp(0x40)<<8)))
#define resetck() {outp(0x43,0x34);outp(0x40,0x0);outp(0x40,0x0);}
#define readbiosck(i) movedata( 0x40, 0x6c, _SS, (unsigned)&i, 4);
#define sec_per_ck 8381/1E10
#define ck_per_sec 1193000.9

#define del(t) delopt(t-del0)

#define buf_size 10000

typedef struct
	{
	int size;
	float tw;
	} params;

unsigned int base;

float del0, middle, err;

int size;
float timewrite;

params parameters;

/****************************************************************************/

void printbin(int p)
{
printf("%d%d%d%d%d%d%d%d",(p&128)>>7,(p&64)>>6,(p&32)>>5,(p&16)>>4,(p&8)>>3,(p&4)>>2,(p&2)>>1,p&1);
}

/****************************************************************************/

error(s)
	char *s;
{
	puts(s);
//	exit(1);
}

/****************************************************************************/

unsigned long readck()
{
short ck,bios;
unsigned long out;

asm cli
readbiosck(bios);
ck=read8253ck();
asm sti

out = (long)(bios)<<16 | ck&0xFFFF;
return out;
}

/****************************************************************************/

delopt(float t)
{
long d,start,end;

//d = t / sec_per_ck;
d = t * ck_per_sec;

start = readck();
end = start + d;
while (readck()<end);
}

/****************************************************************************/

void testadr()
{
long i;

for (i=0;i<65536;i++)
	{
	read_lsb();

	read_lready();
	outp(DATA_OUT,i&0xFF);
	read_lset();

	read_lready();
	outp(DATA_OUT,i>>8);
	read_lset();

//	printf("\n",inp(CONTROL));printbin(i>>8);printf(" ",inp(CONTROL));printbin(i&0xff);
	}
}

/****************************************************************************/

void testwrite()
{
long i;

pro_a();
for (i=0;i<256;i++)
	{
	outp(DATA_OUT,i);

//	delay(1);
//	printf("\n",inp(CONTROL));printbin(i);
	}
}

/****************************************************************************/

testread()
{
long i;
char lsb,msb,x;

read_lsb();
for (i=0;i<256;i++)
	{
	pro_a();
	outp(DATA_OUT,i);

	lsb=(inp(DATA_IN)>>4)^0x8;
	pro_b();
	msb=(inp(DATA_IN)&0xF0)^0x80;

	x=lsb|msb;

//	clrscr();printf("\nx   ");printbin(x);delay(10);

	if (x!=(char)i) return 1;
	}
return 0;
}

/****************************************************************************/

unsigned char read_adr(unsigned int adr)
{
int lsb,msb;

read_lsb();

read_lready();
outp(DATA_OUT,adr&0xFF);
read_lset();

read_lready();
outp(DATA_OUT,adr>>8);
read_lset();

read_lsb();

read_lsb();
lsb=(inp(DATA_IN)>>4)^0x8;
read_msb();
msb=(inp(DATA_IN)&0xF0)^0x80;

return (lsb|msb);
}

/****************************************************************************/

write_adr(unsigned int adr, unsigned char c, float tw)
{
pro_init();

pro_lready();
outp(DATA_OUT,adr&0xFF);
pro_lset();

pro_lready();
outp(DATA_OUT,adr>>8);
pro_lset();
pro_lset();

outp(DATA_OUT,c);
pro_lset();

del(0.15);
pro();
del(tw);

pro_inhibit();
}

/****************************************************************************/

unsigned char mask(char c)
{
switch (c)
	{
	case 0 :;
	case 7 :;
	case 8 :;
	case 9 :;
	case 10:;
	case 13:;
	case 26:;
	return 46;
	}
return c;
}

/****************************************************************************/

void restore()
{
outp(CONTROL,236);
window(1, 1, 80, 25);
textbackground(0);
textcolor(7);
clrscr();
}

/****************************************************************************/

menu()
{
textbackground(BLACK);textcolor(LIGHTRED);cprintf(" ESC");
textbackground(LIGHTCYAN);textcolor(BACKGROUND);cprintf("  Exit ");

textbackground(BLACK);textcolor(LIGHTRED);cprintf(" F2");
textbackground(LIGHTCYAN);textcolor(BACKGROUND);cprintf("  Save ");

textbackground(BLACK);textcolor(LIGHTRED);cprintf(" F3");
textbackground(LIGHTCYAN);textcolor(BACKGROUND);cprintf(" Erase ");

textbackground(BLACK);textcolor(LIGHTRED);cprintf(" F4");
textbackground(LIGHTCYAN);textcolor(BACKGROUND);cprintf(" ex/txt");

textbackground(BLACK);textcolor(LIGHTRED);cprintf(" F5");
textbackground(LIGHTCYAN);textcolor(BACKGROUND);cprintf(" Pro  ");

textbackground(BLACK);textcolor(LIGHTRED);cprintf(" F6");
textbackground(LIGHTCYAN);textcolor(BACKGROUND);cprintf(" Test  ");

textbackground(BLACK);textcolor(LIGHTRED);cprintf("   ");
textbackground(LIGHTCYAN);textcolor(BACKGROUND);cprintf("       ");

textbackground(BLACK);textcolor(LIGHTRED);cprintf("   ");
textbackground(LIGHTCYAN);textcolor(BACKGROUND);cprintf("     ");

textbackground(BACKGROUND);textcolor(TEXT);

}

/****************************************************************************/

stimaerroretw()
{
long p, start, end, s, e;
float d=timewrite, maxt, mint, elapsed, middle;;



#define cost 0.05

mint=maxt=d;//secondi
middle=d;

for(p=1;p<(int)(cost/d);p++)
	{
	start = readck();
	del(d);
	end = readck();

	elapsed=(end-start)*sec_per_ck;

	if (elapsed<mint) mint=elapsed;
	if (elapsed>maxt) maxt=elapsed;

	middle=(middle*p+elapsed)/(p+1);
	if ((maxt-d)>(d-mint)) return 100.*(maxt-d)/d;
	else return 100.*(d-mint)/d;
	}
}

/****************************************************************************/

topbar()
{
/*
cprintf("Vista del contenuto dalla EPROM 27%.3u  %.5lu byte                               ",size,(long)size/8*1024);
*/
cprintf("Vista del contenuto dalla EPROM 27%.3u %.5lu byte tw=%.6f t0=%.6f e=%3u% ",size,(long)size/8*1024, timewrite, del0,stimaerroretw());//(int) err);
}

/****************************************************************************/

void vista_EX(int start_adr)
{
int i,x,y;

clrscr();
textbackground(11);textcolor(15);
topbar();
textbackground(BACKGROUND);textcolor(TEXT);

i=start_adr;
for(y=0;y<23;y++)
	{
	cprintf("%.4X  ",i%size_byte);
	for(x=0;x<4;x++) cprintf("%.2X ",read_adr(base+i++));
	cprintf("%c ",179);
	for(x=0;x<4;x++) cprintf("%.2X ",read_adr(base+i++));
	cprintf("%c ",179);
	for(x=0;x<4;x++) cprintf("%.2X ",read_adr(base+i++));
	cprintf("%c ",179);
	for(x=0;x<4;x++) cprintf("%.2X ",read_adr(base+i++));
	cprintf("  ");
	i-=16;
	for(x=0;x<16;x++) cprintf("%.1c",mask(read_adr(base+i++)));
	cprintf("  \r");
	}

menu();
}

/****************************************************************************/

void vista_TXT(unsigned int start_adr)
{
unsigned long i;

clrscr();
textbackground(11);textcolor(15);
topbar();
textbackground(BACKGROUND);textcolor(TEXT);

i=start_adr;
for(i=start_adr;i<80*23+(unsigned long)start_adr;i++)
	putch(mask(read_adr(i+base)));

menu();
}

/****************************************************************************/

save()
{
char name[60],buf1[buf_size],buf2[buf_size];
FILE *fl;
int i;
unsigned long blk_size, j;

textbackground(11);textcolor(15);
cprintf("\rSalvo sul file ? ");
cscanf("%s",name);
if (getch()==0) getch();


fl=fopen(name,"wb");
if ( fl==NULL )
	{
	printf("\nError opening file");
	return -1;
	}

if (buf_size>size_byte) blk_size=size_byte;
for(j=0;(j*buf_size)<size_byte;j+=1)
	{
	blk_size=( ((j+1)*buf_size)>size_byte?size_byte%buf_size:buf_size);

	for(i=0;i<blk_size;i++)
		{
		buf1[i]=read_adr(i+j*buf_size+base);
		}
	for(i=0;i<blk_size;i++)
		{
		buf2[i]=read_adr(i+j*buf_size+base);
		}
	for(i=0;i<blk_size;i++)
		if ( buf1[i]!=buf2[i] )
			cprintf("ERROR: floating value at %.4X, (%.2X, %.2X)\n\r", i, buf1[i]&0xFF, buf2[i]&0xFF);
//		else printf("GOOD ");

	if ( fwrite( buf1, blk_size, 1, fl ) != 1 )
		printf("\nError writing file");
	}


fclose(fl);
textbackground(BACKGROUND);textcolor(TEXT);
}

/****************************************************************************/

test()
{
char old;

old=inp(CONTROL);

cprintf("\r");
if 	(!testread()) cprintf("Il programmatore non presenta problemi.");
else 	cprintf("Il programmatore Š spento o scollegato.");

outp(CONTROL,old);
if (getch()==0) getch();
}

/****************************************************************************/

erase()
{
unsigned long a,sz=size_byte;

cprintf("\rVerifica Eprom vuota...");
for(a=0; (read_adr(a)==0xFF) && (a!=sz) ;)
	a++;
if (a==sz) cprintf(" La Eprom Š vuota.");
else cprintf(" La Eprom non Š vuota, %.4X",a);
if (getch()==0) getch();
}

/****************************************************************************/

pro_file()
{
char name[60],cw,cr;
FILE *fl;
unsigned int i;
int err;

textbackground(11);textcolor(15);
cprintf("\rNome del file da copiare sulla EPROM ? ");
cscanf("%s",name);
if (getch()==0) getch();

fl=fopen(name,"rb");
if ( fl==NULL )
	{
	printf("\rError opening file");
	if (getch()==0) getch();
	return -1;
	}

i=0+base;
while (!feof(fl))
	{
	cw=fgetc(fl);

	err=0;
	do
		{
		write_adr(i,cw, timewrite);
		read_lsb();
		cr=read_adr(0);
		cr=read_adr(i);
		if (cw==cr) break;
		else err++;
		}
	while(err<10);
	if (err==10)
		{
		cprintf("\rLa EPROM ‚ danneggiata, scrittura fallita all' indirizzo %.4X", i);
		if (getch()==0) getch();
		}
	i++;
	}

fseek(fl,SEEK_SET,0);
i=0;
while (!feof(fl))
	{
	cr=fgetc(fl);
	cw=read_adr(i);

	if (cr!=cw)
		{
		cprintf("\rLa EPROM ‚ danneggiata, scrittura fallita.");
		if (getch()==0) getch();
		break;
		}
	}

fclose(fl);
textbackground(BACKGROUND);textcolor(TEXT);
}

/****************************************************************************/

float trovadel0()
{
int i;
long st,end;
float elapsed;

st = readck();
del(0.);
end = readck();
middle=(end-st)*sec_per_ck;

for(i=1;i<300;i++)
	{
	st = readck();
	del(0.);
	end = readck();
	elapsed=(end-st)*sec_per_ck;
	middle=(middle*i+elapsed)/(i+1);
	}
return middle;
}

/****************************************************************************/

float testtw()
{
char c, cr, cw, crold;
int i, adr;
float t;

adr=0x00D4;
cw='C';
t=0.0001;

crold=read_adr(base+adr);

while(1)
	{
	write_adr(base+adr,cw, t);
	read_lsb();
	cr=read_adr(0xFFFF);
	del(2.);
	cr=read_adr(base+adr);
	if (crold!=cr)
		break;
	else
		{
		printf("Fail tw = %f\n\r", t);
		t+=0.00005;

		}
	if (kbhit())
		{
		c=getch();
		return 0;
		}
	}

cprintf("Tw found: %f\n\r", t);
return t;
}

/****************************************************************************/

gettag(char* s)
{
char c;
char t[80];

switch (s[0])
	{
	case '-':break;
	case '/':break;
	default: return 0;
	}
switch (s[1])
	{
	case 's':return 's';break;
	case 't':return 't';break;
	default: return 0;
	}
}

/****************************************************************************/

settag(char c, char*s)
{
switch (c)
	{
	case 's':parameters.size=atoi(s);break;
	case 't':parameters.tw=atof(s);break;
	default:;
	}
}

/****************************************************************************/

parse(int argv,char **argc)
{
char tag;
int i;

for(i=1;i<argv;i++)
	{
	if ((tag=gettag(argc[i]))==0) break;
	i++;
	settag(tag, argc[i]);
	}
}

/****************************************************************************/

void main(int argv,char **argc)
{
char c,vista=0;
int i,lsb,msb,x,y,start=0x0000;

parameters.size=512;	//Kilobit
parameters.tw=0.0001;	//seconds

parse(argv, argc);

size=parameters.size;
timewrite=parameters.tw;

del0=trovadel0();

err=stimaerroretw();

directvideo=1;

if (!vista) vista_EX(start);
else vista_TXT(start);

switch (size)
	{
	case  32: base=0xE000; break;
	case  64: base=0xC000; break;
	case 128: base=0xC000; break;
	case 256: base=0x8000; break;
	default : base=0x0000;
	}

//testtw();getch();

while(1)
{

if (kbhit())
	{
	if ( c=getch() == 0) c=getch();
	switch (c)
		{
		case 0:	restore();
			exit(0);

		case 72:start=(start-(vista?80:16)) % size_byte;break;
		case 80:start=(start+(vista?80:16)) % size_byte;break;

		case 73:start=(start-(vista?80*23:16*23)) % size_byte;break;
		case 81:start=(start+(vista?80*23:16*23)) % size_byte;break;

		case 71:start=0;break;
		case 79:start=(vista?size_byte-80*23:size_byte-16*23);break;

		case 60:save();break;
		case 61:erase();break;
		case 62:vista=~vista;break;
		case 63:pro_file();break;
		case 64:test();break;
		}

if (!vista) vista_EX(start);
else vista_TXT(start);
	}
}
}

/****************************************

NOTE:
	in programmazione l'algoritmo di verifica non Š efficace
	perchŠ il programmatore (senza eprom ionserita) ha un
	tempo di latenza di + di 1 sec.

	err=0;
	do
		{
		write_adr(i,cw, timewrite);
		read_lsb();
		cr=read_adr(0);
		del(5.);
		cr=read_adr(i);
		if (cw==cr) break;
		else err++;
		}
	while(err<10);
	if (err==10)

****************************************/
