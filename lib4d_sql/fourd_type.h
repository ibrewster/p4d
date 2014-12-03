/*
  +----------------------------------------------------------------------+
  | lib4D_SQL                                                            |
  +----------------------------------------------------------------------+
  | Copyright (c) 2009 The PHP Group                                     |
  +----------------------------------------------------------------------+
  |                                                                      |
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  |                                                                      |
  | Its original copy is usable under several licenses and is available  |
  | through the world-wide-web at the following url:                     |
  | http://freshmeat.net/projects/lib4d_sql                              |
  |                                                                      |
  | Unless required by applicable law or agreed to in writing, software  |
  | distributed under the License is distributed on an "AS IS" BASIS,    |
  | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or      |
  | implied. See the License for the specific language governing         |
  | permissions and limitations under the License.                       |
  +----------------------------------------------------------------------+
  | Contributed by: 4D <php@4d.fr>, http://www.4d.com                    |
  |                 Alter Way, http://www.alterway.fr                    |
  | Authors: Stephane Planquart <stephane.planquart@o4db.com>            |
  |          Alexandre Morgaut <php@4d.fr>                               |
  +----------------------------------------------------------------------+
*/
#ifndef __FOURD_TYPE__
#define __FOURD_TYPE__
typedef enum
{
	VK_UNKNOW=0,
	VK_BOOLEAN,
	VK_BYTE,
	VK_WORD,
	VK_LONG,
	VK_LONG8,
	VK_REAL,
	VK_FLOAT,
	VK_TIMESTAMP,
	VK_DURATION,
	VK_STRING,
	VK_BLOB,
	VK_IMAGE 
}FOURD_TYPE;
/******************************/
/* parse and format FOUR_TYPE */
/******************************/
FOURD_TYPE typeFromString(const char *type);
const char* stringFromType(FOURD_TYPE type);
/******************************************************************/
/* vk_sizeof                                                      */
/******************************************************************/
/* return sizeof type or -1 if varying length or 0 if unknow type */
/******************************************************************/
int vk_sizeof(FOURD_TYPE type);

/***************/
/* Result-Type */
/***************/
typedef enum
{
	UNKNOW=0,
	UPDATE_COUNT,
	RESULT_SET
}FOURD_RESULT_TYPE;
FOURD_RESULT_TYPE resultTypeFromString(const char *type);
const char* stringFromResultType(FOURD_RESULT_TYPE type);

/*********************/
/* Structure of VK_* */
/*********************/
typedef short FOURD_BOOLEAN;
typedef short FOURD_BYTE;
typedef short FOURD_WORD;
typedef	int FOURD_LONG;
typedef	__int64 FOURD_LONG8;
typedef	double FOURD_REAL;
typedef	struct{int exp;char sign;int data_length;void* data;}FOURD_FLOAT;
typedef	struct{short year;char mounth;char day;unsigned int milli;}FOURD_TIMESTAMP;
typedef	__int64 FOURD_DURATION;//in milliseconds
typedef	struct{int length;unsigned char *data;}FOURD_STRING;
typedef	struct{unsigned int length;void *data;}FOURD_BLOB;
//typedef	struct{}FOURD_IMAGE; 

#endif