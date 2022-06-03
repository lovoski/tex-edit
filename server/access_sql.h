#ifndef ACCESS_SQL_H
#define ACCESS_SQL_H

#include <mysql.h>

/* wrap up the complex procedure to initialize database */
MYSQL *initialize_mysql(char *host, char *user, char *pwd, char *db_name);

/* insert change of char data to database */
void insert_op_into_mysql(MYSQL *sql, char *op, 
        const int b_c, const int b_r, const int l_c, const int l_r);

/* another version of of previous function */
void insert_fullop_into_mysql(MYSQL *sql, char *full_query_sentence);

/* update tex file into database */
void insert_texfile_into_mysql(MYSQL *sql, char *file_name, char *text);

/* get tex file as char* */
char *query_texfile_from_database(MYSQL *sql, char *file_name);

/* free the memory allocated */
void free_wrapped_mysql(MYSQL *sql);

#endif