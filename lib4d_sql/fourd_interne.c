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
#include "fourd_int.h"
#include "base64.h"
#include "utils.h"
#include <stdarg.h>
#define __STATEMENT_BASE64__ 1
#define __LOGIN_BASE64__ 1
int Printf(const char* format,...)
{
#if VERBOSE
	va_list ap;
	va_start(ap,format);
	vprintf(format,ap);
	
	return 0;
#else
	return 0;
#endif
}
int Printferr(const char* format,...)
{
#if VERBOSE
	va_list ap;
	va_start(ap,format);
	vfprintf(stderr,format,ap);
	
	return 0;
#else
	return 0;
#endif
}
#ifndef WIN32
void ZeroMemory (void *s, size_t n)
{
	bzero(s,n);
}
int sprintf_s(char *buff,size_t size,const char* format,...)
{
	va_list ap;
	va_start(ap,format);
	vsnprintf(buff,size,format,ap);
	return 0;
}
int _snprintf_s(char *buff, size_t size, size_t count, const char *format,...)
{
	va_list ap;
	va_start(ap,format);
	vsnprintf(buff,((size>count)?count:size),format,ap);
	return 0;
}
int _snprintf(char *buff, int size, const char *format,...)
{
	va_list ap;
	va_start(ap,format);
	vsnprintf(buff,size,format,ap);
	return 0;
}
#endif
int dblogin(FOURD *cnx,unsigned short int id_cnx,const char *user,const char*pwd,const char*image_type)
{
	char msg[2048];
	FOURD_RESULT state;
	unsigned char *user_b64=NULL,*pwd_b64=NULL;
	int len;
	_clear_atrr_cnx(cnx);
#if __LOGIN_BASE64__
	user_b64=base64_encode(user,strlen(user),&len);
	pwd_b64=base64_encode(pwd,strlen(pwd),&len);
	sprintf_s(msg,2048,"%03d LOGIN \r\nUSER-NAME-BASE64:%s\r\nUSER-PASSWORD-BASE64:%s\r\nPREFERRED-IMAGE-TYPES:%s\r\nREPLY-WITH-BASE64-TEXT:Y\r\nPROTOCOL-VERSION:0.1a\r\n\r\n",id_cnx,user_b64,pwd_b64,image_type);
	Free(user_b64);
	Free(pwd_b64);
#else
	sprintf_s(msg,2048,"%03d LOGIN \r\nUSER-NAME:%s\r\nUSER-PASSWORD:%s\r\nPREFERRED-IMAGE-TYPES:%s\r\nREPLY-WITH-BASE64-TEXT:Y\r\nPROTOCOL-VERSION:0.1a\r\n\r\n",id_cnx,user,pwd,image_type);
#endif
	socket_send(cnx,msg);
	if(receiv_check(cnx,&state)!=0)
		return 1;
		
	return 0;
}
//return 0 if ok 1 if error
int _query(FOURD *cnx,unsigned short int id_cmd,const char *request,FOURD_RESULT *result,const char*image_type, int res_size)
{
	char *msg=NULL;
	FOURD_RESULT *res=NULL;
	unsigned char *request_b64=NULL;
	int len;
	Printf("---Debut de _query\n");
	_clear_atrr_cnx(cnx);
	if(!_valid_query(cnx,request)) {
		return 1;
	}
	if(result!=NULL)
		res=result;
	else
		res=calloc(1,sizeof(FOURD_RESULT));
#if __STATEMENT_BASE64__
	request_b64=base64_encode(request,strlen(request),&len);
	char *format_str="%03d EXECUTE-STATEMENT\r\nSTATEMENT-BASE64:%s\r\nOutput-Mode:%s\r\nFIRST-PAGE-SIZE:%i\r\nPREFERRED-IMAGE-TYPES:%s\r\n\r\n";
	size_t buff_size=strlen(format_str)+strlen((const char *)request_b64)+42; //add some extra for the additional arguments and a bit more for good measure.
	msg=(char *)malloc(buff_size);
	snprintf(msg,buff_size,format_str,id_cmd,request_b64,"release",res_size,image_type);
	Free(request_b64);
#else
	char *format_str="%03d EXECUTE-STATEMENT\r\nSTATEMENT:%s\r\nOutput-Mode:%s\r\nFIRST-PAGE-SIZE:%i\r\nPREFERRED-IMAGE-TYPES:%s\r\n\r\n";
	size_t buff_size=strlen(format_str)+strlen(request)+42; //add some extra for the additional arguments and a bit more for good measure.
	msg=(char *)malloc(buff_size);
	snprintf(msg, buff_size,format_str,id_cmd,request,"release",res_size,image_type);
#endif

	cnx->updated_row=-1;
	socket_send(cnx,msg);
	Free(msg);
	
	if(receiv_check(cnx,res)!=0)
		return 1;

	switch(res->resultType)	{
	case UPDATE_COUNT:
		//get Update-count: Nb row updated
		cnx->updated_row=-1;
		socket_receiv_update_count(cnx,res);		
		_free_data_result(res);
		break;
	case RESULT_SET:
		//get data
		socket_receiv_data(cnx,res);
		cnx->updated_row=-1;
		if(result==NULL) {
			_free_data_result(res);
		}
		break;
	default:
		Printferr("Error: Result-Type not supported in query");
	}
	//if(traite_header_reponse(cnx)!=0)
	//	return 1;
	if(result==NULL) {
		Free(res);
	}
	Printf("---Fin de _query\n");
	return 0;
}

int _prepare_statement(FOURD *cnx,unsigned short int id_cmd,const char *request){
	char *msg=NULL;
	FOURD_RESULT *res=calloc(1,sizeof(FOURD_RESULT));
	int len;
	
#if __STATEMENT_BASE64__
	unsigned char *request_b64=NULL;
	request_b64=base64_encode(request,strlen(request),&len);
	char *format_str="%03d PREPARE-STATEMENT\r\nSTATEMENT-BASE64: %s\r\n\r\n";
	unsigned long buff_size=strlen(format_str)+strlen((const char *)request_b64)+2; //add some extra for good measure.
	msg=(char *)malloc(buff_size);
	snprintf(msg,buff_size,format_str,id_cmd,request_b64);
	Free(request_b64);
#else
	char *format_str="%03d PREPARE-STATEMENT\r\nSTATEMENT: %s\r\n\r\n";
	unsigned long buff_size=strlen(format_str)+strlen(request)+2; //add some extra for good measure.
	msg=(char *)malloc(buff_size);
	snprintf(msg,buff_size,format_str,id_cmd,request_b64);
#endif
	
	cnx->updated_row=-1;
	socket_send(cnx,msg);
	Free(msg);
	
	if(receiv_check(cnx,res)!=0)
		return 1;
	
	switch(res->resultType)	{
		case UPDATE_COUNT:
			//get Update-count: Nb row updated
			cnx->updated_row=-1;
			//socket_receiv_update_count(cnx,res);
			_free_data_result(res);
			break;
		case RESULT_SET:
			//get data
			socket_receiv_data(cnx,res);
			cnx->updated_row=-1;
			break;
		default:
			Printferr("Error: Result-Type not supported in query");
	}
	fourd_free_result(res);

	return 0;
}

int _query_param(FOURD *cnx,unsigned short int id_cmd, const char *request,unsigned int nbParam, const FOURD_ELEMENT *param,FOURD_RESULT *result,const char*image_type,int res_size)
{
	char *msg=NULL;
	FOURD_RESULT *res=NULL;
	unsigned char *request_b64=NULL;
	int len;
	char *sParam=NULL;
	unsigned int i=0;
	char *data=NULL;
	unsigned int data_len=0;
	unsigned int size=0;
	//Printf("---Debut de _query_param\n");
	if(!_valid_query(cnx,request)) {
		return 1;
	}
	if(nbParam<=0)
		return _query(cnx,id_cmd,request,result,image_type,res_size);
	_clear_atrr_cnx(cnx);

	if(result!=NULL)
		res=result;
	else
		res=calloc(1,sizeof(FOURD_RESULT));
	

	/* construct param list */
	size_t paramlen=(nbParam+1)*13; //the longest type name is 12 characters, and we add a space between each parameter.
									// add a 1 to the number of parameters because I am paranoid.
	
	sParam=calloc(paramlen, sizeof(char)); //initalized to zero, so we should be able to call strlen() on it without problem
	
	for(i=0;i<nbParam;i++) {
		snprintf(sParam+strlen(sParam),paramlen-1-strlen(sParam)," %s",stringFromType(param[i].type));
		
		/* construct data */
		if(param[i].null==0) {
			data=realloc(data,++size);
			memset(data+(size-1),'1',1);
			data=_serialize(data,&size,param[i].type,param[i].pValue);
		} else {
			Printf("Serialize a null value\n");
			data=realloc(data,++size);
			memset(data+(size-1),'0',1);
		}
	}

	data_len=size;
	/* construct Header */
#if __STATEMENT_BASE64__
	request_b64=base64_encode(request,strlen(request),&len);
	char *msg_format="%03d EXECUTE-STATEMENT\r\nSTATEMENT-BASE64:%s\r\nOutput-Mode:%s\r\nFIRST-PAGE-SIZE:%i\r\nPREFERRED-IMAGE-TYPES:%s\r\nPARAMETER-TYPES:%s\r\n\r\n";
	size_t msg_length=strlen((const char *)request_b64)+strlen(msg_format)+strlen(image_type)+strlen(sParam)+20;
	msg=malloc(msg_length);
	snprintf(msg,msg_length,msg_format,id_cmd,request_b64,"release",res_size,image_type,sParam);
	Free(request_b64);
#else
	char *msg_format="%03d EXECUTE-STATEMENT\r\nSTATEMENT:%s\r\nOutput-Mode:%s\r\nFIRST-PAGE-SIZE:%i\r\nPREFERRED-IMAGE-TYPES:%s\r\nPARAMETER-TYPES:%s\r\n\r\n";
	size_t msg_length=strlen(request)+strlen(msg_format)+strlen(image_type)+strlen(sParam)+20;
	msg=malloc(msg_length);
	snprintf(msg,msg_length,msg_format,id_cmd,request,"release",res_size,image_type,sParam);
#endif
	
	Free(sParam);

	socket_send(cnx,msg);
	Free(msg);
	socket_send_data(cnx,data,data_len);
	if(receiv_check(cnx,res)!=0)
		return 1;

	switch(res->resultType)	{
	case UPDATE_COUNT:
		//get Update-count: Nb row updated
		socket_receiv_update_count(cnx,res);		
		_free_data_result(res);
		break;
	case RESULT_SET:
		//get data
		socket_receiv_data(cnx,res);
		cnx->updated_row=-1;
		if(result==NULL) {
			_free_data_result(res);
		}
		break;
	default:
		Printferr("Error: Result-Type not supported in query");
	}
	//if(traite_header_reponse(cnx)!=0)
	//	return 1;
	if(result==NULL)
		Free(res);
	return 0;
}

/* low level commande 
   command_index and statement_id is identify by result of execute statement commande */
int __fetch_result(FOURD *cnx,unsigned short int id_cmd,int statement_id,int command_index,unsigned int first_row,unsigned int last_row,FOURD_RESULT *result)
{
	char msg[2048];
	
	
	_clear_atrr_cnx(cnx);

	if(result==NULL) {
		return 0;
	}
	sprintf_s(msg,2048,"%03d FETCH-RESULT\r\nSTATEMENT-ID:%d\r\nCOMMAND-INDEX:%03d\r\nFIRST-ROW-INDEX:%d\r\nLAST-ROW-INDEX:%d\r\nOutput-Mode:%s\r\n\r\n",id_cmd,statement_id,command_index,first_row,last_row,"release");
	socket_send(cnx,msg);
	if(receiv_check(cnx,result)!=0)
		return 1;
	socket_receiv_data(cnx,result);

	return 0;
}
/*get next row set in result_set*/
int _fetch_result(FOURD_RESULT *res,unsigned short int id_cmd)
{
	FOURD *cnx=res->cnx;
	FOURD_RESULT *nRes=NULL;
	void *last_data=NULL;
	//int id_statement=res->id_statement;
	unsigned int first_row=res->first_row+res->row_count_sent;
	unsigned int last_row=res->first_row+res->row_count_sent+99;
	if(last_row>=res->row_count) {
		last_row=res->row_count-1;
	}

	nRes=calloc(1,sizeof(FOURD_RESULT));
	_clear_atrr_cnx(cnx);
	/*set paramature unsed in socket_receiv */
	nRes->first_row=first_row;
	nRes->row_count_sent=last_row-first_row+1;
	nRes->cnx=res->cnx;
	nRes->row_type=res->row_type;
	nRes->updateability=res->updateability;
	/*get new Result set in new FOURD_RESULT*/
	if(__fetch_result(cnx,123,res->id_statement,0,first_row,last_row,nRes)){
		return 1;
	}
	/*switch data between res and nRes FOURD_RESULT*/
	last_data=res->elmt;
	res->elmt=nRes->elmt;
	nRes->elmt=last_data;	/*important for free memory after */
	res->first_row=first_row;
	res->row_count_sent=last_row-first_row+1;
	res->error_code=nRes->error_code;
	last_data=res->error_string;
	sprintf_s(res->error_string,sizeof(res->error_string),"%s",nRes->error_string);
	res->status=nRes->status;
	

	/*free memory */
	_free_data_result(nRes);
	Free(nRes);
	
	return 0;

}
int close_statement(FOURD_RESULT *res,unsigned short int id_cmd)
{
	char msg[2048];	
	FOURD *cnx=NULL;
	FOURD_RESULT state;

	if(res==NULL)
		return 0;
	cnx=res->cnx;
	_clear_atrr_cnx(cnx);
	sprintf_s(msg,2048,"%03d CLOSE-STATEMENT\r\nSTATEMENT-ID:%d\r\n\r\n",id_cmd,res->id_statement);
	socket_send(cnx,msg);
	if(receiv_check(cnx,&state)!=0) {
		return 1;
	}
	return 0;
}
//return 0 if ok 1 if error
int dblogout(FOURD *cnx,unsigned short int id_cmd)
{
	char msg[2048];
	FOURD_RESULT state;
	_clear_atrr_cnx(cnx);
	sprintf_s(msg,2048,"%03d LOGOUT\r\n\r\n",id_cmd);
	socket_send(cnx,msg);
	if(receiv_check(cnx,&state)!=0) {
		return 1;
	}
	return 0;
}
int quit(FOURD *cnx,unsigned short int id_cmd)
{
	char msg[2048];
	FOURD_RESULT state;
	_clear_atrr_cnx(cnx);
	sprintf_s(msg,2048,"%03d QUIT\r\n\r\n",id_cmd);
	socket_send(cnx,msg);
	if(receiv_check(cnx,&state)!=0) {
		return 1;
	}
	return 0;
}
int get(const char* msg,const char* section,char *valeur,int max_length)
{
	char *loc=NULL;
	char *fin=NULL;
	loc=strstr(msg,section);
	if(loc==NULL) {		
		//printf("SECTION NON TROUVEE\n");
		return -1;
	}
	loc+=strlen(section);
	loc=strstr(loc,":");
	if(loc==NULL) {
		//printf("PAS DE : APRES LA SECTION\n");
		return -1;
	}
	loc++;
	fin=strstr(loc,"\n");
	if(fin==NULL) {
		//printf("PAS DE FIN DE LIGNE\n");
		return -1;
	}
	if(*(fin-1)=='\r') {
		//Printf("IL Y A CRLF\n");
		#ifdef WIN32
			fin--;
		#endif
	}
	
	_snprintf_s(valeur,max_length,fin-loc,"%s",loc);
	valeur[fin-loc]=0;
	//printf("La section %s contient '%s'\n",section,valeur);
	if(strstr(section,"-Base64")!=NULL) {
		//decode la valeur
		unsigned char *valeur_decode=NULL;
		int len_dec=0;
		valeur_decode=base64_decode(valeur,strlen(valeur),&len_dec);
		valeur_decode[len_dec]=0;
		strncpy_s(valeur,max_length,(const char*)valeur_decode,(size_t)len_dec);
		valeur[len_dec]=0;
		Free(valeur_decode);
	}
	return 0;
}
FOURD_LONG8 _get_status(const char *header,int *status, FOURD_LONG8 *error_code,char *error_string)
{
	char *loc=NULL,*fin=NULL,sStatus[50];
	*status=FOURD_ERROR;
	loc=strstr(header," ");
	if(loc==NULL) {
		return -1;
	}
	loc++;
	fin=strstr(loc,"\n");
	if(fin==NULL) {
		return -1;
	}
	if(*(fin-1)=='\r') {
		#ifdef WIN32
		fin--;
		#endif
	}
	_snprintf_s(sStatus,50,fin-loc,"%s",loc);
	status[fin-loc]=0;
	if(strcmp(sStatus,"OK")==0) {
		//it's ok
		*error_code=0;
		error_string[0]=0;
		*status=FOURD_OK;
		return 0;
	}
	else {
		//there is an error
		*status=FOURD_ERROR;
		{
			char error[50];
			get(header,"Error-Code",error,50);
			*error_code=atoi(error);
		}
		get(header,"Error-Description",error_string,ERROR_STRING_LENGTH);
		return *error_code;
	}
	return -1;
}


void _alias_str_replace(char *list_alias)
{
	char *loc=list_alias;
	char *locm=NULL;
	while((loc=strstr(loc,"] ["))!=NULL) {
		if((loc-list_alias)>1) {
			locm=loc;
			locm--;
			if(locm[0]!=']') {
				loc[1]='\r';
			}
			else {
				loc++;
			}
		}
		else {
			loc[1]='\r';
		}
	}
}
int traite_header_response(FOURD_RESULT* state)
{
	char *header=state->header;
	FOURD_LONG8 ret_get_status=0;
	//get status in the header
	state->elmt=0;
	ret_get_status=_get_status(state->header,&(state->status),&(state->error_code),state->error_string);
	if(ret_get_status<0) {	
		//Technical error in parse header status
		return 1;
	}
	else if(ret_get_status>0) {
		//The header is error-header
		//nothing to do with error-header
		return 1;
	}
	//The header is ok-header
	//get Column-Count
	{
		char column_count[250];
		if(get(header,"Column-Count",column_count,250)==0) {
			state->row_type.nbColumn=atoi(column_count);
			//memory allocate for column name and column type
			state->row_type.Column=calloc(state->row_type.nbColumn,sizeof(FOURD_COLUMN));
			Printf("Column-Count:%d\n",state->row_type.nbColumn);
		}
	}
	//get Column-Types
	{
		char column_type[2048];
		char *column=NULL;
		unsigned int num=0;
		//char *context=NULL;
		if(get(header,"Column-Types",column_type,2048)==0) {
			Printf("Column-Types => '%s'\n",column_type);
			column = strtok_s(column_type, " ",&context);
			if(column!=NULL)
			do{
				Printf("Column %d: %s (%s)\n",num+1,column,stringFromType(typeFromString(column)));
				if(num<state->row_type.nbColumn) {
					state->row_type.Column[num].type=typeFromString(column);
					strncpy_s(state->row_type.Column[num].sType,255,column,strlen(column)+1);
				}
				else {
					Printf("Error: There is more column than Column-Count\n");
				}
				num++;
				column = strtok_s(NULL, " ",&context);
			}while(column!=NULL);
			Printf("Fin de la lecture des colonnes\n");
		}
	}
	//get Column-Aliases-Base64
	{
		char column_alias[2048];
		char *alias=NULL;
		unsigned int num=0;
		//char *context=NULL;
		if(get(header,"Column-Aliases-Base64",column_alias,2048)==0) {
			/* delete the last espace char if exist */
			if(column_alias[strlen(column_alias)-1]==' ') {
				column_alias[strlen(column_alias)-1]=0;
			}
			Printf("Column-Aliases-Base64 => '%s'\n",column_alias);
			_alias_str_replace(column_alias);
			alias = strtok_s(column_alias, "\r",&context);
			if(alias!=NULL)
			do{				
				Printf("Alias %d: '%s'\n",num+1,alias);
				if(num<state->row_type.nbColumn) {
					/* erase [] */
					if(*alias=='[' && alias[strlen(alias)-1]==']') {
						strncpy_s(state->row_type.Column[num].sColumnName,255,alias+1,strlen(alias)-2);
					} else {
						strncpy_s(state->row_type.Column[num].sColumnName,255,alias,strlen(alias));
					}					
				}else {
					Printf("Error: There is more alias than Column-Count\n");
				}
				num++;
				alias = strtok_s(NULL, "\r",&context);
			}while(alias!=NULL);
			Printf("Fin de la lecture des alias\n");
		}
	}
	//get Row-Count
	{
		char row_count[250];
		if(get(header,"Row-Count",row_count,250)==0) {
			state->row_count=atoi(row_count);
			Printf("Row-Count:%d\n",state->row_count);
		}
	}
	//get Row-Count-Sent
	{
		char row_count[250];
		if(get(header,"Row-Count-Sent",row_count,250)==0) {
			Printf("Row-Count-Sent:\"%s\" <=lut\n",row_count);
			state->row_count_sent=atoi(row_count);
			Printf("Row-Count-Sent:%d\n",state->row_count_sent);
		}
	}
	//get Statement-ID
	{
		char statement_id[250];
		if(get(header,"Statement-ID",statement_id,250)==0) {
			state->id_statement=atoi(statement_id);
			Printf("Statement-ID:%d\n",state->id_statement);
		}
	}
	//Column-Updateability
	{
		char updateability[250];
		//state->updateability=1;
		if(get(header,"Column-Updateability",updateability,250)==0) {
			state->updateability=(strstr(updateability,"Y")!=NULL);
			Printf("Column-Updateability:%s\n",updateability);
			Printf("Column-Updateability:%d\n",state->updateability);
		}
	}
	//get Result-Type
	{
		char result_type[250];
		if(get(header,"Result-Type",result_type,250)==0) {
			strstrip(result_type);
			//if Result-Type containt more than 1 Result-type => multirequete => not supproted by this driver
			if(strstr(result_type," ")!=NULL)
			{
				//multiquery not supproted by this driver
				Printf("Result-Type:'%s'\n",result_type);
				Printf("Position %d\n",strstr(result_type," ")-result_type);
				Printferr("Error: Multiquery not supported\n");
				return 1;
			}
			state->resultType=resultTypeFromString(result_type);
			switch(state->resultType) {
			case UPDATE_COUNT:
				break;
			case RESULT_SET:
				break;
			case UNKNOW:
			default:
				Printf("Error: %d Result-Type not supported",result_type);
				break;
			}
		}
	}
	return 0;
}

int receiv_check(FOURD *cnx,FOURD_RESULT *state)
{
	socket_receiv_header(cnx,state);
	if(traite_header_response(state)!=0) {
		Printferr("Error in traite_header_response\n");
		cnx->status=state->status;
		cnx->error_code=state->error_code;
		//_snprintf_s(cnx->error_string,ERROR_STRING_LENGTH,strlen(state->error_string),"%s",state->error_string);
		_snprintf(cnx->error_string,ERROR_STRING_LENGTH,"%s",state->error_string);
		//strncpy_s(cnx->error_string,ERROR_STRING_LENGTH,state->error_string,strlen(state->error_string));
		//printf("traite_header_response return 1=> une erreur\n");
		return 1;
	}
	cnx->status=state->status;
	cnx->error_code=state->error_code;
	strncpy_s(cnx->error_string,ERROR_STRING_LENGTH,state->error_string,ERROR_STRING_LENGTH);
	return 0;
}
void _clear_atrr_cnx(FOURD *cnx)
{
	cnx->error_code=0L;
	strcpy_s(cnx->error_string,ERROR_STRING_LENGTH,"");
	cnx->updated_row=0L;
}
void _free_data_result(FOURD_RESULT *res)
{
	//res->elmt
	unsigned int nbCol=res->row_type.nbColumn;
	unsigned int nbRow=res->row_count_sent;
	unsigned int nbElmt=nbCol*nbRow;
	unsigned int i=0;
	FOURD_ELEMENT *pElmt=res->elmt;
	if(pElmt==NULL) {
		return;
	}
	for(i=0;i<nbElmt;i++,pElmt++) {
		switch(pElmt->type) {
			case VK_BOOLEAN:
			case VK_BYTE:
			case VK_WORD:
			case VK_LONG:
			case VK_LONG8:
			case VK_REAL:
			case VK_DURATION:
			case VK_TIMESTAMP:
			case VK_FLOAT:
				Free(pElmt->pValue);
				break;
			case VK_STRING:
				FreeString((FOURD_STRING *)pElmt->pValue);						
				break;
			case VK_BLOB:
				FreeBlob((FOURD_BLOB *)pElmt->pValue);
				break;
			case VK_IMAGE:
				Printferr("Image-Type not supported\n");
				break;
			default:
				break;
		}
	}
	
	Free(res->elmt);
}

void *_copy(FOURD_TYPE type,void *org)
{
	void *buff=NULL;
	//int size=0;
	if(org!=NULL)
	{
		switch(type) {
			case VK_BOOLEAN:
			case VK_BYTE:
			case VK_WORD:
			case VK_LONG:
				Printf("*******Bind %d ********\n",*(FOURD_LONG*)org);
			case VK_LONG8:
			case VK_REAL:
			case VK_DURATION:
			case VK_TIMESTAMP:
				buff=calloc(1,vk_sizeof(type));
				memcpy(buff,org,vk_sizeof(type));
				break;
			case VK_FLOAT:
				{
					FOURD_FLOAT *f=org;
					FOURD_FLOAT *cp=NULL;
					cp=calloc(1,sizeof(FOURD_FLOAT));
					cp->data=calloc(1,f->data_length);
					cp->exp=f->exp;
					cp->sign=f->sign;
					cp->data_length=f->data_length;
					memcpy(cp->data,f->data,f->data_length);
					buff=cp;
				}
				break;
			case VK_STRING:
				{
					FOURD_STRING *src=org;
					FOURD_STRING *cp=NULL;
					cp=calloc(1,sizeof(FOURD_STRING));
					cp->data=calloc(src->length,2);	/* 2 bytes per char */
					cp->length=src->length;
					memcpy(cp->data,src->data,src->length*2);  /* 2 bytes per char */
					buff=cp;
				}
				break;
			case VK_BLOB:
				{
					FOURD_BLOB *src=org;
					FOURD_BLOB *cp=NULL;
					cp=calloc(1,sizeof(FOURD_BLOB));
					cp->data=calloc(src->length,1);	
					cp->length=src->length;
					memcpy(cp->data,src->data,src->length);
					buff=cp;
				}
				break;
			case VK_IMAGE:
				Printferr("Image-Type not supported\n");
				break;
			default:
				break;
		}
	}
	return buff;
}
char *_serialize(char *data,unsigned int *size, FOURD_TYPE type, void *pObj)
{
	int lSize=0;
	if(pObj!=NULL) {
		switch(type) {
			case VK_BOOLEAN:
			case VK_BYTE:
			case VK_WORD:
			case VK_LONG:
				Printf("*******Serialize %d ********\n",*(FOURD_LONG*)pObj);
			case VK_LONG8:
			case VK_REAL:
			case VK_DURATION: 
				lSize=vk_sizeof(type);
				data=realloc(data,(*size)+lSize);
				memcpy(data+*size,pObj,lSize);
				*size+=lSize;
				break;
			case VK_TIMESTAMP:/* Use other procedure for serialize this one because structure can align */
				{
					FOURD_TIMESTAMP *o=pObj;
					lSize=sizeof(o->year)+sizeof(o->mounth)+sizeof(o->day)+sizeof(o->milli);
					data=realloc(data,(*size)+lSize);
					memcpy(data+*size,&(o->year),2);
					memcpy(data+*size+2,&(o->year),1);
					memcpy(data+*size+3,&(o->year),1);
					memcpy(data+*size+4,&(o->year),4);
					*size+=lSize;
				}
				break;
			case VK_FLOAT:
				{
					FOURD_FLOAT *o=pObj;
					lSize=sizeof(o->exp)+sizeof(o->sign)+sizeof(o->data_length)+o->data_length;
					data=realloc(data,(*size)+lSize);
					memcpy(data+*size,&(o->exp),4);
					memcpy(data+*size+4,&(o->sign),1);
					memcpy(data+*size+5,&(o->data_length),4);
					memcpy(data+*size+9,o->data,o->data_length);
					*size+=lSize;
				}
				break;
			case VK_STRING:
				{
					FOURD_STRING *o=pObj;
					int len=o->length;
					len=-len;
					lSize=sizeof(o->length)+o->length*2;
					data=realloc(data,(*size)+lSize);
					memcpy(data+*size,&len,4);
					memcpy(data+*size+4,o->data,o->length*2);
					*size+=lSize;
				}
				break;
			case VK_BLOB:
				{
					FOURD_BLOB *o=pObj;
					lSize=sizeof(o->length)+o->length*2;
					data=realloc(data,(*size)+lSize);
					memcpy(data+*size,&(o->length),4);
					memcpy(data+*size+4,o->data,o->length*2);
					*size+=lSize;
				}
				break;
			case VK_IMAGE:
				Printferr("Image-Type not supported\n");
				break;
			default:
				break;
		}
	}
	return data;
}void Free(void *p)
{
	if(p) {
		free(p);
		p=NULL;
	}
}
void FreeFloat(FOURD_FLOAT *p)
{
	if(p) {
		Free(p->data);
		Free(p); 
	}
}
void FreeString(FOURD_STRING *p)
{
	if(p) {
		Free(p->data);
		Free(p); 
	}
}
void FreeBlob(FOURD_BLOB *p)
{
	if(p) {
		Free(p->data);
		Free(p);
	}
}
void PrintData(const void *data,unsigned int size)
{
	const char *d=data;
	unsigned int i=0;
	if(size>=1)
		Printf("0x%X",*(char *)(d+i));
	for(i=1;i<size;i++) {
		Printf(" 0x%X",*(char *)(d+i));
	}
}
int _is_multi_query(const char *request)
{
	int i=0;
	size_t len;
	int inCol=0;
	int inStr=0;
	int finFirst=0;
	char car=0;
	if(request==NULL){
		return 0;
	}
	len=strlen(request);
	if(len<1){
		return 0;
	}
	for(i=0;i<len;i++){
		
		car=request[i];
		switch(car){
			case '[':
				/* start of 4D object name */
				if(!inStr){
					if(!inCol){
						/* printf("["); */
						inCol=1;
					}
					else {
						/* printf("_"); */
					}
				}else {
					/* printf("s"); */
				}
				break;
			case ']':
				if(inStr){
					/* printf("s"); */
				}else if(inCol){
					inCol=0;
					/* printf("]"); */
				}else {
					if(i>1){ /* check the previous charactere */
						if(request[i-1]==']'){
							/* not end of colomn name */
							inCol=1;
							/* printf("-"); */
						}else {
							inCol=0;
							/* printf("]"); */
						}
					}else {
						/* printf("_");*/
					}
				}
				
				break;
			case '\'':
				if(!inCol){
				/* printf("'");*/
				if(inStr==0){
					inStr=1;
				}else{
					inStr=0;
				}
				}else{
					/* printf("c"); */
				}
				break;
			case ';':
				/* end of query */
				if(!inCol && !inStr){
					finFirst=1;
					/* printf(";");*/
				}else {
					/*printf("_");*/
				}
				break;
			default:
				if(inCol){
					/* printf("C"); */
				}
				else if(inStr){
					/* printf("S"); */
				}
				else if(car==' '){
					/*printf(" ");*/
				}else{
					if(finFirst){
						/* printf("X"); */
						return 1;
					}else {
						/* printf("*"); */
					}
				}
				break;
		}
		
	}
	return 0;
}
int _valid_query(FOURD *cnx,const char *request)
{
	if(_is_multi_query(request)){
		cnx->error_code=-5001;
		sprintf_s(cnx->error_string,2048,"MultiQuery not supported",2048);
		return 0;
	}
	return 1;
}
