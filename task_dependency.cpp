/* Code Created by Ali Asghar*/

#include <iostream>
#include <string>

using namespace std;

class Task{
private:
    string name;
    bool* dependencies;		// A dynamic array to store the dependencies of the task
    int numDependencies;		// The number of dependencies of the task
    bool completed;			// A flag to indicate whether the task is completed or not

public:
    Task(string n){
        name = n;
        dependencies = NULL;
        numDependencies = 0;
        completed = false;
    }

    ~Task(){
        delete[] dependencies;
    }

	 // Function to add a dependency to the task

    void addDependency(string taskName){
        bool* newDependencies = new bool[numDependencies + 1];
        for (int i = 0; i < numDependencies; i++){
            newDependencies[i] = dependencies[i];
        }
        newDependencies[numDependencies] = false;

        delete[] dependencies;
        dependencies = newDependencies;
        numDependencies++;
    }

    bool isCompleted(){		// Function to check if the task is completed
        return completed;
    }

    void complete(){		// Method to mark the task as completed
        completed = true;
    }

    bool canBeCompleted(){		// Function to check if the task can be completed based on its dependencies
        if (numDependencies == 0){
            return true;
        }

        for (int i = 0; i < numDependencies; i++){
            if (!dependencies[i]) {
                return false;
            }
        }

        return true;
    }

    string getName(){
        return name;
    }
};

class TaskManager{
private:
    Task** tasks;
    int numTasks;
    int maxTasks;

public:
    TaskManager(){
        maxTasks = 10;
        numTasks = 0;
        tasks = new Task*[maxTasks];
    }

    ~TaskManager(){
        for (int i = 0; i < numTasks; i++){
            delete tasks[i];
        }
        delete[] tasks;
    }

    void createTask(string name){
        if (numTasks == maxTasks){
            Task** newTasks = new Task*[maxTasks * 2];		// If the TaskManager is full, double its size
            for (int i = 0; i < numTasks; i++) {
                newTasks[i] = tasks[i];
            }
            delete[] tasks;
            tasks = newTasks;
            maxTasks *= 2;
        }

        Task* task = new Task(name);
        tasks[numTasks] = task;
        numTasks++;
    }
	
	// Method to add a dependency between two tasks
	
    void addDependency(string taskName, string dependentTaskName){
        int taskIndex = -1;
        int dependentTaskIndex = -1;
        
        // Find the indices of the tasks with the given names
        
        for (int i = 0; i < numTasks; i++) {
            if (tasks[i]->getName() == taskName) {
                taskIndex = i;
            } else if (tasks[i]->getName() == dependentTaskName){
                dependentTaskIndex = i;
            }
        }

        if (taskIndex == -1 || dependentTaskIndex == -1){
            cout << "Invalid task name" << endl;
            return;
        }

        tasks[dependentTaskIndex]->addDependency(taskName);
    }

    void completeTask(string name){
        int taskIndex = -1;
        for (int i = 0; i < numTasks; i++){
            if (tasks[i]->getName() == name){
                taskIndex = i;
            }
        }

        if (taskIndex == -1){
            cout << "Invalid task name" << endl;
            return;
        }

        Task* task = tasks[taskIndex];
        if (!task->canBeCompleted()){
            cout << "Cannot complete task " << name << " yet" << endl;
            return;
        }

        task->complete();
        cout << "Task " << name << " completed" << endl;

        for(int i = taskIndex + 1; i < numTasks; i++){
		if (tasks[i]->canBeCompleted()) {
		cout << "Task " << tasks[i]->getName() << " can now be completed" << endl;
			}
		}
	}
};


int main(){
TaskManager manager;

manager.createTask("Get car washed");
manager.createTask("Go to market");
manager.createTask("Cook dinner");

manager.addDependency("Go to market", "Get car washed");

manager.completeTask("Get car washed");
manager.completeTask("Go to market");
manager.completeTask("Cook dinner");

}


