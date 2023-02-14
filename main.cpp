#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include "sha256/sha256.h"
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
public:
    Manager() {}
    void Authentication()
    {
        cout << "Input your username: ";
        getline(cin, username);
        cout << "Input your password: ";
        getline(cin, passwd);
        cout << "The password is: " << passwd
        << "\nThe password sha256 hash is: " << sha256(passwd);
        //TODO: Insert data to database, make sure that you use hashing for the password
    }
};

int main(void)
{
    sqlite3 *db;
    Manager task;
    // task.Authentication();
    string pwd_home = getenv("HOME"); //Provides home directory path
    const string file_name = pwd_home + "/Desktop/Task-Manager-Application/Database/users.db"; //Path to database file
    const string dir_name = pwd_home + "/Desktop/Task-Manager-Application/Database"; //Database directory name
    cout << dir_name << endl;
    /*
        For checking if database exists or not
        If database file does not exist, one should be created
        sqlite3 reference: https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
    */
    if (!file_exist(dir_name)){
        string command = "mkdir ";
        command += dir_name + " && touch" + file_name; //The final comand to make the directory and file 
        system(command.c_str());
        string sql = "CREATE TABLE users(id INTEGER PRIMARY KEY AUTOINCREMENT, username TEST NOT NULL, password TEXT NOT NULL)"; //SQL query to create a table
        int rc;
        rc = sqlite3_open(file_name.c_str(), &db);
        if (rc)
        {
            printf("Cannot open database!\n");
            return 1;
        }
        else
        {
            printf("Database opened successfully!");
        }
        rc = sqlite3_exec(db, (const char*)sql.c_str(), callback, 0, 0);
        if (rc == SQLITE_OK)
        {
            printf("SQL executed successfully!");
        }
        else
        {
            printf("SQL statement error!\n");
        }
        sqlite3_close(db);
        return 0;
    }
    printf("File already exists!\n");
    return 1;
}