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

class TaskManager
{
    string username, passwd;
    int taskCounter;
    typedef vector<tuple<unsigned int, string, unsigned int, string, string>> TaskVector;
    vector<string> timeStampVector, deadlineVector;
    vector<unsigned int> TaskID;
    TaskVector TskVT; // Stored Task ID, Task string, and task priority

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
        string selectQuery = "SELECT id, task, priority, creation_date, deadline FROM " + username + " WHERE id ="
                                                                                                     "(SELECT max(id) FROM " +
                             username + ");";
        try
        {
            lastEntryLoader << selectQuery // taskCounter has id of last inserted entry
                >> [&](unsigned int id, string userTask, unsigned int taskPriority, string creationDate, string deadline)
            {
                TskVT.push_back(tuple<unsigned int, string, unsigned int, string, string>(id, userTask, taskPriority, creationDate, deadline));
                TaskID.push_back(id);
                // timeStampVector.push_back(creationDate);
                // deadlineVector.push_back(deadline);
            };
        }
        catch (const exception &e)
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
        string loadTaskQuery = "SELECT * FROM " + username + " ORDER BY priority DESC;"; // Loads based on highest priority
        string test = getCurrentDatabasePath();
        database loadingDB(getCurrentDatabasePath());
        loadingDB << loadTaskQuery >> [&](unsigned int id, string userTask, unsigned int taskPriority, string creationDate, string deadline)
        {
            TskVT.push_back(tuple<unsigned int, string, unsigned int, string, string>(id, userTask, taskPriority, creationDate, deadline));
            TaskID.push_back(id);
            // timeStampVector.push_back(creationDate);
            // deadlineVector.push_back(deadline);
        };
        taskCounter = TskVT.size();
    }

    /*
        Does what it says. Adds a task into the user's database and calls the loadLastEntry()
        function.
    */

    void addTask()
    {
        taskCounter++;
        string sqlInsertTask = "INSERT INTO " + username + " (task, priority, creation_date, deadline) VALUES (?, ?, ?, ?);";
        string newTask, timestamp, deadline, currentTimestamp; // Temporary string to hold values
        unsigned short priority;
        cout << "\t\t\t\tInput field: ";
        getline(cin >> ws, newTask);
        cout << "\t\t\t\tTask priority: ";
        cin >> priority;
        time_t result = time(nullptr);
        currentTimestamp = asctime(localtime(&result));
        currentTimestamp.erase(remove(currentTimestamp.begin(), currentTimestamp.end(), '\n'), currentTimestamp.end());
    TimestampAgain:
        cout << "\t\t\t\tSet a deadline - MM/DD/YYYY: ";
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
        sort(TskVT.begin(), TskVT.end(), [](auto const &t1, auto const &t2)
             { return get<2>(t1) > get<2>(t2); });
    }
    /*
        This function uses the betty printer library (include link here)
        to print out the tasks in a row and column format, just like how
        MySQL displays data.
    */
    void printUserTask()
    {
        bprinter::TablePrinter tp(&std::cout);
        tp.AddColumn("id", 10);
        tp.AddColumn("Task", 70);
        tp.AddColumn("Priority", 10);
        tp.AddColumn("Creation Date", 40);
        tp.AddColumn("Deadline", 40);
        tp.PrintHeader();
        // auto TimeStampIter = timeStampVector.begin(), deadlineIter = deadlineVector.begin();
        for (auto i = TskVT.begin(); i != TskVT.end(); i++)
        {
            tp << get<0>(*i) << get<1>(*i) << get<2>(*i) << get<3>(*i) << get<4>(*i);
        }
        tp.PrintFooter();
    }
    // Validates that Task ID exists or not
    bool validateTaskID(int taskId)
    {
        for (auto i = TaskID.begin(); i != TaskID.end(); i++)
        {
            if (*i == taskId)
            {
                return true;
            }
        }
        return false;
    }
    void removeTask()
    {
        short int taskId, input;
        string deleteQuery = "DELETE FROM " + username + " WHERE id = ?;";
        database db(getCurrentDatabasePath());
        bool exitFlag = false;
        while (1)
        {
            system("clear");
            cout << "\t\t\t\tProvide an option: \n"
                 << "\t\t\t\t1. Delete Task\n"
                 << "\t\t\t\t2. Exit\n";
            cin >> input;
            switch (input)
            {
            case 1:
            DELETETASKAGAIN:
                system("clear");
                printUserTask();
                cout << "You can input '0' to return\n";
                cout << "Input the task ID: ";
                cin >> taskId;
                if (taskId == 0)
                    break;
                else if (!validateTaskID(taskId))
                    goto DELETETASKAGAIN;
                try
                {
                    db << deleteQuery << taskId;
                }
                catch (const exception &e)
                {
                    cout << e.what() << endl;
                }
                // Delete all vector tuple entries and fill it again by calling LoadTask().
                TskVT.clear();
                TaskID.clear();
                LoadTask();
                taskCounter--;
                break;
            case 2:
                exitFlag = true;
                break;
            default:
                cout << "\t\t\t\tInvalid option provided\n";
                break;
            }
            if (exitFlag)
                break;
        }
    }
    /*
        This function updates the user task based on the task ID
    */
    void updateTask()
    {
        bool exitFlag = false;
        while (1)
        {
            string updatedTask, updateSQL = "UPDATE " + username + " SET task = ? WHERE id = ?;",
                                updatePriority = "UPDATE " + username + " SET priority = ? WHERE id = ?;",
                                updateDeadline = "UPDATE " + username + " SET deadline = ? WHERE id = ?;",
                                newDeadline;
            short int taskID, newPriority;
            database db(getCurrentDatabasePath());
            system("clear");
            cout << "\t\t\t\tSpecify the field to update\n";
            cout << "\t\t\t\t1. Task\n"
                 << "\t\t\t\t2. Priority\n"
                 << "\t\t\t\t3. Deadline\n"
                 << "\t\t\t\t4. Exit"
                 << endl;
            short int input;
            cin >> input;
            switch (input)
            {
            case 1:
            inputTaskIdAgain:
                system("clear");
                printUserTask();
                cout << "\t\t\t\tInput the task ID: ";
                cin >> taskID;
                if (!validateTaskID(taskID))
                    goto inputTaskIdAgain;
                cout << "\t\t\t\tInput the updated task: ";
                getline(cin >> ws, updatedTask);
                try
                {
                    db << updateSQL << updatedTask << taskID;
                }
                catch (const exception &e)
                {
                    cout << e.what() << endl;
                }
                TskVT.clear();
                TaskID.clear();
                LoadTask();
                cout << "\t\t\t\tTask Updated Successfully!\n";
                break;
            case 2:
            PriorityAgain:
                system("clear");
                printUserTask();
                cout << "\t\t\t\tInput the task ID: ";
                cin >> taskID;
                cout << "\t\t\t\tInput the new priority: ";
                cin >> newPriority;
                if (!validateTaskID(taskID))
                    goto PriorityAgain;
                try
                {
                    db << updatePriority << newPriority << taskID;
                }
                catch (const exception &e)
                {
                    cout << e.what() << endl;
                }
                TskVT.clear();
                TaskID.clear();
                LoadTask();
                break;
            case 3:
            DeadlineAgain:
                system("clear");
                printUserTask();
                cout << "\t\t\t\tInput the task ID: ";
                cin >> taskID;
                cout << "\t\t\t\tInput the new deadline: ";
                getline(cin >> ws, newDeadline);
                if (!validateTaskID(taskID) && !validateDeadline(newDeadline))
                    goto DeadlineAgain;
                try
                {
                    db << updateDeadline << newDeadline << taskID;
                }
                catch (const exception &e)
                {
                    cout << e.what() << endl;
                }
                TskVT.clear();
                TaskID.clear();
                LoadTask();
                break;
            case 4:
                exitFlag = true;
                break;
            }
            if (exitFlag)
                break;
        }
    }
    /*
    Prints a month of dates (1-31) and shows the task next to them
    if there are any with their deadlines provided. This could get tricky
    */
    void CalendarViewOfTheMonth()
    {
        // TODO: Implement a calendar view
    }
    // Driver code to manage tasks
    void taskManaging()
    {
        bool exitFlag = false;
        while (1)
        {
            system("clear");
            printUserTask();
            cout << setw(40) << "Hello " + username + '\n'
                << setw(40) << "What is on your mind?\n";
            cout << setw(40) << "1. Add Task\n"
                 << setw(40) << "2. Remove Task\n"
                 << setw(40) << "3. Update task\n"
                 << setw(40) << "4. Exit\n\n"
                 << setw(40) << "Input: ";
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
            case 4:
                exitFlag = true;
                break;
            default:
                cout << "\t\t\t\tInvalid option provided!\n";
                sleep(2);
                break;
            }
            if (exitFlag)
            {
                TskVT.clear();
                TaskID.clear();
                username.clear();
                passwd.clear();
                break;
            }
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
        string tempUser;
        database dbConn(path_to_db);
    CreationAgain:
        system("clear");
        cout << setw(70) << "Welcome to Task Manager account creation!\nInput your username: ";
        getline(cin >> ws, username);
        try
        {
            dbConn << "SELECT username FROM users WHERE username == ?;" << username >> tempUser;
            if (tempUser == username)
            {
                cout << setw(10) << "Username already exists!!\n";
                sleep(3);
                goto CreationAgain;
            }
        }
        catch (const exception &e)
        {
            cout << setw(70) << "Username is available\n";
        }
        SetEcho(false);
        cout << setw(70) << "Please choose a password: ";
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
        dbConn << sql_query
               << username
               << pass_hash;
        // TODO: Create user's database file to hold tasks
        database userConn(getCurrentDatabasePath());
        // TODO: Create user's table
        string sqlQueryDatabase = "CREATE TABLE if not exists " + username + " ("
                                                                             "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, task TEXT, priority INTEGER, "
                                                                             "creation_date TEXT NOT NULL, deadline TEXT NOT NULL);";
        userConn << sqlQueryDatabase;
        cout << setw(70) << "Account created successfully\n";
        sleep(2);
    }

    void Authentication() // Function responsible for signing users in
    {
    LoginAgain:
        system("clear");
        cout << setw(70) << "Input your username: ";
        getline(cin >> ws, username);
        if (!Validate(username))
        {
            cout << "Username has invalid characters!\n";
            exit(1);
        }
        SetEcho(false);
        cout << setw(70) << "Input your password: ";
        getline(cin >> ws, passwd);
        SetEcho();
        cout << endl;
        string pass_hash = sha256(passwd);
        passwd.clear();
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
    // Start main program
    TaskManager task;
    while (1)
    {
        bool exitFlag = false;
        system("clear");
        char input;
        cout << setw(70) << "Welcome to Task Manager\n"
             << setw(60) << "1. Login\n"
             << setw(62) << "2. Sign up\n"
             << setw(60) << "3. Exit\n\n"
             << setw(60) << "Input: ";
        cin >> input;
        switch (input)
        {
        case '1':
            task.Authentication();
            break;
        case '2':
            task.AccountCreation();
            break;
        case '3':
            exitFlag = true;
            break;
        default:
            cout << setw(70) << "Invalid option provided!\n";
            sleep(2);
            system("clear");
            break;
        }
        if (exitFlag)
        {
            cout << setw(70) << "Goodbye!\n";
            sleep(2);
            break;
        }
    }
    return 0;
}
