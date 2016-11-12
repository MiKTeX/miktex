#ifndef _DEFINES_H_
#define _DEFINES_H_

enum { ENC_UTF_8 , ENC_ISO_8859_8, ENC_CP1255 };

/*******************/
/* PROJECT DEFINES */
/*******************/

#define MAX_LINE_SIZE	32768
/* Maximal size of input/output line in the text */

#define MAX_COMMAND_LEN	128
/* Maxima length of defined command */

#define ENC_DEFAULT		ENC_UTF_8
/* The default IO encoding - UTF-8 */


#define MAX_USER_TRANSLATIONS 500

/*******************/

#endif /* _DEFINES_H_ */
