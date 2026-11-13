#include <stdio.h>

#define RECOVER_BITMAP  yes

#define PIXELS_BY_ROW 	4800
#define ROWS_BY_BAND  	104
#define MAX_BAND		61


char cbm[83200];

main (int argc, char *argv[] ) {
	FILE *f,*g,*h;
	int n,i,j,size,bsize,maxbsize;
	long int totsize;
	char *cbmp;
	unsigned char a,b,c;
	unsigned char c1,c2,c3,c4;
	int k;

	for (n=1;n<argc;n++) {
		f = fopen( argv[n], "r" );
#ifdef 	RECOVER_BITMAP
		g = fopen( "recovered.pbm","w" );
		fprintf(g,"P4\n# recovered by cmpanal\n4800 6774\n");
#endif
		//h = fopen( "band.compressed","w");
		totsize=0;
		maxbsize=0;
		for (i=0;!feof(f);i++) {
			if (!fread( &size,1,sizeof(int),f ))
				break;
			fread( cbm,size*4,1,f );
			bsize=0;
			cbmp = cbm;
			for (j=0;j<size;j++) {
				c1 = *cbmp++;	
				c2 = *cbmp++;	
				c3 = *cbmp++;	
				c4 = *cbmp++;
				//if (i==47)
				//	fprintf(h,"c1=%02x c2=%02x c3=%02x c4=%02x\n",c1,c2,c3,c4);
				a = (c2<<6)&0xc0|c1&0x3f;
				b = (c3<<4)&0xf0|(c2>>2)&0x0f;
				c = (c4<<2)&0xfc|(c3>>4)&0x03;
				if (c1&0x40) {
					bsize += ((int)a+2);
#ifdef 	RECOVER_BITMAP
					for (k=0;k<((int)a+1);k++) {
						fwrite(&b,1,1,g);
					}
					fwrite(&c,1,1,g);
#endif
				}
				else {
					bsize += 3;
#ifdef 	RECOVER_BITMAP
					fwrite(&a,1,1,g);
					fwrite(&b,1,1,g);
					fwrite(&c,1,1,g);
#endif
				}
			}
			totsize += bsize;
			if (size > maxbsize) maxbsize=size;
			printf("band: %d, bsize: 0x%x strlength: %d\n", i, bsize, size);
		}
		printf("TOTAL size: %ld , 0x%lx\n",totsize,totsize);
		printf("Max band string size: %d\n",maxbsize);
		//fclose(h);
#ifdef 	RECOVER_BITMAP
		fclose(g);
#endif
		fclose(f);
	}
}

