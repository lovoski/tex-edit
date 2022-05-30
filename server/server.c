#include <stdio.h>
#include <mysql.h>
#include "access_sql.h"
#include "str_utils.h"

int main()
{
    setbuf(stdout, NULL);
    MYSQL *sql = initialize_mysql("localhost", "root", "123456", "tex_store");
    char *tmp = (char *)malloc(200);
    char *string = (char *)malloc(600);
    /* for (int i = 0; i < 100; ++i)
    {
        insert_op_into_mysql(sql, itostr(i, tmp), 1, 2, 3, 4);
    } */
    scanf("%s", string);
    printf("%s, input is:: %s", "input finished", string);
    insert_texfile_into_mysql(sql, "some_tex", string);
    free_wrapped_mysql(sql);
    free(tmp);
    return 0;
}