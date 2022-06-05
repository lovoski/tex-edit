#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include "access_sql.h"
#include "str_utils.h"

#define DCL 600

char *q1, *q2, *tmp;
static char *query1 = "INSERT INTO tex_op (content,b_c,b_r,l_c,l_r) VALUES (\'";
static char *query2 = "INSERT INTO tex_file (file_name,content) VALUES (\'";

MYSQL *initialize_mysql(char *host, char *user, char *pwd, char *db_name)
{
    q1 = init_str_from_stack(DCL, query1);
    q2 = init_str_from_stack(DCL, query2);
    tmp = (char *)malloc(DCL);

    MYSQL *sql = mysql_init(NULL);
    if (sql == NULL) {
        printf("%s\n", "[server]:initialize failed");
        exit(1);
    }
    if (mysql_real_connect(sql, host, user, pwd, db_name, 0, NULL, 0) == NULL) {
        printf("%s\n", "[server]:connection failed");
        exit(1);
    }
    printf("%s\n", "[server]:connected to databse");
    // create area to store *.tex file
    if (mysql_query(sql,
        "CREATE TABLE IF NOT EXISTS tex_file (file_name TEXT, content LONGTEXT)"
    )) {
        printf("Error: %s", mysql_error(sql));
    }
    // create area to store each opearation
    if (mysql_query(sql,
        "CREATE TABLE IF NOT EXISTS tex_op (content TEXT, b_c INT,b_r INT,l_c INT,l_r INT)"
    )) {
        printf("Error: %s", mysql_error(sql));
    }
    return sql;
}

void insert_op_into_mysql(MYSQL *sql, char *op,
    const int b_c, const int b_r, const int l_c, const int l_r)
{
    // the return value is very important, do pay attention to it!!!
    q1 = reset_string(q1, query1); // reset the global string
    q1 = strconcat(q1, op);
    q1 = strconcat(q1, "\',");
    q1 = strconcat(q1, itostr(b_c, tmp));
    q1 = strconcat(q1, ",");
    q1 = strconcat(q1, itostr(b_r, tmp));
    q1 = strconcat(q1, ",");
    q1 = strconcat(q1, itostr(l_c, tmp));
    q1 = strconcat(q1, ",");
    q1 = strconcat(q1, itostr(l_r, tmp));
    q1 = strconcat(q1, ")");
    if (mysql_query(sql, q1)) {
        printf("Error: %s", mysql_error(sql));
    }
}

void insert_fullop_into_mysql(MYSQL *sql, char *full_query_sentence)
{
    if (mysql_query(sql, full_query_sentence)) {
        printf("Error: %s", mysql_error(sql));
    }
}

tex_file *query_texfile_from_database(MYSQL *sql, char *file_name, tex_file *file)
{
    if (mysql_query(sql, "SELECT * FROM tex_file")) {
        printf("error querying tex_file, %s\n", mysql_error(sql));
        file->len = -1;
        file->index = -1;
        return file;
    }
    MYSQL_RES *res = mysql_store_result(sql);
    if (res == NULL) {
        printf("error fetching mysql_res, %s\n", mysql_error(sql));
        file->len = -1;
        file->index = -1;
        return file;
    }
    int field_num = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);
    MYSQL_ROW row;
    int row_count = 0;
    while ((row = mysql_fetch_row(res)) != NULL) {
        if (compare_string(row[0], file_name)) { // file found
            printf("existing tex file found\n");
            unsigned int len = strlen(row[1]);
            file->index = row_count;
            file->len = len;
            printf("found file content=%s, length=%d\n", row[1], len);
            memcpy(file->content, row[1], len);
            return file;
        }
        row_count++;
    }
    file->len = -1;
    return file; // not found
}

int __query_for_texfile_name(MYSQL *sql, char *file_name)
{
    if (mysql_query(sql, "SELECT * FROM tex_file")) {
        printf("Error: %s\n", mysql_error(sql));
        return -1; // exit with -1 when error occurrs
    }
    MYSQL_RES *res = mysql_store_result(sql);
    MYSQL_ROW sql_row;
    while ((sql_row = mysql_fetch_row(res))) {
        if (compare_string(file_name, sql_row[0])) {
            return 2;
        }
    }
    return 0;
}

void insert_texfile_into_mysql(MYSQL *sql, char *file_name, char *text)
{
    if (__query_for_texfile_name(sql, file_name)) {
        // update file
        printf("%s\n", "[server]:update file");
        q1 = reset_string(q1, "UPDATE tex_file SET content=\'");
        q1 = strconcat(q1, text);
        q1 = strconcat(q1, "\' WHERE file_name=\'");
        q1 = strconcat(q1, file_name);
        q1 = strconcat(q1, "\'");
        if (mysql_query(sql, q1)) {
            printf("Error: %s", mysql_error(sql));
        }
    } else {
        // insert file
        printf("%s\n", "[server]:insert new file");
        q2 = reset_string(q2, query2); // reset the global string
        q2 = strconcat(q2, file_name);
        q2 = strconcat(q2, "\',\'");
        q2 = strconcat(q2, text);
        q2 = strconcat(q2, "\')");
        if (mysql_query(sql, q2)) {
            printf("Error: %s\n", mysql_error(sql));
        }
    }
}

void free_wrapped_mysql(MYSQL *sql)
{
    free(q1);
    free(q2);
    free(tmp);
    mysql_close(sql);
}
