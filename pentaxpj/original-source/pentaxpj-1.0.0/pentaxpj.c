/*
 * Printer Filter for Pentax PocketJet / PocketJet II
 * (C) Copyright 2001, Rildo Pragana, http://pragana.net
 * With many contributions and suggestions by Rob Roy, 
 *		Pentax Technologies, http://pentaxtech.com
 * This is free software, released under the General Public License (GPL)
 * The development of this work was sponsored by Pentax Technologies.
 */

#include "pentaxpj.h"

int lp=0;
FILE *bitmapf;
int bmwidth=0, bmheight=0;
int lpdpi = 300;
int lpwidth=308, lpheight=3200;
unsigned char buf[1000];
unsigned char cmdbuf[10]={'\033','~'};
char *gscommands[] = { "/usr/bin/gs", "/bin/gs", "/usr/local/bin/gs", NULL };
char *gscmd=NULL;
char res_str[30]="-r300x300";
char geom_str[30]="";
char *lpdevice=NULL;
int optT=0, optd=175, optR=0, optP=0;
int optf=1, opta=0;
char *opto=NULL;
int tmpwidth=0, tmpheight=0;
int verbose=0;
int serial_flg=0;
struct termios oldtio,newtio;
struct serial_struct oldserialst,serialst;
int a4size=0; // used when sending initial params to printer for A4 size to use LENGTH command instead.

/* paper sizes */
struct paper_size_st {
		char *name;
		int  h200, h300;
		int  w200, w300;
		char *gsgeom200, *gsgeom300;
} paper_size[] = {
	{"letter", 2133, 3200, 204, 308, "1700x2200", "2550x3300"},
	{"legal",  2733, 4100, 204, 308, "1700x2800", "2550x4200"},
/* The printer's internal definition of printable area for A4 size is unnecessarily small.
   This works OK on Windows drivers since the printable area is made known to applications.
   However, our Linux driver works by creating a bitmap based on the whole paper size, and then
   throwing away data at printer top/bottom unprintable area, hoping that we don't throw away
   anything important. Well, in many cases with A4, we were throwing away too much data.
   So, let's define the printable area to be larger, as the printer can really handle it anyway.
   And, we are now using the LENGTH command anyway for A4 size for other printer firmware issues anyway,
   so the printer will already be expecting to have more data corresponding to this new printable area.

*/
//	{"a4",     2200, 3300, 200, 300, "1654x2338", "2481x3507"},
	{"a4",     2272, 3406, 200, 300, "1654x2338", "2481x3507"},
	{NULL, 0,0,0,0,NULL,NULL}
};

/*
 * locate ghostscript program in a path list
 */
void
find_gs () {
	char **tmp;
	tmp = gscommands; 
	while (*tmp != NULL) {
		if (!access(*tmp,X_OK)) {
			break;
		}
		tmp++;
	}
	gscmd = *tmp;
	if (gscmd == NULL) {
		fprintf(stderr,"*** ghostscript interpreter (gs) not found!\n");
		exit(1);
	}
}

/*
 * low-level commaNds to the printer
 */
void
cmdreset () {
	write(lp,"\033@",2);
}

void
cmdff(int yc) {
	int i;
/* avoid bug, as reported by PTC (firmware bug). 
   Now following suggestions given by Rob Roy (only if yc!=0) */
	if (yc!=0)
		write(lp,"\033~*\x00\x00",5);
#ifdef	LONG_FORM_FEED
	write(lp,"\033~\f",3);
#else
	write(lp,"\f",1);
#endif
}

void
cmdh1 (int cmd, int parm) {
	sprintf(cmdbuf+2,"%c%c",cmd,parm);
	write(lp,cmdbuf,4);
}

void
cmdh2 (int cmd, int parm) {
	sprintf(cmdbuf+2,"%c%c%c",cmd,parm % 256, parm / 256);
	write(lp,cmdbuf,5);
}

/*
 * read_bitmap -- read buffer from the bitmapf channel
 *      n = 0 --> get a single line at most HDRLINE_SIZE
 *      n > 0 --> read the number of bytes given by n
 * Returns -1 if error found, 0 if end-of-file, 
 *       or 1 if something read 
 */
int
read_bitmap (int n) {
	struct pollfd ufds;
	char *s;
	int cnt;
	if (verbose > 1) {
		fprintf(stderr,"read_bitmap: %d\n",n);
	}
	if (!n) {
		s = buf;
		cnt = HDRLINE_SIZE;
		while (cnt--) {
			ufds.fd = fileno(bitmapf);
			ufds.events = POLLIN;
			if (poll(&ufds,1,PSHDR_TIMEOUT) <= 0) {
				if (verbose > 2) {
					fprintf(stderr,"poll revents=0x%x\n",ufds.revents);
				}
				if (ufds.revents && POLLHUP)
					return 0;
				return -1;
			}
			if (feof(bitmapf))
				return 0;
			*s = fgetc(bitmapf);
			if (*s == '\n') {
				*++s=0;
				break;
			}
			s++;
		}
	}
	else {
		s=buf;
		while (n) { 
			ufds.fd = fileno(bitmapf);
			ufds.events = POLLIN;
			if (poll(&ufds,1,PSDATA_TIMEOUT) <= 0) {
				if (verbose > 2) {
					fprintf(stderr,"poll revents=0x%x\n",ufds.revents);
				}
				if (ufds.revents && POLLHUP)
					return 0;
				return -1;
			}
			if (feof(bitmapf))
				return 0;
			n -= fread(s,1,n,bitmapf);
			s += n;
		}
	}
	return 1;
}

/*
 * find a cluster of at least "n" null bytes 
 * in a string "s" of a given "size"
 */
unsigned char *
find_zeros( unsigned char *s, int size, int n ) {
	register unsigned char *r,*t;
	register int i=0;
	for (r=s ; r < (s+size-n) ; r+=(i+1) ) {
		for (t=r,i=0;i<n;i++) 
			if (*t++) break;
		if (i==n) return r;
	}
	return NULL;
}

/*
 * skip "n" pixel lines
 */
void
skip_lines ( int n ) {
	while (n>255) { 
		/*ESC~J\x01       Flush line buffer and advance to next line*/
		cmdh1('J',255);
		n -= 255;
	}
	if (n) 
		cmdh1('J',n);
}

/*
 * send bitmap page(s) got from gs pipe to the printer
 */
int
send_bitmap (int skip) {
	int i,j,rb;
	int w,n,wmax;
	unsigned char *s,*t;
	int xc=0,yc=0;
	int newln=0;

	if (verbose > 2)
		fprintf(stderr,"start to send bitmap\n");
	while (!feof(bitmapf)) {
		if ((rb=read_bitmap(0))==0) {
			if (verbose > 2)
				fprintf(stderr,"read_bitmap returned 0\n");
			break;
		}
		if (rb < 0)
			break;
		if (strncmp(buf,"P4",2)) {
			if (verbose) {
				fprintf(stderr,"Error in bitmap file header: %s\n",buf);
			}
			return 0;
		}
		/* bypass the comment line */
		do {
			if (read_bitmap(0)<=0) {
				fprintf(stderr,"Bitmap file seems truncated.\n");
				exit(1);
			}
		} while (buf[0] == '#');
		/* read the bitmap's dimensions */
		if (sscanf(buf,"%d %d",&bmwidth,&bmheight)<2) {
			fprintf(stderr,"Bitmap file with wrong size fields.\n");
			exit(1);
		}
		bmwidth = (bmwidth+7)/8;
		if (verbose) 
			fprintf(stderr,"width = %d, height= %d\n",bmwidth, bmheight);
		wmax = min(lpwidth,bmwidth);

		/* skip top lines (non-printing area) */
		for (i=0;i< skip;i++) {
			if (read_bitmap(bmwidth)<=0)
				break;
		}
		/* send printable area */
		for (i=skip;i<min(lpheight,bmheight);i++) {
			if (verbose>1) {
				fprintf(stderr,"line %d\n",i);
				fflush(stderr);
			}
			if (read_bitmap(bmwidth)<=0)
				break;
			/* send compressed line to device */
// NOTE: wmax will only allow printable width. bmwidth is likely larger than printable width.
// So, data from right edge of bitmap will be discarded to fit in printable area.
			s = t = buf;
			w = wmax;
			while (w) {
				n = COMP_THRESHOLD; 
				t = find_zeros( s, w, n);
				if ((t > s) || (t==NULL)) {
					if (t==NULL) {
						t = s+w;
					}
					if (yc) {
						skip_lines( yc );
						yc=0;
					}
					/*ESC~$##         Set cursor X  command*/
					cmdh2('$',(s-buf)*8);
					/* send pixels till next zeros cluster */
					newln++;
					/*ESC~*++++       Image Data transfer command.*/
					cmdh2('*',t-s);
					if ((write(lp,s,t-s) < 0) && verbose) {
						fprintf(stderr,
							"send_bitmap: error %s writing to device\n",
							strerror(errno));
					}
					w -= (t-s);
					s = t;
				}
				if (w) {
					/* locate next not null pixels cluster */
					while (*(s+n) == 0) {
						if (n == w)
							break;
						n++;
					}
					if (n == wmax) {
						yc++;
						break;
					}
					s += n;
					w -= n;
				}
			}
			if (newln) { 
				skip_lines( 1 );
				newln=0;
			}
		}
		cmdff(yc);
		/* Need to clear the accumulating skip lines, 
			since we just did an FF */
		yc = 0;

// This code is intended to discard the bottom of the bitmap corresponding to the printer's
// bottom unprintable margin...similar to how the "skip" discards the portion of bitmap
// corresponding to printer's top unprintable margin.			
		for (i=lpheight;i<bmheight;i++) {
			/* must keep reading until full bitmap arrives 
				or report an error */
			if (read_bitmap(bmwidth)<=0) {
				fprintf(stderr, "premature end of bitmap (%d)\n",__LINE__);
				break;
			}
		}
	}
	/* if NoFeedMode, send extrafeed and another form feed...do this only 
		at EndDoc time, since it messes with firmware at EndPage time.
	*/
	if (optf == 0 && opta > 0) {
		skip_lines (opta);
		cmdff(0);
	}
	return 0;
}

void
setup_printer () {
// This is an obsolete command. It is no longer supported in either PJII or PJ200.
// Continue to send for historical reasons only.
    /*ESC~q#      Hex(1)      Print Quality Set.*/
	cmdh1('q',optT);
    /*ESC~d##     Hex(2)      Set print density level(0-255).*/
	cmdh2('d',optd);
  	if (a4size) {

		/*ESC~w##      Set Page Width (in bytes)*/
		cmdh2('w',lpwidth);

		/* ESC~l## Set Variable Page Length instead of page width, because of firmware issues.
		
		For PJII, sending ESC~l(3406) is for legacy reasons, since original firmware had a bug
		in case that the last line of printable area had data, the page would not eject.

		For PJ200, sending ESC~l(2272) is for a custom case where perforated roll paper is used.
		Perf roll paper solution requires "end of page" FF mode along with a black mark printed
		on the paper to allow sensing "out of paper". The firmware sets its internal bottom margin
		differently if it thinks it is using A4 paper, causing problems to develop a black mark spec.
		So, using this LENGTH command, the firmware does not think it is using A4 paper, so it sets
		the internal bottom margin consistently with how it is done for Letter/legal paper, allowing
		for a consistent black mark specification. This setting will work well with "cutsheet"
		A4 paper too.
		*/

		cmdh2('l', (lpdpi==200) ? 2272 : 3406 );
	}
	else {
		/*ESC~w##      Set Page Width (in bytes)*/
		cmdh2('w',lpwidth);
		/*ESC~h##      Set Page Height (in raster lines)*/
		cmdh2('h',lpheight);
    }
	/*ESC~f#       Set Paper Eject Mode (or Form Feed Mode)*/
	cmdh1('f',optf);
}

void
gs_handler (int sig) {
	if (verbose)
		fprintf(stderr,"gs command terminated!\n");
//	exit(0);
}

void
cat_handler (int sig) {
	if (verbose)
		fprintf(stderr,"cat command terminated!\n");
//	exit(0);
}

void
null_handler (int sig) {
	if (verbose)
		fprintf(stderr,"ignoring SIGCHLD\n");
}

/* 
 * device specific initialization 
 */
void
device_init( ) {
	struct stat stbuf;
	int c, res,maj,m;

	fstat( lp,&stbuf );
	maj = major(stbuf.st_rdev);
	if (verbose)
		fprintf(stderr,"lpdevice major: %d, minor: %d\n", 
			maj, minor(stbuf.st_rdev));
	if (maj != 4) {
		return;
	}
	serial_flg++;
	tcgetattr(lp,&oldtio); /* save current port settings */
	memcpy(&newtio, &oldtio, sizeof(newtio));
	newtio.c_cflag = B57600 | CS8 | CSTOPB | CLOCAL | CREAD;
	newtio.c_iflag = IXON | IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_cc[VTIME]    = 0; 
	newtio.c_cc[VMIN]     = 1;    
	newtio.c_line         = N_TTY;    

	/*tcflush(lp, TCIFLUSH);*/
	tcsetattr(lp,TCSANOW,&newtio);
	
	/* setup as UNKNOWN serial type (to disable internal FIFO) */
	ioctl(lp,TIOCGSERIAL,&oldserialst);
	memcpy(&serialst, &oldserialst, sizeof(serialst));
	serialst.type = PORT_UNKNOWN;
	serialst.closing_wait = ASYNC_CLOSING_WAIT_INF;
	serialst.flags |= ASYNC_LOW_LATENCY;
	ioctl(lp, TIOCSERCONFIG);
	ioctl(lp,TIOCSSERIAL,&serialst);

	/* setup DTR, RTS */
	ioctl(lp, TIOCMGET, &m);
	m |= TIOCM_DTR;
	m &= ~TIOCM_RTS;
	ioctl(lp, TIOCMSET, &m);
}

void
dump_pbmfile( char *fname ) {
	if (signal(SIGCHLD, null_handler) == SIG_ERR) {
		fprintf(stderr, "cannot set handler for SIGCHLD\n");
		exit(1);
	}
	if (fname==NULL) {
		if (verbose) 
			fprintf(stderr,"reading from stdin\n");
			bitmapf = stdin;
			//setvbuf(bitmapf,NULL,_IONBF,0);
	}
	else {
		bitmapf = fopen( fname,"r" );
	}
	/*fcntl(fileno(bitmapf),F_SETFL,(long)O_NONBLOCK);
	*/
	/*send_bitmap( (lpdpi==200) ? PAGE_TOP_SKIP200 : 
					PAGE_TOP_SKIP300 );*/
	send_bitmap(0);
	fclose(bitmapf);
}

void set_paper( char *req ) {
	int i;
	struct paper_size_st *p;

	// need to know if using A4 paper so we can send variable length command instead of height command
	if (!strcmp(req,"a4")) 
		a4size++;

	for (i=0; paper_size[i].name != NULL; i++) {
		if (!strcmp(paper_size[i].name,req)) {
			p = paper_size + i;
			if (lpdpi==200) {
				lpheight = p->h200;
				lpwidth = p->w200;
				sprintf(geom_str,"-g%s",p->gsgeom200);
				// need to override any previous setting with -W and -H switch
				tmpheight = 0;
				tmpwidth = 0;
			}
			else {
				lpheight = p->h300;
				lpwidth = p->w300;
				sprintf(geom_str,"-g%s",p->gsgeom300);
				// need to override any previous setting with -W and -H switch
				tmpheight = 0;
				tmpwidth = 0;
			}
			return;
		}
	}
	fprintf(stderr,"*** unknown paper size\n");
	exit(1);
}

void
usage() {
	fprintf(stderr,"\nusage: pentaxpj -[2RPTvdWHopcwlinh] file ...\n");
	fprintf(stderr,"option summary:\n");
	fprintf(stderr,"\t-2         set 200x200 pixel resolution\n");
	fprintf(stderr,"\t-R         reset printer and exit\n");
	fprintf(stderr,"\t-P         print raw pbm file\n");
	fprintf(stderr,"\t-T         text quality (default graphics)\n");
	fprintf(stderr,"\t-d dens    set density\n");
	fprintf(stderr,"\t-f mode    set feed mode (0,1, or 2)\n");
	fprintf(stderr,"\t-a lines   add lines in \"No feed\"(-f 0) mode\n");
	fprintf(stderr,"\t-W width   set paper width (* 8 pixels units)\n");
	fprintf(stderr,"\t-H height  set paper height in pixels\n");
	fprintf(stderr,"\t-g geom    set ghostscript geometry <width>x<height>\n");
	fprintf(stderr,"\t-o file    send output to a file\n");
	fprintf(stderr,"\t-p device  send output to a device\n");
	fprintf(stderr,"\t-s paper   set paper size (letter, a4, or legal)\n");
	fprintf(stderr,"\t-v, -vv    verbose, more verbose\n");
	fprintf(stderr,"\tother options specific of lpd (ignored)\n");
	exit(0);
}

/* parse command line arguments */
void
parse_opts( int argc, char **argv ) {
	int c;
	while ((c = getopt(argc,argv,"2RPTvf:a:d:W:H:o:p:hg:s:")) != -1) {
		switch (c) {
			case 'f': {
				sscanf(optarg,"%d",&optf);
				break;
			}
			case 'a': {
				sscanf(optarg,"%d",&opta);
				break;
			}
			case '2': {
				strcpy(res_str,"-r200x200");
				/* set defaults for letter paper */
				lpwidth = 204;
				lpheight = 2133;
				lpdpi = 200;
				break;
			}
			case 'P': {
				optP=1;
				break;
			}
			case 'R': {
				optR=1;
				break;
			}
			case 'T': {
			// Note: This is used for "quality mode" (ESC~q# command).
			// This command is NOT supported inside printer anymore.
			// We support this switch now only for legacy reasons.
				optT=1;
				break;
			}
			case 'd': {
				sscanf(optarg,"%d",&optd);
				/*optd = (optd/25)*25;*/
				optd = optd*25;
				break;
			}
// NOTE: W and H params are now only needed for "custom" page size.
// Setting a standard paper size is now done more simply with the -s <papersizestr> command
			case 'W': {
				sscanf(optarg,"%d",&tmpwidth);
				break;
			}
			case 'H': {
				sscanf(optarg,"%d",&tmpheight);
				break;
			}
			case 'g': {
				sprintf(geom_str,"-g%s",optarg);
				break;
			}
			case 'o': {
				opto = strdup(optarg);
				break;
			}
			case 'p': {
				lpdevice = strdup(optarg);
				break;
			}
			case 's': {
				set_paper(optarg);
				break;
			}
			case 'v': {
				verbose++;
				break;
			}
			case 'h': 
			case '?': {
				usage();
				break;
			}
		}
	}
}

int
main(int argc, char *argv[]) {
	char **fname;
	int optcnt;
	char *optns[30];
	char optbuf[80];
	FILE *cfgf;
	char *s,*sbp;
	int gspipe[2];
	int catpipe[2];
	char catbuf[1024];
	int catn,catfd;
	pid_t gspid,catpid;
	int stdin_flg=0;
	
	/* check defaults saved in our config file */
	// these will be used whether driver is called as "pentaxpj" or "pentaxpj_sh"
	if (!access(CFGFN,R_OK)) {
		cfgf = fopen( CFGFN, "r" );
		fgetc(cfgf); /* bypass '#' comment */
		optcnt = 0;
		optns[optcnt++] = argv[0];
		fgets( optbuf, 80, cfgf );
		sbp = optbuf;
		while( (s = strsep( &sbp, " \t\n" )) != NULL) {
			optns[optcnt++] = strdup(s);
		}
		optns[--optcnt] = NULL;
		parse_opts( optcnt, optns );
		optind = 0;
	}
	/* defaults will be overriden by cmd line args */
	/* only available if called as pentaxpj_sh */
	if (verbose)
		fprintf(stderr,"command \"%s\"\n",argv[0]);
	if (strstr(argv[0],"pentaxpj_sh") != NULL) {
		parse_opts( argc, argv );
	}
	/* else clean argument list */
	// "pentaxpj" as used with spooler will NOT allow a new command line.
	// it will _only_ use the values stored in the config file. 
	else {
		argc = 0;
		argv[0] = NULL;
	}
	/*if (*(argv+optind) == NULL)
		usage();*/

// tmpwidth and tmpheight are set using W and H command line options.
// they will be reset to 0 in the case that the -s <papersizestr> command is also used.
	if (tmpwidth) {
		lpwidth = tmpwidth;
	}
	if (tmpheight) {
		lpheight = tmpheight;
	}
	if (opto!=NULL) {
		lp = open(opto,O_WRONLY | O_TRUNC | O_CREAT);
	}
	else {
		if (lpdevice != NULL) {
			lp = open(lpdevice,O_RDWR | O_NOCTTY);
			/*lp = open(lpdevice,O_WRONLY);*/
		}
		else {
			lp = STDOUT_FILENO;
		}
		device_init();
	}
	cmdreset();
	if (optR) {
		goto closelp;
	}
	if (!optP)
		find_gs();
	if (verbose)
		fprintf(stderr,"gs found at %s\n",gscmd);
	setup_printer(); 
	if (verbose)
		fprintf(stderr,"printer setup done\n");
	if (*(argv+optind) == NULL) {
		stdin_flg++;
	}
	/* setup gscmd geometry option */
	if (!(*geom_str)) {
		sprintf(geom_str,"-g %dx%d", 
			(int)(DEFAULT_PAPER_WIDTH * lpdpi),
			(int)(DEFAULT_PAPER_HEIGHT * lpdpi));
	}
	for (fname=argv+optind;stdin_flg || (*fname != NULL); fname++) {
		if (optP) {
			dump_pbmfile( *fname );
		}
		else {
			if (verbose) 
				fprintf(stderr,"geom_str: %s\n",geom_str);
			pipe(gspipe);
			if (signal(SIGCHLD, gs_handler) == SIG_ERR) {
				fprintf(stderr, "cannot set handler for SIGCHLD\n");
				exit(1);
			}
			if (verbose)
				fprintf(stderr,"fork gs process\n");
			gspid = fork();
			if (gspid == (pid_t) 0) {
				int nullfd;
				if (!verbose) {
					nullfd=open("/dev/null",O_WRONLY);
					dup2(nullfd,STDERR_FILENO);
				}
				close(gspipe[0]);
				dup2(gspipe[1],STDOUT_FILENO);
				if (stdin_flg) {
					if (verbose) {
						fprintf(stderr,"start gs pipe from stdin\n");
						fprintf(stderr,"gs args: %s %s\n",
							res_str,geom_str);
					}
					execl( gscmd,gscmd,"-q","-dNOPAUSE",res_str,geom_str,
						"-sDEVICE=pbmraw","-sOutputFile=-","-", NULL ); 
				}
				else {
					if (signal(SIGCHLD, cat_handler) == SIG_ERR) {
						fprintf(stderr, "cannot set handler for SIGCHLD\n");
						exit(1);
					}
					pipe(catpipe);
					catpid = fork();
					if (catpid == (pid_t) 0) {
						close(STDIN_FILENO);
						close(catpipe[0]);
						dup2(catpipe[1],STDOUT_FILENO);
						//dup2(STDOUT_FILENO,STDERR_FILENO);
						if ((catfd = open(*fname,O_RDONLY))<0) {
							fprintf(stderr,"error opening file %s\n",
								*fname);
							exit(1);
						}
						while(1) {
							catn = read(catfd,catbuf,1024);
							if (verbose > 1)
								fprintf(stderr,"cat: read %d bytes\n",catn);
							if (catn <= 0) {
								if (catn) {
									fprintf(stderr,"cat: error %s\n",
										strerror(errno));
								}
								exit(0);
							}
							else {
								write(STDOUT_FILENO,catbuf,catn);
							}
						}
					}
					else {
						close(catpipe[1]);
						dup2(catpipe[0],STDIN_FILENO);
						if (verbose) {
							fprintf(stderr,"start gs to convert %s\n",*fname);
							fprintf(stderr,"gs args: %s %s\n",
								res_str,geom_str);
						}
						execl( gscmd,gscmd,"-q","-dNOPAUSE",res_str,geom_str,
							"-sDEVICE=pbmraw","-sOutputFile=-","-", NULL ); 
					}
				}
			}
			else {
				close(gspipe[1]);
				bitmapf = fdopen(gspipe[0],"r");
				fcntl(fileno(bitmapf),F_SETFL,(long)O_NONBLOCK);
				if (verbose)
					fprintf(stderr,"send bitmap to printer\n");
// NOTE: GS will format the bitmap based on complete page size. This allows font scaling, etc to remain in proportion.
// However, the printers printable area will be less than a full page size. So, the print data will be discarded at the
// top corresponding to the printer's top unprintable margin, which is different for PJII and PJ200.
				send_bitmap( (lpdpi==200) ? PAGE_TOP_SKIP200 : 
					PAGE_TOP_SKIP300 );
				fclose(bitmapf);
				if (stdin_flg)
					break;
			}
		}
	}
closelp:
	if (serial_flg) {
		tcdrain(lp);
		ioctl(lp, TIOCSERCONFIG);
		tcsetattr(lp,TCSANOW,&oldtio);
	}
	close(lp);
	return 0;
}

/* end of pentaxpj.c */

