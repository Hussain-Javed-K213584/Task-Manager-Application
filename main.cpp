#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <sqlite3.h>

// #include "sqlite/sqlite3.h" //For database
using namespace std;

static int callback(void *data, int argc, char **argv, char **ColName)
{
    int i;
    printf("%s\n", (const char *)data);
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", ColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}
/*
    The following function checks for the users.db file
    If the file does not exists it returns 0
    Code reference: https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exists-using-standard-c-c11-14-17-c
*/
inline bool file_exist(const string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

class Manager
{
    string username, passwd;
    const char *d = "hello";
    unsigned char md[10];
public:
    Manager() {}
    void Authentication()
    {
        SHA1((unsigned char*)d, 10, md);
        cout << "Input your username: ";
        getline(cin, username);
        cout << "Input your password: ";
        getline(cin, passwd);
        for (unsigned char x: md)
        {
            cout << x;
        }
        cout << endl;
        cout << "Inserting into database..\n";
        //TODO: Insert data to database, make sure that you use hashing for the password
    }
};

int main(void)
{
    sqlite3 *db;
    Manager task;
    task.Authentication();
    const string name = "Database/users.db";
    /*
        For checking if database exists or not
        If database file does not exist, one should be created
        sqlite3 reference: https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
    */
    if (!file_exist(name)){
        system("mkdir Database && touch Database/users.db");
        char* sql = "CREATE TABLE users(id INTEGER PRIMARY KEY AUTOINCREMENT, username TEST NOT NULL, password TEXT NOT NULL)"; //SQL query to create a table
        int rc;
        rc = sqlite3_open("Database/users.db", &db);
        if (rc)
        {
            printf("Cannot open database!\n");
            exit(1);
        }
        else
        {
            printf("Database opened successfully!");
        }
        rc = sqlite3_exec(db, (const char*)sql, callback, 0, 0);
        if (rc == SQLITE_OK)
        {
            printf("SQL executed successfully!");
        }
        else
        {
            printf("SQL statement error!\n");
        }
        sqlite3_close(db);
        exit(0);
    }
    printf("File already exists!\n");
    return 1;
}