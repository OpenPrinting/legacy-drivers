AC_INIT([rastertopocketjet], [2.0.0], [Pentax <tmp@tmp.com>],
        [pocketJetCUPS])
AM_INIT_AUTOMAKE

dnl CUPS dirs
AC_PATH_PROG(CUPS_CONFIG, cups-config)
if test -x "${CUPS_CONFIG}" ; then
  cups_version=`${CUPS_CONFIG} --version`

  cups_conf_datadir="`$CUPS_CONFIG --datadir`"
  cups_conf_serverbin="`$CUPS_CONFIG --serverbin`"
  cups_conf_serverroot="`$CUPS_CONFIG --serverroot`"
fi

AM_CONFIG_HEADER(config.h)
AC_PROG_CC
AC_PROG_INSTALL
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
