#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sqlite3.h>
#include <termios.h>
#include <fstream>
#include <dirent.h>

#include "sha256/sha256.h"

#define MAX_PATH 64

// #include "sqlite/sqlite3.h" //For database
using namespace std;

// Responsible to make password input invisible
void SetEcho(bool setter = true)
{
    struct termios t_terminal;
    tcgetattr(STDIN_FILENO, &t_terminal);
    if (!setter)
    {
        t_terminal.c_lflag &= ~ECHO;
    }
    else
    {
        t_terminal.c_lflag |= ECHO;
    }
    (void)tcsetattr(STDIN_FILENO, TCSANOW, &t_terminal);
}

// Sqlite callback
static int callback(void *data, int argc, char **argv, char **ColName)
{
    int i;
    printf("%s\n", (const char *)data);
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", ColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    strcpy((char *)data, argv[0]);
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

// Function to validate username, preventing SQLi (SQL Injection)
bool Validate(string &username)
{
    for (int i = 0; i < username.length(); i++)
    {
        if (username[i] == 32 || username[i] == 39 || username[i] == 40 || username[i] == 41)
        {
            return false;
        }
    }
    return true;
}

class Manager
{
    string username, passwd;
    void LoadTask()
    {

    }
public:
    Manager() {}

    void AccountCreation() // Function responsible for creating user accounts
    {
    CreationAgain:
        string path_to_db(getenv("HOME"));
        path_to_db += "/.taskmgr/users.db";
        string pass_hash;
        sqlite3 *db;    // Sqlite connector
        int rc;         // return code
        char *ErrorMSG; // Will store error message
        cout << "Welcome to Task Manager account creation!\nInput your username: ";
        getline(cin, username);
        SetEcho(false);
        cout << "Please choose a password: ";
        getline(cin, passwd);
        SetEcho();
        cout << endl;
        pass_hash = sha256(this->passwd); // Will contain password hash
        if (!Validate(username))          // Validate user input
        {
            cout << "Username can only contain letters and numbers!\n";
            system("clear");
            goto CreationAgain;
        }
        cout << "SUCCESS!\n";
        string sql_query = "INSERT INTO users (username, password) VALUES ("
                           "'" +
                           username + "'" + ", " + "'" + pass_hash + "'" + " );"; // Insert Query
        cout << "Executing the following query: " << sql_query << endl;
        sleep(5);
        rc = sqlite3_open(path_to_db.c_str(), &db);
        if (rc)
        {
            printf("Cannot open database!\n");
            exit(1);
        }
        rc = sqlite3_exec(db, sql_query.c_str(), callback, 0, &ErrorMSG);
        if (rc != SQLITE_OK)
        {
            cout << "Error executing query!\n";
            cout << ErrorMSG << endl;
            exit(1);
        }
        cout << "Execution successfull!\n";
    }

    void Authentication() // Function responsible for signing users in
    {
    LoginAgain:
        cout << "Input your username: ";
        getline(cin, username);
        if (!Validate(username))
        {
            cout << "Username has invalid characters!\n";
            exit(1);
        }
        SetEcho(false);
        cout << "Input your password: ";
        getline(cin, passwd);
        SetEcho();
        cout << endl;
        string pass_hash = sha256(passwd);
        cout << "Password Hash: " << pass_hash << endl;
        // TODO: Get the account credentials from the database. Remember password hashes must match. Use Where condition in query
        string path_to_db(getenv("HOME"));
        path_to_db += "/.taskmgr/users.db";
        sqlite3 *db;
        char *ErrMsg = 0;
        int rc = sqlite3_open(path_to_db.c_str(), &db);
        if (rc)
        {
            cout << "Cannot open database!\n";
            exit(1);
        }
        char *pass_from_db = new char[258];
        string sql_get_query = "SELECT password FROM users WHERE username = '" + username + "';";
        rc = sqlite3_exec(db, sql_get_query.c_str(), callback, (void *)pass_from_db, &ErrMsg);
        if (rc)
        {
            cout << "Cannot execute sql!\n";
            exit(1);
        }
        if (strcmp(pass_from_db, pass_hash.c_str()) != 0)
        {
            cout << "Incorrect login!\n";
            goto LoginAgain;
        }
        cout << "Authentication successfull!\n";
        // Authentication code is complete

        /*
            Call the LoadTask function to load the user's task and list them
            on the console.
        */
    }
};

int main(void)
{
    // This section of code is completed

    /*
        This section of code creates the user database and inits the database.
        The database consists of the user's username and password.
    */
    sqlite3 *db;
    Manager task;
    string home(getenv("HOME"));
    home += "/.taskmgr";
    DIR *db_dir = opendir(home.c_str()); // Checks if directory exists
    if (ENOENT == errno)
    {
        int check = mkdir(home.c_str(), 0777); // Create directory if it does not exists
        cout << check;
    }
    string path_to_db_file;
    path_to_db_file += home + "/users.db";
    /*
        For checking if database exists or not
        If database file does not exist, one should be created
        sqlite3 reference: https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
    */
    if (!file_exist(path_to_db_file))
    {

        string sql = "CREATE TABLE users(id INTEGER PRIMARY KEY AUTOINCREMENT, username TEST NOT NULL, password TEXT NOT NULL)"; // SQL query to create a table
        int rc;
        rc = sqlite3_open(path_to_db_file.c_str(), &db);
        if (rc)
        {
            printf("Cannot open database!\n");
            return 1;
        }
        else
        {
            printf("Database opened successfully!");
        }
        rc = sqlite3_exec(db, (const char *)sql.c_str(), callback, 0, 0);
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
    else
    {
        // task.AccountCreation();
        task.Authentication();
    }
    printf("\nFile already exists!\n");
    return 0;
}
