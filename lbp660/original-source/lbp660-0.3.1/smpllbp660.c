#include <stdio.h>
#include <unistd.h> /* needed for ioperm() */
#include <asm/io.h> /* for outb() and inb() */
#include <sys/time.h>

int page[] = {
-1, 0x89, /*100*/
-1, 0x8a, /*172*/
-1, 0x8b, /*244*/
-3, 0x8b, 0x89, 0x8c, /*333*/
-7, 0x8c, 0x4, 0x94, 0x3f, 0x95, 0x58, 0x94, /*456*/
-1, 0x95, /*528*/
-1, 0x89, /*600*/
-1, 0x8a, /*672*/
-1, 0x8b, /*744*/
-1, 0x89, /*816*/
-1, 0x8a, /*888*/
-1, 0x8b, /*960*/
-3, 0x8b, 0x89, 0x90, /*1049*/
-5, 0x91, 0x0, 0x90, 0x0, 0x89, /*1155*/
-1, 0x8a, /*1227*/
-1, 0x8b, /*1299*/
-1, 0x89, /*1372*/
-1, 0x8a, /*1444*/
-3, 0x8d, 0xa7, 0x8b, /*1533*/
-3, 0x8b, 0x8a, 0x90, /*1622*/
-3, 0x90, 0x8, 0x89, /*1712*/
-1, 0x8a, /*1784*/
-1, 0x8e, /*1856*/
-1, 0x90, /*1928*/
-3, 0x90, 0x9, 0x89, /*2018*/
-1, 0x8a, /*2090*/
-3, 0x8d, 0x40, 0x90, /*2179*/
-5, 0x90, 0xc9, 0xa0, 0x0, 0xa0, /*2285*/
-17, 0x81, 0xdc, 0x82, 0x0, 0x83, 0x61, 0x84, 0x0, 0x85, 0x58, 0x86, 0x2, 0x87, 0x9c, 0x88, 0x1a, 0x81, /*2493*/
-1, 0x82, /*2565*/
-1, 0x83, /*2637*/
-1, 0x84, /*2709*/
-1, 0x85, /*2781*/
-1, 0x86, /*2853*/
-1, 0x87, /*2925*/
-1, 0x88, /*2997*/
-1, 0x89, /*3070*/
-1, 0x8a, /*3142*/
-1, 0x8e, /*3214*/
-1, 0x89, /*3287*/
-1, 0x8a, /*3359*/
-3, 0x8d, 0x2, 0x89, /*3449*/
-1, 0x8a, /*3521*/
-1, 0x8e, /*3593*/
-1, 0x89, /*3666*/
-1, 0x8a, /*3738*/
-3, 0x8d, 0x9d, 0x89, /*3828*/
-1, 0x8a, /*3900*/
-1, 0x8e, /*3972*/
-1, 0x89, /*4045*/
-1, 0x8a, /*4117*/
-3, 0x8d, 0x2, 0x89, /*4207*/
-1, 0x8a, /*4279*/
-1, 0x8e, /*4351*/
-3, 0x93, 0x0, 0x89, /*4441*/
-1, 0x8a, /*4513*/
-3, 0x8d, 0x2, 0x89, /*4603*/
-1, 0x8a, /*4675*/
-1, 0x8e, /*4747*/
-1, 0x89, /*4820*/
-1, 0x8a, /*4892*/
-1, 0x89, /*4965*/
-1, 0x8a,
-257,
-1, 0x8a, /*7852*/
-1, 0x8e, /*7924*/
-1, 0x89, /*7997*/
-1, 0x8a, /*8069*/
-1, 0x89, /*8142*/
-1, 0x8a, /*8214*/
-3, 0x8d, 0x1, 0x89, /*8304*/
-1, 0x8a, /*8376*/
-1, 0x89, /*8449*/
-1, 0x8a, /*8521*/
//-3, 0x80, 0xff, 0x7f, /*8604*/
-256, /* data */
-1, 0x8a, /*9655*/
-1, 0x8e, /*9727*/
-1, 0x89, /*9800*/
-1, 0x8a, /*9872*/
-3, 0x8d, 0x2, 0x89, /*9962*/
-1, 0x8a, /*10034*/
-1, 0x89, /*10107*/
-1, 0x8a, /*10179*/
-1, 0x89, /*10252*/
-1, 0x8a, /*10324*/
//-3, 0x80, 0xff, 0x7f, /*10407*/
-256, /* data */
-1, 0x8a, /*11458*/
-1, 0x8e, /*11530*/
-1, 0x89, /*11603*/
-1, 0x8a, /*11675*/
-1, 0x89, /*11748*/
-1, 0x8a, /*11820*/
-3, 0x8d, 0x1, 0x89, /*11910*/
-1, 0x8a, /*11982*/
-1, 0x89, /*12055*/
-1, 0x8a, /*12127*/
//-3, 0x80, 0xff, 0x7f, /*12210*/
-256, /* data */
-1, 0x8a, /*13261*/
-1, 0x8e, /*13333*/
-1, 0x89, /*13406*/
-1, 0x8a, /*13478*/
-3, 0x8d, 0x2, 0x89, /*13568*/
-1, 0x8a, /*13640*/
-1, 0x89, /*13713*/
-1, 0x8a, /*13785*/
//-3, 0x80, 0xff, 0x7f, /*13868*/
-256, /* data */
-1, 0x8a, /*14919*/
-1, 0x8e, /*14991*/
-1, 0x89, /*15064*/
-1, 0x8a, /*15136*/
-1, 0x89, /*15209*/
-1, 0x8a, /*15281*/
-3, 0x8d, 0x1, 0x89, /*15371*/
-1, 0x8a, /*15443*/
//-3, 0x80, 0xff, 0x7f, /*15526*/
-256, /* data */
-1, 0x8a, /*16577*/
-1, 0x8e, /*16649*/
-1, 0x89, /*16722*/
-1, 0x8a, /*16794*/
-3, 0x8d, 0x2, 0x89, /*16884*/
-1, 0x8a, /*16956*/
-1, 0x89, /*17029*/
-1, 0x8a, /*17101*/
-1, 0x89, /*17174*/
-1, 0x8a, /*17246*/
//-3, 0x80, 0xff, 0x7f, /*17329*/
-256, /* data */
-1, 0x8a, /*18380*/
-1, 0x8e, /*18452*/
-1, 0x89, /*18525*/
-1, 0x8a, /*18597*/
-1, 0x89, /*18670*/
-1, 0x8a, /*18742*/
-3, 0x8d, 0x1, 0x89, /*18832*/
-1, 0x8a, /*18904*/
//-3, 0x80, 0xff, 0x7f, /*18987*/
-256, /* data */
-1, 0x8a, /*20038*/
-1, 0x8e, /*20110*/
-1, 0x89, /*20183*/
-1, 0x8a, /*20255*/
-3, 0x8d, 0x2, 0x89, /*20345*/
-1, 0x8a, /*20417*/
-1, 0x89, /*20490*/
-1, 0x8a, /*20562*/
//-3, 0x80, 0xff, 0x7f, /*20645*/
-256, /* data */
-1, 0x8a, /*21696*/
-1, 0x8e, /*21768*/
-1, 0x89, /*21841*/
-1, 0x8a, /*21913*/
-1, 0x89, /*21986*/
-1, 0x8a, /*22058*/
-3, 0x8d, 0x1, 0x89, /*22148*/
-1, 0x8a, /*22220*/
-1, 0x89, /*22293*/
-1, 0x8a, /*22365*/
//-3, 0x80, 0xff, 0x7f, /*22448*/
-256, /* data */
-1, 0x8a, /*23499*/
-1, 0x8e, /*23571*/
-1, 0x89, /*23644*/
-1, 0x8a, /*23716*/
-3, 0x8d, 0x2, 0x89, /*23806*/
-1, 0x8a, /*23878*/
-1, 0x89, /*23951*/
-1, 0x8a, /*24023*/
//-3, 0x80, 0xff, 0x7f, /*24106*/
-256, /* data */
-1, 0x8a, /*25157*/
-1, 0x8e, /*25229*/
-1, 0x89, /*25302*/
-1, 0x8a, /*25374*/
-1, 0x89, /*25447*/
-1, 0x8a, /*25519*/
-3, 0x8d, 0x1, 0x89, /*25609*/
-1, 0x8a, /*25681*/
-1, 0x89, /*25754*/
-1, 0x8a, /*25826*/
//-3, 0x80, 0xff, 0x7f, /*25909*/
-256, /* data */
-1, 0x8a, /*26960*/
-1, 0x8e, /*27032*/
-1, 0x89, /*27105*/
-1, 0x8a, /*27177*/
-3, 0x8d, 0x2, 0x89, /*27267*/
-1, 0x8a, /*27339*/
-1, 0x89, /*27412*/
-1, 0x8a, /*27484*/
//-3, 0x80, 0xff, 0x7f, /*27567*/
-256, /* data */
-1, 0x8a, /*28618*/
-1, 0x8e, /*28690*/
-1, 0x89, /*28763*/
-1, 0x8a, /*28835*/
//-3, 0x80, 0xff, 0x7f, /*28918*/
-256, /* data */
-1, 0x8a, /*6189*/
-1, 0x8e, /*6261*/
-1, 0x89, /*6334*/
-1, 0x8a, /*6406*/
-3, 0x8d, -1, 0x89, /*6496*/
-1, 0x8a, /*6568*/
-1, 0x89, /*6641*/
-1, 0x8a,
-256,
-260 }; //, /*5037*/
//-5, 0x8d, 0x1, 0x80, 0xff, 0x7f, /*5138*/
//-257, /* data */

#if 0
int bandinit[] = {
-1, 0x8a, /*6189*/
-1, 0x8e, /*6261*/
-1, 0x89, /*6334*/
-1, 0x8a, /*6406*/
-3, 0x8d, -1, 0x89, /*6496*/
-1, 0x8a, /*6568*/
-1, 0x89, /*6641*/
-1, 0x8a,
-256 }; //, /*6713*/
#endif

//#if 0
int bandinit[] = {
-1, 0x8a, /*51996*/
-1, 0x8e, /*52068*/
-257,
-1, 0x8a, /*51996*/
-1, 0x8e, /*52068*/
-256,
-1, 0x8a, /*51996*/
-256,
-1, 0x8a, /*51996*/
-258,
-1, 0x8a, /*51996*/
-1, 0x8e, /*52068*/
-256,
-1, 0x8a, /*51996*/
-256,
-1, 0x8a, /*51996*/
-256,
-260 /* data */
};
//#endif

#if 0
////-3, 0x80, 0xff, 0x7f, /*6801*/
//-256, /* data */
int bandinit[] = {
-1, 0x8a, /*29969*/
//-5, 0x8d, 0x1, 0x80, 0xff, 0x7f, /*30069*/
-257, /* data */
-1, 0x8a, /*31121*/
//-3, 0x80, 0xff, 0x7f, /*31204*/
-256, /* data */
-1, 0x8a, /*32256*/
-1, 0x8e, /*32328*/
//-3, 0x80, 0xff, 0x7f, /*32411*/
-256, /* data */
-1, 0x8a, /*33463*/
//-5, 0x8d, 0x2, 0x80, 0xff, 0x7f, /*33563*/
-258, /* data */
-1, 0x8a, /*34615*/
//-3, 0x80, 0xff, 0x7f, /*34698*/
-256, /* data */
-1, 0x8a, /*35750*/
-1, 0x8e, /*35822*/
//-3, 0x80, 0xff, 0x7f, /*35905*/
-256, /* data */
-1, 0x8a, /*36957*/
//-3, 0x80, 0xff, 0x7f, /*37040*/
-256, /* data */
-1, 0x8a, /*38092*/
//-5, 0x8d, 0x1, 0x80, 0xff, 0x7f, /*38192*/
-257, /* data */
-1, 0x8a, /*39244*/
//-3, 0x80, 0xff, 0x7f, /*39327*/
-256, /* data */
-1, 0x8a, /*40379*/
-1, 0x8e, /*40451*/
//-3, 0x80, 0xff, 0x7f, /*40534*/
-256, /* data */
-1, 0x8a, /*41586*/
//-5, 0x8d, 0x2, 0x80, 0xff, 0x7f, /*41686*/
-258, /* data */
-1, 0x8a, /*42738*/
//-3, 0x80, 0xff, 0x7f, /*42821*/
-256, /* data */
-1, 0x8a, /*43873*/
-1, 0x8e, /*43945*/
//-3, 0x80, 0xff, 0x7f, /*44028*/
-256, /* data */
-1, 0x8a, /*45080*/
//-3, 0x80, 0xff, 0x7f, /*45163*/
-256, /* data */
-1, 0x8a, /*46215*/
//-5, 0x8d, 0x1, 0x80, 0xff, 0x7f, /*46315*/
-257, /* data */
-1, 0x8a, /*47367*/
//-3, 0x80, 0xff, 0x7f, /*47450*/
-256, /* data */
-1, 0x8a, /*48502*/
-1, 0x8e, /*48574*/
//-3, 0x80, 0xff, 0x7f, /*48657*/
-256, /* data */
-1, 0x8a, /*49709*/
//-5, 0x8d, 0x2, 0x80, 0xff, 0x7f, /*49809*/
-258, /* data */
-1, 0x8a, /*50861*/
//-3, 0x80, 0xff, 0x7f, /*50944*/
-256, /* data */
-1, 0x8a, /*51996*/
-1, 0x8e, /*52068*/
//-3, 0x80, 0xff, 0x7f, /*52151*/
-256, /* data */
-1, 0x8a, /*53203*/
//-3, 0x80, 0xff, 0x7f, /*53286*/
-256, /* data */
-1, 0x8a, /*54338*/
//-5, 0x8d, 0x1, 0x80, 0xff, 0x7f, /*54438*/
-257, /* data */
-1, 0x8a, /*55490*/
//-3, 0x80, 0xff, 0x7f, /*55573*/
-256, /* data */
-1, 0x8a, /*56625*/
-1, 0x8e, /*56697*/
//-3, 0x80, 0xff, 0x7f, /*56780*/
-256, /* data */
-1, 0x8a, /*57832*/
//-5, 0x8d, 0x2, 0x80, 0xff, 0x7f, /*57932*/
-258, /* data */
-1, 0x8a, /*58984*/
//-3, 0x80, 0xff, 0x7f, /*59067*/
-256, /* data */
-1, 0x8a, /*60119*/
-1, 0x8e, /*60191*/
//-3, 0x80, 0xff, 0x7f, /*60274*/
-256, /* data */
-1, 0x8a, /*61326*/
//-3, 0x80, 0xff, 0x7f, /*61409*/
-256, /* data */
-1, 0x8a, /*62461*/
//-5, 0x8d, 0x1, 0x80, 0xff, 0x7f, /*62561*/
-257, /* data */
-1, 0x8a, /*63613*/
//-3, 0x80, 0xff, 0x7f, /*63696*/
-256, /* data */
-1, 0x8a, /*64748*/
-1, 0x8e, /*64820*/
//-3, 0x80, 0xff, 0x7f, /*64903*/
-256, /* data */
-1, 0x8a, /*65955*/
//-5, 0x8d, 0x2, 0x80, 0xff, 0x7f, /*66055*/
-258, /* data */
-1, 0x8a, /*67107*/
//-3, 0x80, 0xff, 0x7f, /*67190*/
-256, /* data */
-1, 0x8a, /*68242*/
-1, 0x8e, /*68314*/
//-3, 0x80, 0xff, 0x7f, /*68397*/
-256, /* data */
-1, 0x8a, /*69449*/
//-3, 0x80, 0xff, 0x7f, /*69532*/
-256, /* data */
-1, 0x8a, /*70584*/
//-5, 0x8d, 0x1, 0x80, 0xff, 0x7f, /*70684*/
-257, /* data */
-1, 0x8a, /*71736*/
//-3, 0x80, 0xff, 0x7f, /*71819*/
-256, /* data */
-1, 0x8a, /*72871*/
-1, 0x8e, /*72943*/
//-3, 0x80, 0xff, 0x7f, /*73026*/
-256, /* data */
-1, 0x8a, /*74078*/
//-5, 0x8d, 0x2, 0x80, 0xff, 0x7f, /*74178*/
-258, /* data */
-1, 0x8a, /*75230*/
//-3, 0x80, 0xff, 0x7f, /*75313*/
-256, /* data */
-1, 0x8a, /*76365*/
-1, 0x8e, /*76437*/
//-3, 0x80, 0xff, 0x7f, /*76520*/
-256, /* data */
-1, 0x8a, /*77572*/
//-3, 0x80, 0xff, 0x7f, /*77655*/
-256, /* data */
-1, 0x8a, /*78707*/
//-5, 0x8d, 0x1, 0x80, 0xff, 0x7f, /*78807*/
-257, /* data */
-1, 0x8a, /*79859*/
//-3, 0x80, 0xff, 0x7f, /*79942*/
-256, /* data */
-1, 0x8a, /*80994*/
-1, 0x8e, /*81066*/
//-3, 0x80, 0xff, 0x7f, /*81149*/
-256, /* data */
-1, 0x8a, /*82201*/
//-5, 0x8d, 0x2, 0x80, 0xff, 0x7f, /*82301*/
-258, /* data */
-1, 0x8a, /*83353*/
//-3, 0x80, 0xff, 0x7f, /*83436*/
-256, /* data */
-1, 0x8a, /*84488*/
-1, 0x8e, /*84560*/
//-3, 0x80, 0xff, 0x7f, /*84643*/
-256, /* data */
-1, 0x8a, /*85695*/
//-3, 0x80, 0xff, 0x7f, /*85778*/
-256, /* data */
-1, 0x8a, /*86830*/
//-5, 0x8d, 0x1, 0x80, 0xff, 0x7f, /*86930*/
-257, /* data */
-1, 0x8a, /*87982*/
//-3, 0x80, 0xff, 0x7f, /*88065*/
-256, /* data */
-1, 0x8a, /*88633*/
-1, 0x8e, /*88705*/
-1, 0x89, /*88778*/
-1, 0x8a, /*88850*/
-3, 0x8d, 0x2, 0x89, /*88940*/
-1, 0x8a, /*89012*/
-1, 0x89, /*89085*/
-1, 0x8a, /*89157*/
-1, 0x89, /*89230*/
-1, 0x8a, /*89302*/
-1, 0x8e, /*89374*/
-1, 0x89, /*89447*/
-1, 0x8a, /*89519*/
-1, 0x89, /*89592*/
-1, 0x8a, /*89664*/
-3, 0x8d, 0x1, 0x89, /*89754*/
-1, 0x8a, /*89826*/
-1, 0x89, /*89899*/
-1, 0x8a, /*89971*/
-1, 0x89, /*90044*/
-1, 0x8a, /*90116*/
-1, 0x8e, /*90188*/
-1, 0x89, /*90261*/
-1, 0x8a, /*90333*/
-3, 0x8d, 0x2, 0x89, /*90423*/
-1, 0x8a, /*90495*/
-1, 0x89, /*90568*/
-1, 0x8a, /*90640*/
-1, 0x89, /*90713*/
-1, 0x8a, /*90785*/
-1, 0x8e, /*90857*/
-1, 0x89, /*90930*/
-1, 0x8a, /*91002*/
-1, 0x89, /*91075*/
-1, 0x8a, /*91147*/
-3, 0x8d, 0x1, 0x89, /*91237*/
-1, 0x8a, /*91309*/
-1, 0x89, /*91382*/
-1, 0x8a, /*91454*/
-1, 0x89, /*91527*/
-1, 0x8a, /*91599*/
-1, 0x89, /*91672*/
-1, 0x8a, /*91744*/
-1, 0x8e, /*91816*/
-1, 0x89, /*91889*/
-1, 0x8a, /*91961*/
-3, 0x8d, 0x2, 0x89, /*92051*/
-1, 0x8a, /*92123*/
-1, 0x89, /*92196*/
-1, 0x8a, /*92268*/
-1, 0x89, /*92341*/
-1, 0x8a, /*92413*/
-1, 0x8e, /*92485*/
-1, 0x89, /*92558*/
-1, 0x8a, /*64900*/
-260 /* END */
};
#endif


#define DATA 0x378
#define STATUS DATA+1
#define CONTROL DATA+2

#ifdef DEBUG
#define INLINE 
#else
#define INLINE inline
#endif

struct timeval lasttv;
struct timeval newtv;

unsigned char parity[] = {
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
};

void INLINE ssleep(const int usec) {
   //return;
   gettimeofday(&lasttv, NULL);
   while (1) {
      gettimeofday(&newtv, NULL);
      if (((newtv.tv_usec - lasttv.tv_usec) + ((newtv.tv_sec - lasttv.tv_sec)*1000000)) > usec) {
         break;
      }
   }
}

void INLINE segfault() {
   int* i = 0;
   (*i)++;
   exit(-1);
}

void INLINE dataout(int data) {
	outb(data, DATA);
}

void INLINE ctrlout(int cmd) {
	outb(cmd, CONTROL);
}

int INLINE ctrlin() {
	return inb(CONTROL);
}

void INLINE checkctrl(int control) {
	int ctrl = inb(CONTROL);
	if (ctrl != control) {
		printf("Error, wrong control : %x instead of %x\n", ctrl, control);
      segfault();
	}
}

int INLINE statusin() {
	return inb(STATUS);
}

void INLINE checkstatus(int status) {
	int stat = statusin();
	if (stat != status) {
		printf("Error, wrong status : %x instead of %x\n", stat, status);
		segfault();
	}
}

int INLINE cmdout(int cmd) {
   int stat;   
   ctrlout(cmd);
   ssleep(1);
   stat = statusin();
   checkctrl(0xc0 | cmd);
	return stat;
}

void INLINE checkcmdout(int cmd, int status, int mask) {
   int stat = cmdout(cmd);
	if ((stat & mask) != (status & mask)) {
		printf("Error, wrong status (checkcmdout) : %x instead of %x (mask : %x)\n", stat, status, mask);
		segfault();
	}
}

int INLINE cmddataouts(int cmd, int data, int sleep) {
   int stat;   
   ctrlout(cmd);
   ssleep(sleep);
   stat = statusin();
   dataout(data);
   checkctrl(0xc0 | cmd);
	return stat;
}

int INLINE cmddataout(int cmd, int data) {
   cmddataouts(cmd, data, 10);
}

void INLINE checkcmddataouts(int cmd, int data, int status, int mask, int sleep) {
   int stat = cmddataouts(cmd, data, sleep);
	if ((stat & mask) != (status & mask)) {
		printf("Error, wrong status (checkcmddataout) : %x instead of %x (mask : %x)\n", stat, status, mask);
		segfault();
	}
}

void INLINE checkcmddataout(int cmd, int data, int status, int mask) {
   checkcmddataouts(cmd, data, status, mask, 15);
}

void INLINE data6out(int data) {
   //printf("6 O0,%x\n", data);
   // Must be : cmdout(2, 4[e6])
   checkcmddataout(0x06, data, 0xfe, 0xf7);
   ctrlout(0x06);
   ssleep(10);
   checkcmdout(0x7, 0x7e, 0xf7);
   checkcmdout(0x6, 0xfe, 0xf7);
   ctrlout(0x06);
}

void INLINE data64out(int* data, int start, int end) {
   int i;
   // Must be : cmdout(2, 4[e6])
   checkcmddataout(0x06, data[start], 0xfe, 0xf7);
   //printf("64 (%d, %d) O0,%x", start, end, data[start]);
   for (i = start + 1; i < end; i += 2) {
      ctrlout(0x06);
      checkcmdout(0x07, 0x7e, 0xf7);
      checkcmddataout(0x06, data[i], 0xfe, 0xf7);
      //printf(",%x", data[i]);
      
      ctrlout(0x04);
      checkcmdout(0x05, 0x7e, 0xf7);
      checkcmddataout(0x04, data[i+1], 0xfe, 0xf7);
      //printf(",%x", data[i+1]);
   }
   //printf("\n");
   ctrlout(0x06);
   checkcmdout(0x7, 0x7e, 0xf7);
   checkcmdout(0x6, 0xfe, 0xf7);
   ctrlout(0x06);
}

void writepacketrep(int num, int data, int pend) {
   int end = pend;
   int packet[4];
   
   if (pend == -1) {
      end = 0;
   }
   
   packet[0] = 0x40 | ((num-1) & 0x3F);
   packet[1] = 0x80 | ((num-1) >> 6) | ((data & 0x0F) << 2);
   packet[2] = (data >> 4) | ((end & 0x03)  << 6);
   packet[3] = 0x80 | (end >> 6);
   packet[1] |= (parity[packet[1]&0x3f]) << 6;
   packet[2] |= (parity[packet[1]&0x3f]^1) << 6;
   packet[3] |= (parity[packet[1]&0x3f]) << 6;
   
   dataout(packet[0]);
   dataout(packet[1]);
   dataout(packet[2]);
   if (pend != -1) {
      dataout(packet[3]);
   }
}

/* index : 0 if it is the first band... 
 * type : 0 : classic     (-3, 0x80, 0xff, 0x7f)
 *        1 : auxiliary 1 (-5, 0x8d, 0x1, 0x80, 0xff, 0x7f)
 *        2 : auxiliary 2 (-5, 0x8d, 0x2, 0x80, 0xff, 0x7f)
 */
void sendband(int index, int type) {
   int i, j;
   int stat;

   printf("Initing band(%d - %d)...\n", index, type);

   if (type != 0) {
      checkcmddataout(0x06, 0x8d, 0xfe, 0xf7);
//      ctrlout(0x06);
      checkcmdout(0x07, 0x7e, 0xf7);
      checkcmddataout(0x06, type, 0xfe, 0xf7);
      ctrlout(0x04);
      checkcmdout(0x05, 0x7e, 0xf7);
      checkcmddataouts(0x04, 0x80, 0xf6, 0xf7, 1);
      ctrlout(0x06);
      checkcmdout(0x07, 0x76, 0xf7);
      checkcmddataouts(0x06, 0xff, 0xf6, 0xf7, 1);
   }
   else {
      ctrlout(0x02);
      checkctrl(0xc2);
      checkcmddataouts(0x06, 0x80, 0xfe, 0xf7, 1);
//      ctrlout(0x06);
      checkcmdout(0x07, 0x7e, 0xf7);
      checkcmddataouts(0x06, 0xff, 0xfe, 0xf7, 1);
   }
   ctrlout(0x04);
   ctrlout(0x05);
   ssleep(45000);
   printf("Waiting for ready status...\n", statusin());
   int cnt = 0;
   /*while (((statusin() & 0xf0) != 0x70) && (cnt < 2000)) {
      ssleep(10);
      cnt++;
   }*/
   printf("Band inited (0x%x)\n", statusin());

   /* data */
   //usleep(4);
   
   if (index == 0) {
      for (i = 0; i < 512; i++) {
         writepacketrep(89, 0x42, 0x42);
         writepacketrep(89, 0x00, 0x00);
         writepacketrep(89, 0xC3, 0xC3);
         writepacketrep(89, 0x00, 0x00);
         writepacketrep(89, 0xDB, 0xDB);
         writepacketrep(149, 0x00, 0x00);
      }
   }
   else if (index == 1) {
      for (i = 0; i < 24; i++) {
         writepacketrep(89, 0x01, 0x01);
         writepacketrep(89, 0x42, 0x42);
         writepacketrep(89, 0x01, 0x01);
         writepacketrep(89, 0xC3, 0xC3);
         writepacketrep(89, 0x01, 0x01);
         writepacketrep(149, 0x00, (i == 23) ? -1 : 0x00);
      }
   }
/*   }
   else {
      i = 1;
      dataout(0x7f);
      dataout(0xbf);
      dataout(0x4f);
      dataout(0x80);
   }*/
   else {
      for (i = 0; i < 233; i++) {
         writepacketrep(256, 0x00, 0x00);
/*         dataout(0x7f);
         dataout(0x83);
         dataout(0x40);
         dataout(0x80);   */
      }
      writepacketrep(119, 0x00, -1);
   }
   /*dataout(0x4d);
   dataout(0x83);
   dataout(0x40);*/
   
   checkctrl(0xc5);
   checkcmddataouts(0x04, 0x89, 0xf6, 0xf7, 3000);
   ctrlout(0x06);
   checkcmdout(0x07, 0x76, 0xf7);
   checkcmdout(0x06, 0xf6, 0xf7);
   ctrlout(0x06);
}

void sendpage() {
   int i = 0;
   int sig = 0;
   int ret = 0;
   int offset = 0;
   int len = 0;

   printf("Start init\n");
   dataout(0x24);
   //ssleep(40000);   
   //checkctrl(0xc6);
   dataout(0x06);
/*   printf("Loop one\n");
   for (i = 0; i < 100; i++) { //839
      statusin();
      ssleep(40000);
   }
   printf("end\n");*/
   ssleep(100);
   //checkctrl(0xc6);
   ctrlout(0x0a);
   ctrlout(0x0a);
   ctrlout(0x0e);
   ssleep(1000000); //16
   dataout(0x24);
   checkctrl(0xce);
   ctrlout(0x06);
   ssleep(150); /* 100-250 */
   checkstatus(0x3e);
   checkctrl(0xc6);
   ctrlout(0x07);
   ctrlout(0x07);
   ctrlout(0x04);
   ssleep(40);
   checkstatus(0xde);
   checkctrl(0xc4);
   ctrlout(0x06);
   ssleep(40);
   checkstatus(0xfe);
   ssleep(10);
   checkctrl(0xc6);
   ctrlout(0x06);
   sig = 0; /* true if a 5e has been received */
   i = 0; /* 0e - 4e count */
   printf("Loop two\n");
   while (1) {
      ret = cmdout(0x02);
      if (ret == 0x0e) {
         i++;
      }
      else if (ret == 0x1e) {
         i = 0;      
      }
      else {
   		printf("Error, wrong status (init 2nd loop) : %x instead of 0x[01]e\n", ret);
   		free(NULL);
   		exit(0);
      }
      
      if (sig && (i == 21)) {
         break;
      }
      
      ret = cmdout(0x00);
      if (ret == 0x4e) {
         i++;
      }
      else if (ret == 0x5e) {
         i = 0;
         sig = 1;
      }
      else {
   		printf("Error, wrong status (init 2nd loop) : %x instead of 0x[45]e\n", ret);
   		free(NULL);
   		exit(0);
      }    
   }
   printf("end\n");
   
   checkcmdout(0x06, 0xfe, 0xff);
   ctrlout(0x04);
   checkcmdout(0x0c, 0x2e, 0xff);
   ctrlout(0x0c);
   ssleep(15);
   dataout(0x20);
   checkctrl(0xcc);
   checkcmdout(0x06, 0x3e, 0xff);
   ctrlout(0x07);
   ctrlout(0x07);
   ctrlout(0x04);
   ssleep(40);
   checkstatus(0xde);
   checkctrl(0xc4);
   ctrlout(0x06);
   ssleep(40);
   checkstatus(0xfe);
   sleep(2);
   printf("Loop three\n");
   for (i = 0; i < 12287; i++) {
      dataout(0);
   }
   printf("end\n");
   ssleep(500);
   checkstatus(0xfe);
   dataout(0xa0);
   checkctrl(0xc6);
   ctrlout(0x06);
   checkcmdout(0x07, 0x7e, 0xff);
   ctrlout(0x06);
   ssleep(10);
   checkstatus(0xfe);
   dataout(0x00);
   checkctrl(0xc6);   
   ctrlout(0x04);
   checkcmdout(0x05, 0x7e, 0xff);
   ctrlout(0x04);
   ssleep(20);
   checkstatus(0xfe);
   dataout(0xa0);
   checkctrl(0xc4);
   ctrlout(0x06);
   checkcmdout(0x07, 0x7e, 0xff);
   checkcmdout(0x06, 0xfe, 0xff);
   ctrlout(0x06);
   offset = 0;
   
   printf("Waiting for init end...\n");
   sleep(5);
   
   printf("Sending page...\n");
   i = 0; //Band counter
   sig = -1;
   
   int* data = &page;
   
   while (1) {
      ret = cmdout(0);
      if (ret != sig) {
         //printf("sig : 0x%x\n", sig);
         sig = ret;
      }
      if ((cmdout(2) & 0xf7) == 0x46) { //0x46 or 0x4e
         len = -data[offset];
         if (len == 260) {
            offset = 0;
            data = &bandinit;
         }
         else if (len > 255) {
            printf("Sending band %d...\n", i);
            sendband(i, len-256);
            offset++;
            
            i++;
         }
         else if (len > 1) {
            if (data[offset+2] == -1) {
               data[offset+2] = (i % 2) + 1;
               data64out(data, offset+1, offset+1+len);
               data[offset+2] = -1;
            }
            else {
               data64out(data, offset+1, offset+1+len);
            }
            offset += len+1;
         }
         else {
            data6out(data[offset+1]);
            offset += 2;
         }
      }
      if (i > 40) {
         break;
      }
   }
   printf("OK\n");
}

int main(void) {
   if (ioperm(DATA,3,1)) {
      printf("Sorry, you were not able to gain access to the ports\n");
      printf("You must be root to run this program\n");
      exit(1);
   }

   sendpage();

   return 0;
}
