dnl $Id$
dnl config.m4 for extension sinaip

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(sinaip, for sinaip support,
[  --with-sinaip             Include sinaip support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(sinaip, whether to enable sinaip support,
dnl Make sure that the comment is aligned:
dnl [  --enable-sinaip           Enable sinaip support])

if test "$PHP_SINAIP" != "no"; then
  dnl Write more examples of tests here...

  --with-sinaip -> check with-path
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/sinaip.h"  # you most likely want to change this
  if test -r $PHP_SINAIP/$SEARCH_FOR; then # path given as parameter
    SINAIP_DIR=$PHP_SINAIP
  else # search default path list
    AC_MSG_CHECKING([for sinaip files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        SINAIP_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  
  dnl if test -z "$SINAIP_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the sinaip distribution])
  dnl fi

  # --with-sinaip -> add include path
  PHP_ADD_INCLUDE($SINAIP_DIR/include)

  dnl # --with-sinaip -> check for lib and symbol presence
  dnl LIBNAME=sinaip # you may want to change this
  dnl LIBSYMBOL=sinaip # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SINAIP_DIR/lib, SINAIP_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SINAIPLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong sinaip lib version or lib not found])
  dnl ],[
  dnl   -L$SINAIP_DIR/lib -lm
  dnl ])
  
  PHP_SUBST(SINAIP_SHARED_LIBADD)

  PHP_NEW_EXTENSION(sinaip, sinaip.c, $ext_shared)
fi
