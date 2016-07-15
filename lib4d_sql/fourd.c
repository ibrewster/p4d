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

#include "fourd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fourd.h"
#include "fourd_int.h"

FOURD* fourd_init()
{
	//int iResult=0;
	FOURD* cnx=calloc(1,sizeof(FOURD));

	cnx->socket = INVALID_SOCKET;
	cnx->connected=0;
	cnx->init=0;
	#ifdef WIN32
	// Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &cnx->wsaData);
    if (iResult != 0) {
        Printf("WSAStartup failed: %d\n", iResult);
        return NULL;
    }
  #endif
	cnx->init=1;
	fourd_set_preferred_image_types(cnx,DEFAULT_IMAGE_TYPE);
	return cnx;
}

int fourd_connect(FOURD *cnx,const char *host,const char *user,const char *password,const char *base,unsigned int port)
{
	if(!cnx->init)
	{
		//not init
		Printferr("Erreur: FOURD object did not initialised\n");
		cnx->error_code=-1;
		strncpy_s(cnx->error_string,2048,"FOURD object did not initialised",2048);
		return 1;
	}
	if(cnx->connected)
	{
		//deja connecter
		Printferr("Erreur: already connected\n");
		cnx->error_code=-1;
		strncpy_s(cnx->error_string,2048,"Already connected",2048);
		return 1;
	}
	if(socket_connect_timeout(cnx,host,port,15))
	{
		//erreur de connection
		Printferr("Erreur in socket_connect\n");
		cnx->connected=0;
		//cnx->error_code=-1;
		//strncpy_s(cnx->error_string,2048,"Error during connection",2048);
		return 1;
	}
	if(dblogin(cnx,1,user,((password==NULL)?"":password),cnx->preferred_image_types)!=0)
	{
		//erreur de login
		Printferr("Erreur: in login function\n");
		cnx->connected=0;
		if(cnx->error_code==0) {
			cnx->error_code=-1;
			strncpy_s(cnx->error_string,2048,"Error during login",2048);
		}
		return 1;
	}
	cnx->connected=1;
	//Printferr("Erreur: not erreur\n");
	cnx->error_code=0;
	strncpy_s(cnx->error_string,2048,"",2048);
	return 0;
}
int fourd_close(FOURD *cnx)
{
	if(dblogout(cnx,4)!=0)
		return 1;
	if(quit(cnx,5)!=0)
		return 1;
	socket_disconnect(cnx);
	return 0;
}
void fourd_free(FOURD* cnx)
{
#ifdef WIN32
	WSACleanup();
#endif
	if (cnx->preferred_image_types!=NULL){
		free(cnx->preferred_image_types);
		cnx->preferred_image_types=NULL;
	}

	if (cnx!=NULL) {
		free(cnx);
		cnx=NULL;
	}
}


FOURD_LONG8 fourd_affected_rows(FOURD *cnx)
{
	return cnx->updated_row;
}
int fourd_errno(FOURD *cnx)
{
	return (int)cnx->error_code;
}
const char * fourd_error(FOURD *cnx)
{
	return cnx->error_string;
}
int fourd_exec(FOURD *cnx,const char *query)
{
	return _query(cnx,3,query,NULL,cnx->preferred_image_types,100);
}
FOURD_RESULT* fourd_query(FOURD *cnx,const char *query)
{
	FOURD_RESULT* result;
	
	result=calloc(1,sizeof(FOURD_RESULT));
	result->cnx=cnx;
	if(_query(cnx,3,query,result,cnx->preferred_image_types,100)==0)
	{
		result->numRow=-1;
		return result;
	}
	else
	{
		fourd_free_result(result);
		return NULL;
	}
}
void fourd_free_result(FOURD_RESULT *res)
{
	if(res!=NULL && res->elmt!=NULL)
			_free_data_result(res);
	Free(res->row_type.Column);
	Free(res);
}
int fourd_next_row(FOURD_RESULT *res)
{
	res->numRow++;
	if(res->numRow>=res->row_count)
		return 0;	/*error*/
	if(res->numRow > res->first_row+res->row_count_sent-1) { /*row out of local result_set but in serveur statement */
		if(_fetch_result(res,123))
			return 0;
	}
	return 1;
}
int fourd_close_statement(FOURD_RESULT *res)
{
	if(close_statement(res,7)!=0)
		return 1;
	return 0;
}
FOURD_LONG * fourd_field_long(FOURD_RESULT *res,unsigned int numCol)
{
	unsigned int nbCol=res->row_type.nbColumn;
	//unsigned int nbRow=res->row_count;
	FOURD_ELEMENT *elmt;
	unsigned int indexElmt=0;	/*index of element in table <> numRow*nbCol + numCol */
	//if(res->numRow>=nbRow)	//what can is do in this case...
	indexElmt=(res->numRow-res->first_row)*nbCol+numCol;
	elmt=&(res->elmt[indexElmt]);
	if(elmt->null==0)
	{
		//FOURD_LONG x=*((int *)elmt->pValue);
		//printf("/////%d//////",x);
		return (FOURD_LONG *)elmt->pValue;
	}
	return 0;
}
FOURD_STRING * fourd_field_string(FOURD_RESULT *res,unsigned int numCol)
{
	int nbCol=res->row_type.nbColumn;
	//int nbRow=res->row_count;
	unsigned int indexElmt=0;	/*index of element in table <> numRow*nbCol + numCol */
	FOURD_ELEMENT *elmt=NULL;
	//if(res->numRow>=nbRow)	//what can is do in this case...
	//	return NULL;
	indexElmt=(res->numRow-res->first_row)*nbCol+numCol;
	elmt=&(res->elmt[indexElmt]);
	if(elmt->null==0)
	{
		//FOURD_STRING x=*((int *)elmt->pValue);
		//printf("/////%d//////",x);
		FOURD_STRING *x=(FOURD_STRING *)elmt->pValue;
		return x;
	}
	return NULL;
}
void * fourd_field(FOURD_RESULT *res,unsigned int numCol)
{
	unsigned int nbCol=res->row_type.nbColumn;
	unsigned int nbRow=res->row_count;
	unsigned int indexElmt=0;	/*index of element in table <> numRow*nbCol + numCol */
	FOURD_ELEMENT *elmt=NULL;

	if(res->numRow>=nbRow){	
		res->cnx->error_code=-1;
		sprintf_s(res->cnx->error_string,2048,"num Row out of bounds",2048);
		return NULL;
	}
	if(numCol>=nbCol){
		res->cnx->error_code=-1;
		sprintf_s(res->cnx->error_string,2048,"num Column out of bounds",2048);
		return NULL;
	}
	indexElmt=(res->numRow-res->first_row)*nbCol+numCol;

	elmt=&(res->elmt[indexElmt]);
	if(elmt->null!=0) {	/*if elmt is null*/
		return NULL;
	}
	return elmt->pValue;
}

int fourd_field_to_string(FOURD_RESULT *res,unsigned int numCol,char **value,size_t *len)
{
	unsigned int nbCol=res->row_type.nbColumn;
	unsigned int nbRow=res->row_count;
	FOURD_ELEMENT *elmt=NULL;
	unsigned int indexElmt=0;	/*index of element in table <> numRow*nbCol + numCol */
	if(res->numRow>=nbRow){	
		*value=NULL;
		*len=0;
		res->cnx->error_code=-1;
		sprintf_s(res->cnx->error_string,2048,"num Row out of bounds",2048);
		return 0;
	}
	if(numCol>=nbCol){
		*value=NULL;
		*len=0;
		res->cnx->error_code=-1;
		sprintf_s(res->cnx->error_string,2048,"num Column out of bounds",2048);
		return 0;
	}
	indexElmt=(res->numRow-res->first_row)*nbCol+numCol;
	elmt=&(res->elmt[indexElmt]);
	if(elmt->null!=0) {	/*if elmt is null*/
		*value=NULL;
		*len=0;
	}
	else {
		switch(elmt->type) {
		case VK_BOOLEAN:
			{
				*value=calloc(2,sizeof(char));
				sprintf_s(*value,2,"%s",(*((FOURD_BOOLEAN *)elmt->pValue)==0?"1":"0"));
				*len=strlen(*value);
				return 1;
			}
		case VK_BYTE:
		case VK_WORD:
		case VK_LONG:
		case VK_LONG8:
		case VK_DURATION:
			{
				*value=calloc(22,sizeof(char));
				sprintf_s(*value,22,"%d",*((FOURD_LONG *)elmt->pValue));
				*len=strlen(*value);
				return 1;
			}
			break;
		case VK_REAL:
			{
				*value=calloc(64,sizeof(char));	
				sprintf_s(*value,64,"%lf",*((FOURD_REAL *)elmt->pValue));
				*len=strlen(*value);
				return 1;
			}
			break;
		case VK_FLOAT:
			//Varying length
			return 0;
			break;
		case VK_TIME:
		case VK_TIMESTAMP:
			{
				FOURD_TIMESTAMP *t=elmt->pValue;
				unsigned int h,m,s,milli;
				milli=t->milli;
				h=milli/(60*60*1000);
				milli-=h*(60*60*1000);
				m=milli/(60*1000);
				milli-=m*(60*1000);
				s=milli/(1000);
				milli-=s*(1000);

				*value=calloc(24,sizeof(char));	
				sprintf_s(*value,24,"%0.4d/%0.2d/%0.2d %0.2d:%0.2d:%0.2d.%0.3d",t->year,t->mounth,t->day,h,m,s,milli);
				*len=strlen(*value);
				return 1;
			}
		case VK_STRING:
			{
				FOURD_STRING *str=elmt->pValue;
				int size=0;
				*value=NULL;
				size=str->length;
				*value=calloc(size,2);	/*2 bytes per char*/
				memcpy(*value,str->data,str->length*2);
				*len=str->length*2;
				return 1;
			}
		case VK_BLOB:
		case VK_IMAGE:
			//Varying length
			return 0;
			break;
		default:
				return 0; //since this is what would happen if it just fell out of the switch statement anyway.
				break;
		}
		return 0;
	} 
	return 0;
}

const char * fourd_get_column_name(FOURD_RESULT *res,unsigned int numCol)
{
	unsigned int nbCol=res->row_type.nbColumn;
	if(numCol>=nbCol)
		return "";
	if(res->row_type.Column==NULL)
		return "";
	return res->row_type.Column[numCol].sColumnName;
}

FOURD_TYPE fourd_get_column_type(FOURD_RESULT *res,unsigned int numCol)
{
	unsigned int nbCol=res->row_type.nbColumn;
	FOURD_TYPE type=VK_UNKNOW;
	if(numCol>=nbCol)
		return 0;
	if(res->row_type.Column==NULL)
		return 0;
	type=res->row_type.Column[numCol].type;
	return type;
}

int fourd_num_columns(FOURD_RESULT *res)
{
	return res->row_type.nbColumn;
}

void fourd_free_statement(FOURD_STATEMENT *state){
	if (state->query!=NULL){
		free(state->query);
		state->query=NULL;
	}
	
	if(state->elmt!=NULL){
		free(state->elmt);
		state->elmt=NULL;
	}
	
	if (state->preferred_image_types!=NULL){
		free(state->preferred_image_types);
		state->preferred_image_types=NULL;
	}
	
	Free(state);
}

FOURD_STATEMENT * fourd_prepare_statement(FOURD *cnx,const char *query)
{
	FOURD_STATEMENT* state=NULL;
	if(cnx==NULL || !cnx->connected || query==NULL)
		return NULL;
	
	//if(_prepare_statement(cnx, 3, query)!=0)
	//	return NULL;
	
	//try to prepare the statement. If it doesn't work, oh well.
	_prepare_statement(cnx, 3, query);
	
	state=calloc(1,sizeof(FOURD_STATEMENT));
	state->cnx=cnx;
	state->query=(char *)malloc(strlen(query)+1);

	/* allocate arbitrarily five elements in this table */
	state->nbAllocElement=5;
	state->elmt=calloc(state->nbAllocElement,sizeof(FOURD_ELEMENT));	
	state->nb_element=0;
	
	/* copy query into statement */
	sprintf(state->query,"%s",query);
	fourd_set_statement_preferred_image_types(state,cnx->preferred_image_types);
	
	return state;
}

FOURD_STRING *fourd_create_string(char *param,int length){
	//length is the character length of the string. Byte length is twice that due to UTF-16LE encoding
	FOURD_STRING *cp=NULL;
	cp=calloc(1,sizeof(FOURD_STRING));
	cp->data=calloc(length,2);	/* 2 bytes per char */
	cp->length=length;
	memcpy(cp->data,param,length*2);  /* 2 bytes per char */
	
	return cp;
}

int fourd_bind_param(FOURD_STATEMENT *state,unsigned int numParam,FOURD_TYPE type, void *val)
{
	/* realloc the size of memory if necessary */
	if(numParam>=state->nbAllocElement) {
		state->nbAllocElement=numParam+5;
		state->elmt=realloc(state->elmt,(sizeof(FOURD_ELEMENT)*state->nbAllocElement));
	}
	if(numParam>=state->nb_element) {
		state->nb_element=numParam+1;	/*zero-based index */
	}
	state->elmt[numParam].type=type;
	if(val==NULL) {
		state->elmt[numParam].null=1;
		state->elmt[numParam].pValue=NULL;
	}
	else {
		state->elmt[numParam].null=0;
		state->elmt[numParam].pValue=_copy(type,val);
	}
	return 0;
}
FOURD_RESULT *fourd_exec_statement(FOURD_STATEMENT *state, int res_size)
{
	FOURD_RESULT *result=NULL;
	result=calloc(1,sizeof(FOURD_RESULT));
	result->cnx=state->cnx;
	if(_query_param(state->cnx,6,state->query,state->nb_element,state->elmt,result,state->preferred_image_types,res_size)==0)
	{
		result->numRow=-1;
		return result;
	}
	else
	{
		fourd_free_result(result);
		return NULL;
	}
}
void fourd_set_preferred_image_types(FOURD* cnx,const char *types)
{
	if(cnx->preferred_image_types)	{
		Free(cnx->preferred_image_types);
	}
	if(types)	{
		cnx->preferred_image_types=malloc(strlen(types)+1);
		sprintf_s(cnx->preferred_image_types,strlen(types)+1,"%s",types);
	}
	else	{
		cnx->preferred_image_types=NULL;
	}

}
void fourd_set_statement_preferred_image_types(FOURD_STATEMENT *state,const char *types)
{
	if(state->preferred_image_types)	{
		Free(state->preferred_image_types);
	}
	if(types)	{
		state->preferred_image_types=malloc(strlen(types)+1);
		sprintf_s(state->preferred_image_types,strlen(types)+1,"%s",types);
	}
	else	{
		state->preferred_image_types=NULL;
	}
}
const char* fourd_get_preferred_image_types(FOURD* cnx)
{
	return cnx->preferred_image_types;
}
const char* fourd_get_statement_preferred_image_types(FOURD_STATEMENT *state)
{
	return state->preferred_image_types;
}
void fourd_timeout(FOURD* cnx,int timeout)
{
	cnx->timeout=timeout;
}