#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
// #include <sqlite3.h>
#include "sqlite_modern_cpp-dev/hdr/sqlite_modern_cpp.h"
#include <termios.h>
#include <fstream>
#include <iomanip>
#include <dirent.h>
#include <vector>
#include <ctime>

#include "sha256/sha256.h"

#define MAX_PATH 64

using namespace sqlite;
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

class TaskManager
{
    string username, passwd;
    int taskCounter;
    pair<vector<int>, vector<string>> userTasks;
    /*
    This will load the task of the user from the database created.
    Call the function and work will done. Tasks will stored in vector pair.
    */
    void validateDeadline(string &deadline)
    {
        // TODO: Complete date time validation
        if (deadline.length() > 11 || deadline[0] > 1 || deadline[0] < 0 || deadline [1] > 9 || deadline[1] < 0 || \
        deadline[3] < 0 || deadline[3] > 3 || deadline[4] < 0 || deadline[4] > 9 || deadline[])
        {
            cout << "Invalid!";
        }
    }
    void LoadTask() // Function loads the task from data base
    {
    }

    void addTask()
    {
        string sqlInsertTask = "INSERT INTO " + username + " (tasks, pirority, creation_date, deadline) VALUES (?, ?, ?, ?);";
        string newTask, timestamp, deadline, currentTimestamp;
        short priority;
        cout << "Input field: ";
        getline(cin >> ws, newTask);
        cout << "Task priority: ";
        cin >> priority;
        time_t result = time(nullptr);
        currentTimestamp = asctime(localtime(&result));
        cout << "Set a deadline - MM/DD/YYYY\n";
        getline(cin >> ws, deadline);
        validateDeadline(deadline);
    }
    void removeTask()
    {
    }
    void updateTask()
    {
    }

public:
    TaskManager()
    {
        taskCounter = 0;
    }

    void AccountCreation() // Function responsible for creating user accounts and the user's task database
    {
        string path_to_db(getenv("HOME"));
        path_to_db += "/.taskmgr/users.db";
        string path_to_folder(getenv("HOME"));
        path_to_folder += "/.taskmgr/";
        string pass_hash;
        database dbConn(path_to_db);
    CreationAgain:
        cout << "Welcome to Task Manager account creation!\nInput your username: ";
        getline(cin >> ws, username);
        SetEcho(false);
        cout << "Please choose a password: ";
        getline(cin >> ws, passwd);
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
        string sql_query = "INSERT INTO users (username, password) VALUES (?, ?);"; // Insert Query
        cout << "Executing the following query: " << sql_query << endl;
        sleep(5);
        dbConn << sql_query
               << username
               << pass_hash;
        cout << "Execution successfull!\n";
        // TODO: Create user's database file to hold tasks
        database userConn(path_to_folder + username + ".db");
        // TODO: Create user's table
        string sqlQueryDatabase = "CREATE TABLE if not exists " + username + " ("
                                                                             "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, task TEXT, depends_on INTEGER, priority INTEGER, "
                                                                             "creation_date TEXT NOT NULL, deadline TEXT NOT NULL);";
        userConn << sqlQueryDatabase;
    }

    void Authentication() // Function responsible for signing users in
    {
    LoginAgain:
        cout << "Input your username: ";
        getline(cin >> ws, username);
        if (!Validate(username))
        {
            cout << "Username has invalid characters!\n";
            exit(1);
        }
        SetEcho(false);
        cout << "Input your password: ";
        getline(cin >> ws, passwd);
        SetEcho();
        cout << endl;
        string pass_hash = sha256(passwd);
        cout << "Password Hash: " << pass_hash << endl;
        // TODO: Get the account credentials from the database. Remember password hashes must match. Use Where condition in query
        string path_to_db(getenv("HOME")), pass_from_db;
        path_to_db += "/.taskmgr/users.db";
        database dbConn(path_to_db);
        string sql_get_query = "SELECT password FROM users WHERE username = ?;";
        dbConn << sql_get_query << username >> pass_from_db; // Password will be stored in pass_from_db var
        if (pass_from_db != pass_hash)                       // Compares the received hash
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
        LoadTask();
    }

    void taskManaging()
    {
        cout << "Hello " + username + "\nWhat is on your mind?\n";
        cout << "1. Add Task\n"
             << "2. Remove Task\n"
             << "3. Update task\n";
        short input;
        cin >> input;
        switch (input)
        {
        case 1:
            addTask();
            break;
        case 2:
            removeTask();
            break;
        case 3:
            updateTask();
            break;
        }
    }
};

int main(void)
{
    // This section of code is completed

    /*
        This section of code creates the user database and inits the database.
        The database consists of the user's username and password.
    */
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
    database db(path_to_db_file);

    string sql_create_users = "CREATE TABLE if not exists users(id INTEGER PRIMARY KEY AUTOINCREMENT, username TEST NOT NULL, password TEXT NOT NULL)"; // SQL query to create a table
    db << sql_create_users;

    printf("\nUser database file already exists!\n");
    sleep(2);
    system("clear");
    // Start main program
    TaskManager task;
    while (1)
    {
        char input;
        cout << "\t\t\t\tWelcome to Task Manager\n"
             << "\t\t\t\t1. Login\n"
             << "\t\t\t\t2. Sign up\n"
             << "\t\t\t\tInput: ";
        cin >> input;
        if (input == '1')
        {
            task.Authentication();
        }
        else if (input == '2')
        {
            task.AccountCreation();
            cout << "Account creation successful!\n";
            sleep(1);
            system("clear");
        }
        else
        {
            cout << "\t\t\t\tInvalid Input\n";
            sleep(1);
            system("clear");
            input = '\0';
        }
    }
    return 0;
}
