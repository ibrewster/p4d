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
const char * fourd_sqlstate(FOURD *cnx)
{
	switch(cnx->error_code){
		case -10060: return "08001";/* Unable to connect to server => Client unable to establish connection */
		case -1: return "01S00";
		/*case 1105: return "08004";*/	/* Failed to authenticate. => Server rejected the connection */
		
		case 1101: return "42P01";	/* Failed to execute statement. => Undefined table <= TABLE DOES NOT EXIST */
		case 1102: return "42S22";  /* Column not found <= colonne DOES NOT EXIST*/
		case 1103: return "42P01";  /* Undefined table <= TABLE NOT DECLARED IN FROM CLAUSE */
		case 1104: return "42702";  /* Ambiguous column <= AMBIGUOUS COLUMN NAME */
		case 1105: return "42P09";  /* Ambiguous alias <= TABLE ALIAS SAME AS TABLE NAME */
		case 1106: return "42P09";  /* Ambiguous alias <= DUPLICATE TABLE ALIAS */
		case 1107: return "42P09";  /* Ambiguous alias <= DUPLICATE TABLE IN FROM CLAUSE  */
		case 1108: return "HY004";	/* Failed to execute statement. => Invalid SQL data type <= INCOMPATIBLE TYPES */
		
		case 1109: return "HY000";	/*  <= INVALID ORDER BY INDEX */
		case 1110: return "42P08";	/* Ambiguous parameter <= WRONG AMOUNT OF PARAMETERS */
		
		case 1111: return "HY105";	/* Invalid parameter type <= INCOMPATIBLE PARAMETER TYPE */
		case 1112: return "42883";	/* Undefined function <= UNKNOWN FUNCTION */
		case 1113: return "22012";	/* Division by zero <= DIVISION BY ZERO */
		
		case 1114: return "HY000";	/*  <= ORDER BY INDEX NOT ALLOWED */
		case 1115: return "HY000";	/*  <= DISTINCT NOT ALLOWED */
		case 1116: return "HY000";	/*  <= NESTED COLUMN FUNCTIONS NOT ALLOWED */
		case 1117: return "HY000";	/*  <= COLUMN FUNCTIONS NOT ALLOWED */
		
		case 1118: return "25007";	/* Schema and data statement mixing not supported <= CAN NOT MIX COLUMN AND SCALAR OPERATIONS */
		case 1119: return "42803";	/* Grouping error <= INVALID GROUP BY INDEX */
		case 1120: return "42803";	/* Grouping error <= GROUP BY INDEX NOT ALLOWED */
		case 1121: return "42803";	/* Grouping error <= GROUP BY NOT ALLOWED WITH SELECT ALL */
		case 1122: return "42702";	/* Ambiguous column <= NOT A COLUMN EXPRESSION */
		case 1123: return "42803";	/* Grouping error <= NOT A GROUPING COLUMN IN AGGREGATE ORDER BY */
		
		case 1124: return "HY000";	/*  <= MIXED LITERAL TYPES IN PREDICATE */
		
		case 1125: return "2200B";	/* Escape character conflict <= LIKE ESCAPE IS NOT ONE CHAR */
		case 1126: return "2200B";	/* Escape character conflict <= BAD LIKE ESCAPE CHAR */
		case 1127: return "2200B";	/* Escape character conflict <= UNKNOWN ESCAPE SEQUENCE IN LIKE */
		
		case 1128: return "HY000";	/*  <= COLUMNS FROM MORE THAN ONE QUERY IN COLUMN FUNCTION */
		
		case 1129: return "42803";	/* Grouping error <= SCALAR EXPRESSION WITH GROUP BY */
		
		case 1130: return "HY000";	/*  <= SUBQUERY HAS MORE THAN ONE COLUMN */
		case 1131: return "HY000";	/*  <= SUBQUERY MUST HAVE ONE ROW */
		
		case 1132: return "21S01";	/* Insert value list does not match column list <= INSERT VALUE COUNT DOES NOT MATCH COLUMN COUNT */
		case 1133: return "21S01";	/* Insert value list does not match column list <= DUPLICATE COLUMN IN INSERT */
		case 1134: return "23502";	/* Not null violation <= COLUMN DOES NOT ALLOW NULLS */
		case 1135: return "42701";	/* Duplicate column <= DUPLICATE COLUMN IN UPDATE */
		case 1136: return "42P07";	/* Duplicate table <= TABLE ALREADY EXISTS */

		case 1137: return "42701";	/* Duplicate column <= DUPLICATE COLUMN IN CREATE TABLE */
		case 1138: return "42701";	/* Duplicate column <= DUPLICATE COLUMN IN COLUMN LIST */
		
		case 1139: return "HY000";	/*  <= MORE THAN ONE PRIMARY KEY NOT ALLOWED */
		
		case 1140: return "42830";	/* Invalid foreign key <= AMBIGUOUS FOREIGN KEY NAME */
		case 1141: return "42830";	/* Invalid foreign key <= COLUMN COUNT MISMATCH IN FOREIGN KEY */
		case 1142: return "42830";	/* Invalid foreign key <= COLUMN TYPE MISMATCH IN FOREIGN KEY */
		case 1143: return "42S22";	/* Column not found <= FAILED TO FIND MATCHING PRIMARY COLUMN */
		
		case 1144: return "HY000";	/*  <= UPDATE AND DELETE CONSTRAINTS MUST BE THE SAME */
		
		case 1145: return "42830";	/* Invalid foreign key <= FOREIGN KEY DOES NOT EXIST */
		case 1146: return "22020";	/* Invalid limit value <= INVALID LIMIT VALUE IN SELECT */
		
		case 1147: return "HY000";	/*  <= INVALID OFFSET VALUE IN SELECT */
		case 1148: return "HY000";	/*  <= PRIMARY KEY DOES NOT EXIST */
		
		case 1149: return "42830";	/* Invalid foreign key <= FAILED TO CREATE FOREIGN KEY */
		
		case 1150: return "HY000";	/*  <= FIELD IS NOT IN PRIMARY KEY */
		case 1151: return "HY000";	/*  <= FIELD IS NOT UPDATEABLE */
		
		case 1153: return "HY090";	/* Invalid string or buffer length <= BAD DATA TYPE LENGTH */
		case 1154: return "HY000";	/* General error <= EXPECTED EXECUTE IMMEDIATE COMMAND  */
		
		
		case 1203: return "HY000";	/*  <= FUNCTIONALITY IS NOT IMPLEMENTED */
		case 1204: return "HY000";	/*  <= FAILED TO CREATE NEW RECORD */
		case 1205: return "HY000";	/*  <= FAILED TO UPDATE FIELD */
		case 1206: return "HY000";	/*  <= FAILED TO DELETE RECORD */
		case 1207: return "HY000";	/*  <= NO MORE JOIN SEEDS POSSIBLE*/
		case 1208: return "HY000";	/*  <= FAILED TO CREATE TABLE */
		case 1209: return "HY000";	/*  <= FAILED TO DROP TABLE */
		case 1210: return "HY000";	/*  <= CANT BUILD BTREE FOR ZERO RECORDS */
		case 1211: return "HY000";	/*  <= COMMAND COUNT GREATER THAN ALLOWED */
		case 1212: return "HY000";	/*  <= FAILED TO CREATE DATABASE */
		case 1213: return "HY000";	/*  <= FAILED TO DROP COLUMN  */
		case 1214: return "HY000";	/*  <= VALUE IS OUT OF BOUNDS */
		case 1215: return "HY000";	/*  <= FAILED TO STOP SQL_SERVER */
		case 1216: return "HY000";	/*  <= FAILED TO LOCALIZE */
		case 1217: return "HY000";	/*  <= FAILED TO LOCK TABLE FOR READING */
		case 1218: return "HY000";	/*  <= FAILED TO LOCK TABLE FOR WRITING */
		case 1219: return "HY000";	/*  <= TABLE STRUCTURE STAMP CHANGED */
		case 1220: return "HY000";	/*  <= FAILED TO LOAD RECORD */
		case 1221: return "HY000";	/*  <= FAILED TO LOCK RECORD FOR WRITING */
		case 1222: return "HY000";	/*  <= FAILED TO PUT SQL LOCK ON A TABLE */
		
		case 1301: return "42601";	/* Failed to parse statement. => Syntax error */
		
		
		case 1401: return "HY000";	/*  <= COMMAND NOT SPECIFIED */
		case 1402: return "HY000";	/*  <= ALREADY LOGGED IN */
		case 1403: return "HY000";	/*  <= SESSION DOES NOT EXIST */
		case 1404: return "HY000";	/*  <= UNKNOWN BIND ENTITY */
		case 1405: return "HY000";	/*  <= INCOMPATIBLE BIND ENTITIES */
		case 1406: return "HY000";	/*  <= REQUEST RESULT NOT AVAILABLE */
		case 1407: return "HY000";	/*  <= BINDING LOAD FAILED */
		case 1408: return "HY000";	/*  <= COULD NOT RECOVER FROM PREVIOUS ERRORS */
		case 1409: return "HY000";	/*  <= NO OPEN STATEMENT */
		case 1410: return "HY000";	/*  <= RESULT EOF */
		case 1411: return "HY000";	/*  <= BOUND VALUE IS NULL */
		case 1412: return "HY000";	/*  <= STATEMENT ALREADY OPENED */
		case 1413: return "HY000";	/*  <= FAILED TO GET PARAMETER VALUE */
		case 1414: return "HY000";	/*  <= INCOMPATIBLE PARAMETER ENTITIES */
		case 1415: return "HY000";	/*  <= PARAMETER VALUE NOT SPECIFIED */
		case 1416: return "HY000";	/*  <= COLUMN REFERENCE PARAMETERS FROM DIFFERENT TABLES */
		case 1417: return "HY000";	/*  <= EMPTY STATEMENT */
		case 1418: return "HY000";	/*  <= FAILED TO UPDATE VARIABLE */
		case 1419: return "HY000";	/*  <= FAILED TO GET TABLE REFERENCE */
		case 1420: return "HY000";	/*  <= FAILED TO GET TABLE CONTEXT */
		case 1421: return "HY000";	/*  <= COLUMNS NOT ALLOWED */
		case 1422: return "HY000";	/*  <= INVALID COMMAND COUNT */
		case 1423: return "HY000";	/*  <= INTO CLAUSE NOT ALLOWED */
		case 1424: return "HY000";	/*  <= EXECUTE IMMEDIATE NOT ALLOWED */
		case 1425: return "HY000";	/*  <= ARRAY NOT ALLOWED IN EXECUTE IMMEDIATE */
		case 1426: return "HY000";	/*  <= COLUMN NOT ALLOWED IN EXECUTE IMMEDIATE */
		case 1427: return "HY000";	/*  <= NESTED BEGIN END SQL NOT ALLOWED */
		case 1428: return "HY000";	/*  <= RESULT IS NOT A SELECTION */
		case 1429: return "HY000";	/*  <= INTO ITEM IS NOT A VARIABLE (LANGUAGE RUNTIME) */
		case 1430: return "HY000";	/*  <= VARIABLE WAS NOT FOUND (LANGUAGE RUNTIME) */
		case 1501: return "HY000";	/*  <= SEPARATOR_EXPECTED */
		
		case 1502: return "22007";	/* Invalid datetime format <= FAILED TO PARSE DAY OF MONTH */
		case 1503: return "22007";	/* Invalid datetime format <= FAILED TO PARSE MONTH */
		case 1504: return "22007";	/* Invalid datetime format <= FAILED TO PARSE YEAR */
		case 1505: return "22007";	/* Invalid datetime format <= FAILED TO PARSE HOUR */
		case 1506: return "22007";	/* Invalid datetime format <= FAILED TO PARSE MINUTE */
		case 1507: return "22007";	/* Invalid datetime format <= FAILED TO PARSE SECOND */
		case 1508: return "22007";	/* Invalid datetime format <= FAILED TO PARSE MILLISECOND */
		case 1509: return "22007";	/* Invalid datetime format <= INVALID AM PM USAGE */
		case 1510: return "22007";	/* Invalid datetime format <= FAILED TO PARSE TIME ZONE */
		
		case 1511: return "22007";	/* Invalid datetime format <= UNEXPECTED CHARACTER */
		case 1512: return "22007";	/* Invalid datetime format <= FAILED TO PARSE TIMESTAMP */
		case 1513: return "22007";	/* Invalid datetime format <= FAILED TO PARSE DURATION */
		case 1551: return "22007";	/* Invalid datetime format <= FAILED */
		
		case 1601: return "HY000";	/*  <= NULL INPUT STRING */
		case 1602: return "HY000";	/*  <= NON TERMINATED STRING */
		case 1603: return "HY000";	/*  <= NON TERMINATED COMMENT */
		case 1604: return "HY000";	/*  <= INVALID NUMBER */
		case 1605: return "HY000";	/*  <= UNKNOWN START OF TOKEN */
		case 1606: return "HY000";	/*  <= NON TERMINATED NAME */
		case 1607: return "HY000";	/*  <= NO VALID TOKENS */
		case 1837: return "HY000";	/*  <= DB4D QUERY FAILED */
		case 2000: return "HY000";	/*  <= CACHEABLE NOT INITIALIZED */
		case 2001: return "HY000";	/*  <= VALUE ALREADY CACHED */
		case 2002: return "HY000";	/*  <= CACHED VALUE NOT FOUND */
		case 3000: return "HY000";	/*  <= HEADER NOT FOUND */
		case 3001: return "HY000";	/*  <= UNKNOWN COMMAND */
		case 3002: return "HY000";	/*  <= ALREADY LOGGED IN */
		case 3003: return "HY000";	/*  <= NOT LOGGED IN */
		case 3004: return "HY000";	/*  <= UNKNOWN OUTPUT MODE */
		case 3005: return "HY000";	/*  <= INVALID STATEMENT ID */
		case 3006: return "HY000";	/*  <= UNKNOWN DATA TYPE */
		case 3007: return "HY000";	/*  <= STILL LOGGED IN */
		case 3008: return "HY000";	/*  <= SOCKET READ ERROR */
		case 3009: return "HY000";	/*  <= SOCKET WRITE ERROR */
		case 3010: return "HY000";	/*  <= BASE64 DECODING ERROR */ 
		case 3011: return "HY000";	/*  <= SESSION TIMEOUT */
		case 3012: return "HY000";	/*  <= FETCH TIMESTAMP ALREADY EXISTS */ 
		case 3013: return "HY000";	/*  <= BASE64 ENCODING ERROR */
		case 3014: return "HY000";	/*  <= INVALID HEADER TERMINATOR */

		case -5001: return "0LP01"; /* driver not support multiquery */
		default: return "HY000";
	}
}