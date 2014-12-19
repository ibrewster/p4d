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
#ifndef __FOURD_INT__
#define __FOURD_INT__ 1
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int Printf(const char* format,...);
int Printferr(const char* format,...);

/*******************/
/* communication.c */
/*******************/
int socket_connect(FOURD *cnx,const char *host,unsigned int port);
void socket_disconnect(FOURD *cnx);
int socket_send(FOURD *cnx,const char*msg);
int socket_send_data(FOURD *cnx,const char*msg,int len);
//int socket_receiv(FOURD *cnx);
int socket_receiv_header(FOURD *cnx,FOURD_RESULT *state);
int socket_receiv_data(FOURD *cnx,FOURD_RESULT *state);
int socket_receiv_update_count(FOURD *cnx,FOURD_RESULT *state);
int set_sock_blocking(int socketd, int block);
int socket_connect_timeout(FOURD *cnx,const char *host,unsigned int port,int timeout);
/*******************/
/* fourd_interne.c */
/*******************/
//return 0 for OK et -1 for no readable header and error_code
int dblogin(FOURD *cnx,unsigned short int id_cnx,const char *user,const char*pwd,const char*image_type);
int dblogout(FOURD *cnx,unsigned short int id_cmd);
int quit(FOURD *cnx,unsigned short int id_cmd);
//return 0 for OK et -1 for no readable header and error_code
int _query(FOURD *cnx,unsigned short int id_cmd,const char *request,FOURD_RESULT *result,const char*image_type, int res_size);
int __fetch_result(FOURD *cnx,unsigned short int id_cmd,int statement_id,int command_index,unsigned int first_row,unsigned int last_row,FOURD_RESULT *result);
int _fetch_result(FOURD_RESULT *res,unsigned short int id_cmd);
int get(const char* msg,const char* section,char *valeur,int max_length);
//FOURD_LONG8 get_status(FOURD* cnx);
//int traite_header_reponse(FOURD* cnx);
int traite_header_response(FOURD_RESULT* cnx);
FOURD_LONG8 _get_status(const char *header,int *status,FOURD_LONG8 *error_code,char *error_string);
int receiv_check(FOURD *cnx,FOURD_RESULT *state);
void _free_data_result(FOURD_RESULT *res);
//clear connection attribut
void _clear_atrr_cnx(FOURD *cnx);
int close_statement(FOURD_RESULT *res,unsigned short int id_cmd);

int _prepare_statement(FOURD *cnx,unsigned short int id_cmd,const char *request);
int _query_param(FOURD *cnx,unsigned short int id_cmd, const char *request,unsigned int nbParam, const FOURD_ELEMENT *param,FOURD_RESULT *result,const char*image_type, int res_size);
int _is_multi_query(const char *request);
int _valid_query(FOURD *cnx,const char *request);
/*********************/
/* Memory Allocation */
/*********************/
void *_copy(FOURD_TYPE type,void *org);
char *_serialize(char *data,unsigned int *size, FOURD_TYPE type, void *pObj);
void Free(void *p);
void FreeFloat(FOURD_FLOAT *p);
void FreeString(FOURD_STRING *p);
void FreeBlob(FOURD_BLOB *p);
void PrintData(const void *data,unsigned int size);
#ifndef WIN32
	void ZeroMemory (void *s, size_t n);
#define WSAGetLastError() errno
#define strtok_s(a,b,c) strtok(a,b)
#define strcpy_s(s,size,cs) strncpy(s,cs,size)
#define strncpy_s(s,ms,cs,size) strncpy(s,cs,size)
	int sprintf_s(char *buff,size_t size,const char* format,...);
	int _snprintf_s(char *buff, size_t size, size_t count, const char *format,...);
#endif


#endif
