#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
// #include <sqlite3.h>
#include <termios.h>
#include <fstream>
#include <iomanip>
#include <dirent.h>
#include <vector>
#include <ctime>
#include <tuple>

#include "bprinter/include/bprinter/table_printer.h"
#include "sqlite_modern_cpp/hdr/sqlite_modern_cpp.h"
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
    typedef vector< tuple<unsigned int, string, unsigned int>> TaskVector;
    vector<string> timeStampVector, deadlineVector;
    TaskVector TskVT; // Stored Task ID, Task string, and task priority
    /*
        A pretty bad implementation of timestamp validation. Should not be used in production code.
        Good enough for the semester project. Try to use a parser; take help from here:
        https://stackoverflow.com/questions/19482378/how-to-parse-and-validate-a-date-in-stdstring-in-c
    */
    bool validateDeadline(string &deadline)
    {
        // TODO: Complete date time validation
        if (deadline.length() > 11 || deadline[0] > '1' || deadline[0] < '0' || deadline[1] > '9' || deadline[1] < '0' ||
            deadline[3] < '0' || deadline[3] > '3' || deadline[4] < '0' || deadline[4] > '9' || deadline[6] < '0' || deadline[7] < '0' || deadline[8] < '0' || deadline[9] < '0')
        {
            cout << "Invalid!";
            return false;
        }
        return true;
        // Completed
    }
    /*
    The following functions returns the path to the database that stores
    user specific task and their related information.
    */
    string getCurrentDatabasePath()
    {
        string path_to_user_database = getenv("HOME");
        path_to_user_database += "/.taskmgr/" + username + ".db";
        return path_to_user_database;
        // Completed
    }
    /*
        The following function loads the last added entry into our vector tuple.
        Note that it also fetches the task creation timestamp and deadline as well
        which is stored in a seperate database.
    */
    bool loadLastEntry()
    {
        database lastEntryLoader(getCurrentDatabasePath());
        string selectQuery = "SELECT id, task, priority, creation_date, deadline FROM " + username + " where id == ?;";
        try{
            lastEntryLoader << selectQuery << taskCounter // taskCounter has id of last inserted entry
                        >> [&](unsigned int id, string userTask, unsigned int taskPriority, string creationDate, string deadline){
                            TskVT.push_back(tuple <unsigned int, string, unsigned int> (id, userTask, taskPriority));
                            timeStampVector.push_back(creationDate);
                            deadlineVector.push_back(deadline);
                        };
        }
        catch(const exception& e)
        {
            cout << e.what() << endl;
            return false;
        }
        return true;
        // Completed
    }
    /*
        This function is responsible to load the user's task from the database
        and store them in a vector tuple. This function is executed
        everytime the user logs in. This has a seperate functionality than
        the loadLastEntry() function.
    */
    void LoadTask()
    {
        // TODO: Load user's task in the vector tuple
        string loadTaskQuery = "SELECT * FROM " + username + ";";
        string test = getCurrentDatabasePath();
        database loadingDB(getCurrentDatabasePath());
        loadingDB << loadTaskQuery
                >>[&](unsigned int id, string userTask, unsigned int taskPriority, string creationDate, string deadline){
                    TskVT.push_back(tuple <unsigned int, string, unsigned int> (id, userTask, taskPriority));
                    timeStampVector.push_back(creationDate);
                    deadlineVector.push_back(deadline);
                };
    }

    /*
        Does what it says. Adds a task into the user's database and calls the loadLastEntry()
        function.
    */

    void addTask()
    {
        string sqlInsertTask = "INSERT INTO " + username + " (task, priority, creation_date, deadline) VALUES (?, ?, ?, ?);";
        string newTask, timestamp, deadline, currentTimestamp; // Temporary string to hold values
        unsigned short priority;
        cout << "Input field: ";
        getline(cin >> ws, newTask);
        cout << "Task priority: ";
        cin >> priority;
        time_t result = time(nullptr);
        currentTimestamp = asctime(localtime(&result));
        currentTimestamp.erase(remove(currentTimestamp.begin(), currentTimestamp.end(), '\n'), currentTimestamp.end());
    TimestampAgain:
        cout << "Set a deadline - MM/DD/YYYY\n";
        getline(cin >> ws, deadline);
        if (!validateDeadline(deadline))
        {
            cout << "Invalid Timestamp\n";
            goto TimestampAgain;
        }
        database currentDatabase(getCurrentDatabasePath());
        currentDatabase << sqlInsertTask
                        << newTask
                        << priority
                        << currentTimestamp
                        << deadline;
        loadLastEntry(); // Loads the recently added task in the vector
    }
    void printUserTask()
    {
        bprinter::TablePrinter tp(&std::cout);
        tp.AddColumn("id", 10);
        tp.AddColumn("Task", 70);
        tp.AddColumn("Priority", 10);
        tp.AddColumn("Creation Date", 40);
        tp.AddColumn("Deadline", 40);

        tp.PrintHeader();
        auto TimeStampIter = timeStampVector.begin(), deadlineIter = deadlineVector.begin();
        for (auto i = TskVT.begin(); i != TskVT.end() && TimeStampIter != timeStampVector.end() && deadlineIter != deadlineVector.end(); i++)
        {
            tp << get<0>(*i) << get<1>(*i) << get<2>(*i) << *TimeStampIter << *deadlineIter;
        }
        tp.PrintFooter();
    }
    void removeTask()
    {
    }
    void updateTask()
    {
    }

    // Driver code to manage tasks
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
        database userConn(getCurrentDatabasePath());
        // TODO: Create user's table
        string sqlQueryDatabase = "CREATE TABLE if not exists " + username + " ("
                                                                             "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, task TEXT, priority INTEGER, "
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
        printUserTask();
        taskManaging();
    }

    ~TaskManager()
    {
        // This does nothing since every variable declared is on stack.
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
        system("clear");
        char input;
        cout << "\t\t\t\tWelcome to Task Manager\n"
             << "\t\t\t\t1. Login\n"
             << "\t\t\t\t2. Sign up\n"
             << "\t\t\t\tInput: ";
        cin >> input;
        switch(input)
        {
            case '1':
                task.Authentication();
                break;
            case '2':
                task.AccountCreation();
                break;
        }
    }
    return 0;
}
