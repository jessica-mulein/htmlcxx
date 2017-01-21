/*
 *  Version: 1.0
 *  Date:    2012/12/12
 *  Author:  ruzzzua[]gmail.com
 */

#ifndef __FIX_CHAR_H__
#define __FIX_CHAR_H__

#include <cctype>

inline int is_ascii_space(unsigned char ch) {  // unsigned char !!!
	// TODO Read RFC
	return isspace(ch);
	//return ch <= ' ';

	// locale independent
	//return '\t' == ch || '\f' == ch ||  '\n' == ch || '\r' == ch || '\v' == ch || ' ' == ch;
}

#endif
