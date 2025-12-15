bin_PROGRAMS = min12xxw esc-m
man_MANS = min12xxw.1
dist_man_MANS = min12xxw.1
min12xxw_SOURCES = min12xxw.c getopt.c getopt.h getopt1.c getline.c getline.h
esc_m_SOURCES = esc-m.c
# make sure a few extra files are included in the distribution tarballs
EXTRA_DIST = FAQ README.OSX format.txt usblogs
# but don't include subversion files
dist-hook:
	rm -rf `find $(distdir)/usblogs -name '.svn'`
