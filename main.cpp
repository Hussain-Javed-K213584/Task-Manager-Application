#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <sqlite3.h>
#include <termios.h>

#include "sha256/sha256.h"

// #include "sqlite/sqlite3.h" //For database
using namespace std;

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
    (void) tcsetattr(STDIN_FILENO, TCSANOW, &t_terminal);
}

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
public:
    Manager() {}

    void AccountCreation() // Function responsible for creating user accounts
    {
        string pass_hash;
        sqlite3 *db; // Sqlite connector
        int rc; //return code
        char *ErrorMSG; // Will store error message
        cout << "Welcome to Task Manager account creation!\nInput your username: ";
        getline(cin, username);
        SetEcho(false);
        cout << "Please choose a password: ";
        getline(cin, passwd);
        SetEcho();
        cout << endl;
        pass_hash = sha256(this->passwd); // Will contain password hash
        if (!Validate(username)) // Validate user input 
        {
            cout << "Username can only contain letters and numbers!\n";
        }
        cout << "SUCCESS!\n";
        string sql_query = "INSERT INTO users (username, password) VALUES (" "'" + username + "'" + ", " + "'" + pass_hash + "'" + " );"; // Insert Query
        cout << "Executing the following query: " << sql_query << endl;
        sleep(5);
        rc = sqlite3_open("../Database/users.db", &db);
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
        cout << "Input your username: ";
        getline(cin, username);
        SetEcho(false);
        cout << "Input your password: ";
        getline(cin, passwd);
        SetEcho();
        cout << endl;
        string pass_hash = sha256(passwd);
        cout << "Password Hash: " << pass_hash << endl;
        //TODO: Get the account credentials from the database. Remember password hashes must match. Use Where condition in query
    }
};

int main(void)
{
    sqlite3 *db;
    Manager task;
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
        command += dir_name + " && touch " + file_name; //The final comand to make the directory and file 
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
    else
    {
        task.AccountCreation();
    }
    printf("\nFile already exists!\n");
    return 1;
}
