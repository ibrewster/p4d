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
#ifndef __FOURD__
#define __FOURD__ 1

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Wspiapi.h>
#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <errno.h>
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#endif 


#define VERBOSE 0


/* taille maximal de 2K pour les envoi de requÕte  */
/* #define BUFFER_LENGTH 131072 */
/* taille maximal de 128K pour les r»ponse */
#define BUFFER_LENGTH 131072
#define ERROR_STRING_LENGTH 2048

#define MAX_HEADER_SIZE 2048
#define DEFAULT_IMAGE_TYPE "jpg"
#define MAX_LENGTH_COLUMN_NAME 255

#define FOURD_OK 0
#define FOURD_ERROR 1



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
	VK_TIME,
	VK_TIMESTAMP,
	VK_DURATION,
	VK_TEXT,
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
#ifdef WIN32
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
typedef	struct{int length;void *data;}FOURD_BLOB;
/* typedef	struct{}FOURD_IMAGE;  */
#else
typedef short FOURD_BOOLEAN;
typedef short FOURD_BYTE;
typedef short FOURD_WORD;
typedef int FOURD_LONG;
typedef long long FOURD_LONG8;
typedef double FOURD_REAL;
typedef struct{int exp;unsigned char sign;int data_length;void* data;}FOURD_FLOAT;
typedef struct{short year;unsigned char mounth;unsigned char day;unsigned int milli;}FOURD_TIMESTAMP;
typedef long long FOURD_DURATION;//in milliseconds
typedef struct{int length;unsigned char *data;}FOURD_STRING;
typedef struct{int length;void *data;}FOURD_BLOB;
/* typedef       struct{}FOURD_IMAGE; */

#endif


typedef struct{
	/* Socket Win32 */
#ifdef WIN32
	WSADATA wsaData;
	SOCKET socket;
#else
	int socket;
#endif

	int init;		/*boolean*/
	int connected;	/*boolean*/

	/*deprecated: use FOURD_RESULT*/
	/*char reponse[BUFFER_LENGTH];
	int reponse_len;*/

	/* status */
	int status;//1 pour OK, 0 pour KO
	FOURD_LONG8 error_code;
	char error_string[ERROR_STRING_LENGTH];

	/* updated row */
	FOURD_LONG8 updated_row;
	
	/* PREFERRED-IMAGE-TYPES */
	char *preferred_image_types;
	int timeout;

} FOURD;

typedef struct{
	FOURD_TYPE type;
	char null;//0 not null, 1 null
	void *pValue;
}FOURD_ELEMENT;

typedef struct{
	char sType[255];
	FOURD_TYPE type;
	char sColumnName[MAX_LENGTH_COLUMN_NAME];
}FOURD_COLUMN;

typedef struct{
	unsigned int nbColumn;
	FOURD_COLUMN *Column;
}FOURD_ROW_TYPE;

typedef struct{
	FOURD *cnx;
	char header[MAX_HEADER_SIZE];
	unsigned int header_size;

	/*state of statement (OK or KO)*/
	int status;	/*FOURD_OK or FOURD_ERRROR*/
	FOURD_LONG8 error_code;
	char error_string[ERROR_STRING_LENGTH];
	
	/*result of parse header
	  RESULT_SET for select
	  UPDATE_COUNT for insert, update, delete*/
	FOURD_RESULT_TYPE resultType;
	
	/*Id of statement used with 4D SQL-serveur*/
	int id_statement;
	/*Id commande use for request */
	int id_commande;
	/*updateability is true or false */
	int updateability;

	/*total of row count */
	unsigned int row_count;

	/*row count in data buffer
	  for little select, row_count_sent = row_cout
	  for big select, row_count_sent = 100 for the first result_set
	*/
	unsigned int row_count_sent;
	/*num of the first row 
	for the first response in big select
	with default parametre on serveur : 0 */
	unsigned int first_row;
	
	/* row_type of this statement 
	   containe column count, column name and column type*/
	FOURD_ROW_TYPE row_type;

	/*data*/
	FOURD_ELEMENT *elmt;

	/*current row index*/
	unsigned int numRow;
}FOURD_RESULT;

typedef struct {
	FOURD *cnx;
	char *query;	/*MAX_HEADER_SIZE is using because the query is insert into header*/
	unsigned int nb_element;
	unsigned int nbAllocElement;
	FOURD_ELEMENT *elmt;
	/* PREFERRED-IMAGE-TYPES */
	char *preferred_image_types;
}FOURD_STATEMENT;


FOURD* fourd_init();
int fourd_connect(FOURD *cnx,const char *host,const char *user,const char *password,const char *base,unsigned int port);
int fourd_close(FOURD *cnx);
int fourd_exec(FOURD *cnx,const char *query);
FOURD_LONG8 fourd_affected_rows(FOURD *cnx);
//gestion des erreurs
int fourd_errno(FOURD *cnx);
const char * fourd_error(FOURD *cnx);
const char * fourd_sqlstate(FOURD *cnx);
void fourd_free(FOURD* cnx);
void fourd_free_statement(FOURD_STATEMENT *state);
void fourd_timeout(FOURD* cnx,int timeout);

/*function on FOURD_RESULT*/
FOURD_LONG8 fourd_num_rows(FOURD_RESULT *result);
FOURD_RESULT *fourd_query(FOURD *cnx,const char *query);
int fourd_close_statement(FOURD_RESULT *res);
void fourd_free_result(FOURD_RESULT *res);

/*function for field*/
FOURD_LONG * fourd_field_long(FOURD_RESULT *res,unsigned int numCol);
FOURD_STRING * fourd_field_string(FOURD_RESULT *res,unsigned int numCol);
void * fourd_field(FOURD_RESULT *res,unsigned int numCol);
int fourd_next_row(FOURD_RESULT *res);

const char * fourd_get_column_name(FOURD_RESULT *res,unsigned int numCol);
FOURD_TYPE fourd_get_column_type(FOURD_RESULT *res,unsigned int numCol);
int fourd_num_columns(FOURD_RESULT *res);
int fourd_field_to_string(FOURD_RESULT *res,unsigned int numCol,char **value,size_t *len);

FOURD_STATEMENT * fourd_prepare_statement(FOURD *cnx,const char *query);
FOURD_STRING *fourd_create_string(char *param,int length);
int fourd_bind_param(FOURD_STATEMENT *state,unsigned int numParam,FOURD_TYPE type, void *val);
FOURD_RESULT *fourd_exec_statement(FOURD_STATEMENT *state, int res_size);

void fourd_set_preferred_image_types(FOURD* cnx,const char *types);
void fourd_set_statement_preferred_image_types(FOURD_STATEMENT *state,const char *types);
const char* fourd_get_preferred_image_types(FOURD* cnx);
const char* fourd_get_statement_preferred_image_types(FOURD_STATEMENT *state);
#endif
