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

FOURD_TYPE typeFromString(const char *type);
const char* stringFromType(FOURD_TYPE type);

int vk_sizeof(FOURD_TYPE type);

typedef enum
{
	UNKNOW=0,
	UPDATE_COUNT,
	RESULT_SET
}FOURD_RESULT_TYPE;

FOURD_RESULT_TYPE resultTypeFromString(const char *type);
const char* stringFromResultType(FOURD_RESULT_TYPE type);


typedef short FOURD_BOOLEAN;
typedef short FOURD_BYTE;
typedef short FOURD_WORD;
typedef int FOURD_LONG;
typedef long long FOURD_LONG8;
typedef double FOURD_REAL;
typedef struct{int exp;unsigned char sign;int data_length;void* data;}FOURD_FLOAT;
typedef struct{short year;unsigned char mounth;unsigned char day;unsigned int milli;}FOURD_TIMESTAMP;
typedef long long FOURD_DURATION;//in milliseconds
typedef struct{int length;char *data;}FOURD_STRING;
typedef struct{int length;void *data;}FOURD_BLOB;
typedef struct{
	FOURD_TYPE type;
	char null;//0 not null, 1 null
	void *pValue;
}FOURD_ELEMENT;

typedef struct{
	char sType[255];
	FOURD_TYPE type;
	char sColumnName[255];
}FOURD_COLUMN;

typedef struct{
	unsigned int nbColumn;
	FOURD_COLUMN *Column;
}FOURD_ROW_TYPE;

typedef struct{
	int socket;

	int init;		/*boolean*/
	int connected;	/*boolean*/

	/*deprecated: use FOURD_RESULT*/
	/*char reponse[BUFFER_LENGTH];
	int reponse_len;*/

	/* status */
	int status;//1 pour OK, 0 pour KO
	FOURD_LONG8 error_code;
	char error_string[2048];

	/* updated row */
	FOURD_LONG8 updated_row;

	/* PREFERRED-IMAGE-TYPES */
	char *preferred_image_types;
	int timeout;

} FOURD;

typedef struct{
	FOURD *cnx;
	char header[2048];
	unsigned int header_size;

	/*state of statement (OK or KO)*/
	int status;	/*FOURD_OK or FOURD_ERRROR*/
	FOURD_LONG8 error_code;
	char error_string[2048];

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

FOURD_RESULT *fourd_query(FOURD *cnx,const char *query);
FOURD_LONG8 fourd_num_rows(FOURD_RESULT *result);
FOURD_LONG8 fourd_affected_rows(FOURD *cnx);
int fourd_num_columns(FOURD_RESULT *res);
int fourd_close(FOURD *cnx);

FOURD_STATEMENT * fourd_prepare_statement(FOURD *cnx,const char *query);
FOURD_STRING *fourd_create_string(char *param,int length);
int fourd_bind_param(FOURD_STATEMENT *state,unsigned int numParam,FOURD_TYPE type, void *val);
FOURD_RESULT *fourd_exec_statement(FOURD_STATEMENT *state, int res_size);
int fourd_close_statement(FOURD_RESULT *res);
void fourd_free_result(FOURD_RESULT *res);
void fourd_free(FOURD* cnx);
void fourd_free_statement(FOURD_STATEMENT *state);

int fourd_next_row(FOURD_RESULT *res);
void * fourd_field(FOURD_RESULT *res,unsigned int numCol);
FOURD_STRING * fourd_field_string(FOURD_RESULT *res,unsigned int numCol);
FOURD_LONG * fourd_field_long(FOURD_RESULT *res,unsigned int numCol);

const char * fourd_get_column_name(FOURD_RESULT *res,unsigned int numCol);
FOURD_TYPE fourd_get_column_type(FOURD_RESULT *res,unsigned int numCol);

int fourd_field_to_string(FOURD_RESULT *res,unsigned int numCol,char **value,size_t *len);

int fourd_errno(FOURD *cnx);
const char * fourd_error(FOURD *cnx);

/* Misc other C functions needed by the p4d driver */
void free(void *);

