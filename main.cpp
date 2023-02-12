#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/stat.h>

#include "sqlite/sqlite3.h" //For database
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

inline bool file_exist(const string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

class Manager
{
    string username, passwd;
public:
    Manager() {}
    void Authentication()
    {
        cout << "Input your username: ";
        getline(cin, username);
        cout << "Input your password: ";
        getline(cin, passwd);
        cout << "Inserting into database..\n";
    }
};

int main(void)
{
    sqlite3 *db;
    Manager task;
    const string name = "Database/users.db";
    if (!file_exist(name)){
        system("mkdir Database && touch Database/users.db");
        char* sql = "CREATE TABLE users(id INTEGER PRIMARY KEY AUTOINCREMENT, username TEST NOT NULL, password TEXT NOT NULL)";
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