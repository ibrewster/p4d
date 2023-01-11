/*
 +----------------------------------------------------------------------+
 | PHP Version 5                                                        |
 +----------------------------------------------------------------------+
 | Copyright (c) 1997-2008 The PHP Group                                |
 +----------------------------------------------------------------------+
 | This source file is subject to version 3.01 of the PHP license,      |
 | that is bundled with this package in the file LICENSE, and is        |
 | available through the world-wide-web at the following url:           |
 | http://www.php.net/license/3_01.txt                                  |
 | If you did not receive a copy of the PHP license and are unable to   |
 | obtain it through the world-wide-web, please send a note to          |
 | license@php.net so we can mail you a copy immediately.               |
 +----------------------------------------------------------------------+
 | Author: Jim Winstead <jimw@php.net>                                  |
 +----------------------------------------------------------------------+
 */

/* $Id: base64.c 287670 2009-08-25 08:35:39Z fourd $ */

#include <string.h>
#include <stdlib.h>
//#include "php.h"
#include "base64.h"

/* {{{ */
static const char base64_table[] =
	{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
	};

static const char base64_pad = '=';

static const short base64_reverse_table[256] = {
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -1, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 62, -2, -2, -2, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -2, -2, -2, -2, -2, -2,
	-2,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2, -2,
	-2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
};
/* }}} */

/* {{{ php_base64_encode */
//PHPAPI 
unsigned char *base64_encode(const char *str, size_t length, int *ret_length)
{
	const unsigned char *current = (const unsigned char*)str;
	unsigned char *p;
	unsigned char *result;

	if (((length + 2) / 3) >= (1 << (sizeof(int) * 8 - 2))) {
		if (ret_length != NULL) {
			*ret_length = 0;
		}
		return NULL;
	}

	result = (unsigned char *)malloc(((length + 2) / 3) * 4* sizeof(char)+1);
	p = result;

	while (length > 2) { /* keep going until we have less than 24 bits */
		*p++ = base64_table[current[0] >> 2];
		*p++ = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
		*p++ = base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
		*p++ = base64_table[current[2] & 0x3f];

		current += 3;
		length -= 3; /* we just handle 3 octets of data */
	}

	/* now deal with the tail end of things */
	if (length != 0) {
		*p++ = base64_table[current[0] >> 2];
		if (length > 1) {
			*p++ = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
			*p++ = base64_table[(current[1] & 0x0f) << 2];
			*p++ = base64_pad;
		} else {
			*p++ = base64_table[(current[0] & 0x03) << 4];
			*p++ = base64_pad;
			*p++ = base64_pad;
		}
	}
	if (ret_length != NULL) {
		*ret_length = (int)(p - result);
	}
	*p = '\0';
	return result;
}
/* }}} */

/* {{{ */
/* generate reverse table (do not set index 0 to 64)
static unsigned short base64_reverse_table[256];
#define rt base64_reverse_table
void php_base64_init(void)
{
	char *s = emalloc(10240), *sp;
	char *chp;
	short idx;

	for(ch = 0; ch < 256; ch++) {
		chp = strchr(base64_table, ch);
		if(ch && chp) {
			idx = chp - base64_table;
			if (idx >= 64) idx = -1;
			rt[ch] = idx;
		} else {
			rt[ch] = -1;
		}
	}
	sp = s;
	sprintf(sp, "static const short base64_reverse_table[256] = {\n");
	for(ch =0; ch < 256;) {
		sp = s+strlen(s);
		sprintf(sp, "\t% 3d,% 3d,% 3d,% 3d,% 3d,% 3d,% 3d,% 3d,% 3d,% 3d,% 3d,% 3d,% 3d,% 3d,% 3d,% 3d,\n", rt[ch+0], rt[ch+1], rt[ch+2], rt[ch+3], rt[ch+4], rt[ch+5], rt[ch+6], rt[ch+7], rt[ch+8], rt[ch+9], rt[ch+10], rt[ch+11], rt[ch+12], rt[ch+13], rt[ch+14], rt[ch+15]);
		ch += 16;
	}
	sprintf(sp, "};");
	php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Reverse_table:\n%s", s);
	efree(s);
}
*/
/* }}} */

//PHPAPI 
unsigned char *base64_decode(const char *str, size_t length, int *ret_length)
{
	return base64_decode_ex(str, length, ret_length, 0);
}

/* {{{ php_base64_decode */
/* as above, but backwards. :) */
//PHPAPI 
unsigned char *base64_decode_ex(const char *str, size_t length, int *ret_length, int strict)
{
	const char *current = str;
	int ch, i = 0, j = 0, k;
	/* this sucks for threaded environments */
	unsigned char *result;
	
	result = (unsigned char *)malloc(length + 1);

	/* run through the whole string, converting as we go */
	while ((ch = *current++) != '\0' && length-- > 0) {
		if (ch == base64_pad) break;

		ch = base64_reverse_table[ch];
		if ((!strict && ch < 0) || ch == -1) { /* a space or some other separator character, we simply skip over */
			continue;
		} else if (ch == -2) {
			free(result);
			result=NULL;
			return NULL;
		}

		switch(i % 4) {
		case 0:
			result[j] = ch << 2;
			break;
		case 1:
			result[j++] |= ch >> 4;
			result[j] = (ch & 0x0f) << 4;
			break;
		case 2:
			result[j++] |= ch >>2;
			result[j] = (ch & 0x03) << 6;
			break;
		case 3:
			result[j++] |= ch;
			break;
		}
		i++;
	}

	k = j;
	/* mop things up if we ended on a boundary */
	if (ch == base64_pad) {
		switch(i % 4) {
		case 1:
			free(result);
			result=NULL;
			return NULL;
		case 2:
			k++;
		case 3:
			result[k++] = 0;
		}
	}
	if(ret_length) {
		*ret_length = j;
	}
	result[j] = '\0';
	return result;
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
