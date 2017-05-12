/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2011 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header 310447 2011-04-23 21:14:10Z bjori $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_sinaip.h"
#include <unistd.h>
#include <sys/mman.h>

/* If you declare any globals in php_sinaip.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(sinaip)
*/

#define BASE_PTR 5
#define	PHP_SINAIP_OBJECT_TAG			"sinaip handler"
#define PHP_SINAIP_MODULE_VERSION "0.5"

/* True global resources - no need for thread safety here */
static int le_sinaip;

/* {{{ sinaip_functions[]
 *
 * Every user visible function must have an entry in sinaip_functions[].
 */
const zend_function_entry sinaip_functions[] = {
	PHP_FE(sinaip_init,	NULL)
	PHP_FE(sinaip_preload,	NULL)
	PHP_FE(sinaip_count,	NULL)
	PHP_FE(sinaip_search,	NULL)
	PHP_FE(sinaip_close,	NULL)
	{NULL, NULL, NULL}	/* Must be the last line in sinaip_functions[] */
};
/* }}} */

/* {{{ sinaip_module_entry
 */
zend_module_entry sinaip_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"sinaip",
	sinaip_functions,
	PHP_MINIT(sinaip),
	PHP_MSHUTDOWN(sinaip),
	PHP_RINIT(sinaip),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(sinaip),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(sinaip),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_SINAIP_MODULE_VERSION, /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SINAIP
ZEND_GET_MODULE(sinaip)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("sinaip.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_sinaip_globals, sinaip_globals)
    STD_PHP_INI_ENTRY("sinaip.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_sinaip_globals, sinaip_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_sinaip_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_sinaip_init_globals(zend_sinaip_globals *sinaip_globals)
{
	sinaip_globals->global_value = 0;
	sinaip_globals->global_string = NULL;
}
*/
/* }}} */

#define	DELREF_SINAIP(x)	{	\
	if (x != NULL) {	\
		Z_DELREF_P(x);		\
		if (Z_REFCOUNT_P(x) <= 0) {	\
			zval_dtor(x);	\
			FREE_ZVAL(x);	\
		}	\
		x = NULL;	\
	}	\
}

static ZEND_RSRC_DTOR_FUNC(php_sinaip_dtor)
{
	if (rsrc->ptr) 
	{
		struct php_sinaip *ps = (struct php_sinaip *) rsrc->ptr;

		fclose(ps->fp);
		efree(ps->index_start);
		efree(ps->index_end);
		efree(ps->index_ptr);
		munmap(ps->mmap, ps->size);
		
		DELREF_SINAIP(ps->zt);
		efree(ps);
		rsrc->ptr = NULL;
	}
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(sinaip)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	le_sinaip = zend_register_list_destructors_ex(php_sinaip_dtor, NULL, PHP_SINAIP_OBJECT_TAG, module_number);
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(sinaip)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(sinaip)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(sinaip)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(sinaip)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "SinaIP support", "enabled");
	php_info_print_table_row(2, "SinaIP Version", PHP_SINAIP_MODULE_VERSION);
	php_info_print_table_row(2, "author", "MagicBear");
	php_info_print_table_row(2, "E-mail", "magicbearmo@gmail.com");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

static int le_sinaip;

static void *_php_create_sinaip()
{
	struct php_sinaip *ps;

	ps = (struct php_sinaip *)emalloc(sizeof(struct php_sinaip));
	ps->zt = NULL;
	ps->index_start = NULL;
	ps->index_end = NULL;
	ps->index_ptr = NULL;
	ps->nCount = 0;
	ps->rsrc_id = ZEND_REGISTER_RESOURCE(NULL, ps, le_sinaip);

	return ((void *)ps);
}

long sinaip_query(struct php_sinaip *ps, unsigned int ip)
{
	int i_s = 1;
	int i_e = ps->nCount;
	int run = 0;
	while (i_s <= i_e)
	{
		int i_mid = (i_s+i_e)/2;
		
		if (ps->index_start[i_mid-1] == 0)
		{
			ps->ptr = BASE_PTR+(i_mid-2)*sizeof(int);
			ps->index_start[i_mid-1] = htonl(*(int *)(ps->mmap+ps->ptr));
			/*
			fseek(ps->fp,BASE_PTR+(i_mid-2)*sizeof(int),SEEK_SET);
			if (1 != fread(&ps->index_start[i_mid-1],sizeof(int),1,ps->fp)) return 0;
			ps->index_start[i_mid-1] = htonl(ps->index_start[i_mid-1]);*/
		}
		
		if (ip == ps->index_start[i_mid-1]){
			return i_mid-1;
		}else if (ps->index_start[i_mid-1] > ip)
		{
			i_e = i_mid-1;
		}else
		{
			i_s = i_mid + 1;
		}
		run++;
	}
	if (i_e > 1 && ps->index_start[i_e-1] == 0)
	{
		ps->ptr = BASE_PTR+(i_e-2)*sizeof(int);
		ps->index_start[i_e-1] = htonl(*(int *)(ps->mmap+ps->ptr));
		/*
		fseek(ps->fp,BASE_PTR+(i_e-2)*sizeof(int),SEEK_SET);
		if (1 != fread(&ps->index_start[i_e-1],sizeof(int),1,ps->fp)) return 0;
		ps->index_start[i_e-1] = htonl(ps->index_start[i_e-1]);*/
	}
	if (i_e > 1 && ps->index_end[i_e-1] == 0)
	{
		ps->ptr = BASE_PTR+(ps->nCount+i_e-2)*sizeof(int);
		ps->index_end[i_e-1] = htonl(*(int *)(ps->mmap+ps->ptr));
		/*
		fseek(ps->fp,BASE_PTR+(ps->nCount+i_e-2)*sizeof(int),SEEK_SET);
		if (1 != fread(&ps->index_end[i_e-1],sizeof(int),1,ps->fp)) return 0;
		ps->index_end[i_e-1] = htonl(ps->index_end[i_e-1]);*/
	}
	if (i_e >= 1 && ps->index_start[i_e] && ip>ps->index_start[i_e-1] && ip<=ps->index_end[i_e-1])
	{
		return i_e-1;
	}else
	{
		return 0;
	}
}

/* {{{ proto string sinaip_init(string file)
   Return a resource to open */
PHP_FUNCTION(sinaip_init)
{
	char *arg = NULL;
	int arg_len, len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}
	
	struct php_sinaip *ps;
	
	ps = (struct php_sinaip *)_php_create_sinaip();
	if (ps == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't create new sinaip handler");
		RETURN_FALSE;
	}
	
	ps->fp = fopen(arg,"rb");
	if (!ps->fp)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't load sinaip library");
		RETURN_FALSE;
	}
	
	fseek(ps->fp,1,SEEK_SET);
	if (fread(&ps->nCount,sizeof(int),1,ps->fp)!=1){
		fclose(ps->fp);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't load sinaip library, read error");
		RETURN_FALSE;
	}
	ps->nCount = htonl(ps->nCount);
	
	if (!ps->nCount)
	{
		fclose(ps->fp);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't load sinaip library");
		RETURN_FALSE;
	}
	
	ps->index_start = (unsigned int *)ecalloc(ps->nCount+1, sizeof(int));
	ps->index_end = (unsigned int *)ecalloc(ps->nCount+1, sizeof(int));
	ps->index_ptr = (unsigned int *)ecalloc(ps->nCount+1, sizeof(int));
	
	fseek(ps->fp,0,SEEK_END);
	ps->size = ftell(ps->fp);
	ps->mmap=mmap(0, ps->size, PROT_READ, MAP_SHARED, fileno(ps->fp), 0);
	
	RETVAL_RESOURCE(ps->rsrc_id);
}
/* }}} */

/* {{{ proto string sinaip_preload(resource sinaip)
   Load library index to memory */
PHP_FUNCTION(sinaip_preload)
{
	zval *arg = NULL;
	int arg_len, len;
	struct php_sinaip *ps;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &arg, &arg_len) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(ps, struct php_sinaip *, &arg, -1, PHP_SINAIP_OBJECT_TAG,le_sinaip);

	if (ps == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't access sinaip");
		RETURN_FALSE;
	}
	
	fseek(ps->fp, BASE_PTR, SEEK_SET);
	
	if (fread((ps->index_start+1), sizeof(int), ps->nCount, ps->fp) != ps->nCount) RETVAL_FALSE;
	if (fread((ps->index_end+1), sizeof(int), ps->nCount, ps->fp) != ps->nCount) RETVAL_FALSE;
	if (fread((ps->index_ptr+1), sizeof(int), ps->nCount, ps->fp) != ps->nCount) RETVAL_FALSE;
	
	int i;
	for (i=1;i<=ps->nCount;i++)
	{
		ps->index_start[i] = htonl(ps->index_start[i]);
		ps->index_end[i] = htonl(ps->index_end[i]);
		ps->index_ptr[i] = htonl(ps->index_ptr[i]);
	}
	
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto string sinaip_count(resource sinaip)
   Return ip library count */
PHP_FUNCTION(sinaip_count)
{
	zval *arg = NULL;
	int arg_len, len;
	struct php_sinaip *ps;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &arg, &arg_len) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(ps, struct php_sinaip *, &arg, -1, PHP_SINAIP_OBJECT_TAG,le_sinaip);

	if (ps == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't access sinaip");
		RETURN_FALSE;
	}
	
	RETVAL_LONG(ps->nCount);
}
/* }}} */

char *getstr(struct php_sinaip *ps, unsigned char *len)
{
	*len = ps->mmap[ps->ptr++];
	if (!*len) return NULL;
	char *data;// = (char *)emalloc(*len+1);
	data = ps->mmap+ps->ptr;
	//memcpy(data,ps->mmap+ps->ptr,*len);
	ps->ptr+=*len;
	return data;
	/*
	if (fread(len,sizeof(char),1,ps->fp)!=1) return NULL;
	if (!*len) return NULL;
	char *data = (char *)emalloc(*len+1);
	if (fread(data, sizeof(char), *len, ps->fp)!=*len) return NULL;
	data[*len] = 0;
	return data;*/
}

/* {{{ proto string sinaip_search(resource sinaip, unsigned long ip)
   Return ip library key */
PHP_FUNCTION(sinaip_search)
{
	zval *arg = NULL;
	int len;
	unsigned long ip;
	struct php_sinaip *ps;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &arg, &ip) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(ps, struct php_sinaip *, &arg, -1, PHP_SINAIP_OBJECT_TAG,le_sinaip);

	if (ps == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't access sinaip");
		RETURN_FALSE;
	}
	
	long key = sinaip_query(ps,ip);
	if (key == 0)
	{
		RETVAL_FALSE;
	}else
	{
		if (ps->index_ptr[key] == 0)
		{
			ps->ptr = BASE_PTR+(ps->nCount*2+key-1)*sizeof(int);
			ps->index_ptr[key] = htonl(*(int *)(ps->mmap+ps->ptr));
			//fseek(ps->fp,BASE_PTR+(ps->nCount*2+key-1)*sizeof(int),SEEK_SET);
			//if (1 != fread(&ps->index_ptr[key],sizeof(int),1,ps->fp)) RETVAL_FALSE;
			//ps->index_ptr[key] = htonl(ps->index_ptr[key]);
		}
		ps->ptr = ps->index_ptr[key]+4;
		/*fseek(ps->fp,ps->index_ptr[key],SEEK_SET);
		int nlen = 0;
		if (1 != fread(&nlen,sizeof(int),1,ps->fp)) RETVAL_FALSE;
		nlen = htonl(nlen);*/
		array_init(return_value);
		
		unsigned char ilen;
		char *data;
		add_assoc_long(return_value, "start", ps->index_start[key]);
		add_assoc_long(return_value, "end", ps->index_end[key]);
		data=getstr(ps,&ilen);
		add_assoc_stringl(return_value, "country", data, ilen, 1);
		data=getstr(ps,&ilen);
		add_assoc_stringl(return_value, "province", data, ilen, 1);
		data=getstr(ps,&ilen);
		add_assoc_stringl(return_value, "city", data, ilen, 1);
		data=getstr(ps,&ilen);
		add_assoc_stringl(return_value, "district", data, ilen, 1);
		data=getstr(ps,&ilen);
		add_assoc_stringl(return_value, "isp", data, ilen, 1);
		data=getstr(ps,&ilen);
		add_assoc_stringl(return_value, "type", data, ilen, 1);
		data=getstr(ps,&ilen);
		add_assoc_stringl(return_value, "desc", data, ilen, 1);
		data=getstr(ps,&ilen);
		add_assoc_stringl(return_value, "lat", data, ilen, 1);
		data=getstr(ps,&ilen);
		add_assoc_stringl(return_value, "lng", data, ilen, 1);
	}
	//RETVAL_LONG();
}
/* }}} */

/* {{{ proto string sinaip_close(resource sinaip)
   Release library resource */
PHP_FUNCTION(sinaip_close)
{
	zval *arg = NULL;
	int arg_len, len;
	struct php_sinaip *ps;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &arg, &arg_len) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(ps, struct php_sinaip *, &arg, -1, PHP_SINAIP_OBJECT_TAG,le_sinaip);

	if (ps == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't access sinaip");
		RETURN_FALSE;
	}
	
	zend_list_delete(ps->rsrc_id);
	RETURN_TRUE;
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
