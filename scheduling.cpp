
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

using namespace std;


struct event
{
    string type;
    int cpuTime;
};
struct process
{
    bool running = false;
    bool blocked = false;
    bool ready = false;

    int eyeD;
    int startTime;
    int arrivalTime;

    queue<event*> actions;
 };




/*Reading in processes into a vector, each process has a queue of all the requests*/
void readFile(vector<process*> &list, int &cores)
{
    string data;
    event *add;
    process *temp;

    bool isNCore = false;
    bool isStart = false;
    bool isEye = false;
    bool ssD = false;
    bool tty = false;
    bool isCore = false;

    int numProcesses = 0;

    while(cin >> data)
    {
        // cout << data;
        if(isNCore)
        {
            cores = stoi(data);
            isNCore = false;
        }    
        else if(isStart)
        {
            temp = new process;
            temp->startTime = stoi(data);
            list.push_back(temp);
            isStart = false;
        }
        else if(isEye)
        {
            temp->eyeD = stoi(data);
            isEye = false;
        }
        else if(ssD)
        {
            add = new event;
            add->type = "SSD";
            add->cpuTime = stoi(data);

            temp->actions.push(add);
            ssD = false;
        }
        else if(tty)
        {
            add = new event;
            add->type = "TTY";
            add->cpuTime = stoi(data);

            temp->actions.push(add);
            tty = false;
        }
        else if(isCore)
        {
            add = new event;
            add->type = "CORE";
            add->cpuTime = stoi(data);

            temp->actions.push(add);
            isCore = false;
        }
        
        if(data == "NCORES")                                    
        {
            isNCore = true;
        }
        else if(data == "START")                        //check if list is empty 
        {
            isStart = true;
        }
        else if(data == "PID")                        //check if list is empty 
        {
            isEye = true;
        }
        else if(data == "CORE")                        //check if list is empty 
        {
            isCore = true;
        }
        else if(data == "TTY")                        //check if list is empty 
        {
            tty = true;
        }
        else if(data == "SSD")                        //check if list is empty 
        {
            ssD = true;
        }
        else if(data == "END") //-999 will represent END
        {
            temp = new process;
            temp->eyeD = -999;

            list.push_back(temp);
        }
    }
}
void processTable(vector<process*> &list)
{
    cout << "Process Table:\n";
    for(int i =0; i <list.size() - 1; i++)
    {
        if(list[i]->blocked == true)
        {
            cout << "Process " << list[i]->eyeD << " is BLOCKED.\n";
        }
        else if(list[i]->running == true)
        {
            cout << "Process " << list[i]->eyeD << " is RUNNING.\n";
        }
        else if(list[i]->ready == true)
        {
            cout << "Process " << list[i]->eyeD << " is READY.\n";
        }
    }
}


/*Goes through interactive and noninteractive queue and checks 
    if possible to push to core, also sets status when pushed*/
void checkReady(float &time, queue<process*> &NI, queue<process*> &inT, queue<process*> &core, 
                int &freeCores, float &coreTime, vector<process*> &list)
{
    if(freeCores > 0)
    {
        if(!(inT.empty()) && time >= inT.front()->startTime)
        {
            core.push(inT.front());
            cout << "Process " << inT.front()->eyeD << " sent to core at " << time << " ms.\n";
            processTable(list);
            coreTime += inT.front()->actions.front()->cpuTime;
            core.front()->arrivalTime = time;
            freeCores--;
            inT.pop();

            core.front()->blocked = false;
            core.front()->running = true;
            core.front()->ready = false;
        }
        else if(!(NI.empty()) && time >= NI.front()->startTime)
        {
            core.push(NI.front());
            cout << "\nProcess " << NI.front()->eyeD << " sent to core at " << time << " ms.\n";
            processTable(list);
            coreTime += NI.front()->actions.front()->cpuTime;
            core.front()->arrivalTime = time;
            freeCores--;
            NI.pop();

            core.front()->blocked = false;
            core.front()->running = true;
            core.front()->ready = false;
        }
    }
}

/*Checks core and pushing to ssd or tty when ready, 
    updates ssd acceses and core time, updates process state whenever moved*/
void checkRunning(float &time, queue<process*> &core, int &freeCores, queue<process*> &ssd, queue<process*> &tty, int &ssdAccesses, float &ssdTime, vector<process*> &list)
{

    if(!(core.empty()) && (core.front()->arrivalTime + core.front()->actions.front()->cpuTime) <= time)
    {
        // cout << "303" << endl;
        core.front()->actions.pop();
        // cout << "305" << endl;
        if(core.front()->actions.empty())
        {
            cout << "\nProcess " << core.front()->eyeD << " terminates at " << time << " ms." << endl;
            
            core.front()->blocked = false;
            core.front()->running = false;
            core.front()->ready = false;
            processTable(list);

            core.pop();
            freeCores++;
        }
        else
        {
            // cout << "299" << endl;
            if(core.front()->actions.front()->type == "SSD")
            {
                ssd.push(core.front());
                ssdAccesses++;
                ssdTime += core.front()->actions.front()->cpuTime;
                core.front()->arrivalTime = time;

                core.pop();
                freeCores++;

                ssd.front()->blocked = true;
                ssd.front()->running = false;
                ssd.front()->ready = false;
                // cout << "311" << endl;
            }
            else if(core.front()->actions.front()->type == "TTY")
            {
                tty.push(core.front());
                core.front()->arrivalTime = time;

                core.pop();
                freeCores++;

                tty.front()->blocked = true;
                tty.front()->running = false;
                tty.front()->ready = false;
                // cout << "324" << endl;
            }
        }
    }
}

/*Checks ssd and tty for conditions to move, 
    updates process state whenever moved*/
void checkBlocked(float &time, queue<process*> &ssd, queue<process*> &tty, queue<process*> &NI, queue<process*> &inT)
{
    // cout << "337\n";
    // cout << ssd.front()->arrivalTime << "arived\n";
    // cout << ssd.front()->actions.front()->cpuTime << "needs" << endl;
    if(!(ssd.empty()) && ((ssd.front()->arrivalTime + ssd.front()->actions.front()->cpuTime) <= time))
    {
        // cout << "340" << endl;
        ssd.front()->actions.pop();
        // cout << "342" << endl;
        if(ssd.front()->actions.empty())
        {
            // cout << ssd.front()->eyeD << " terminated @ " << time << endl;
            ssd.front()->blocked = false;
            ssd.front()->running = false;
            ssd.front()->ready = false;
            ssd.pop();
        }
        else
        {
            if(ssd.front()->actions.front()->type == "CORE") // maybe try pushing straight to core
            {
                NI.push(ssd.front());
                
                ssd.front()->blocked = false;
                ssd.front()->running = false;
                ssd.front()->ready = true;
                ssd.pop();
            }
        }
    }
    // cout << "360" << endl;
    if(!(tty.empty()) && ((tty.front()->arrivalTime + tty.front()->actions.front()->cpuTime) <= time))
    {
        tty.front()->actions.pop();
        if(tty.front()->actions.empty())
        {
            cout << tty.front()->eyeD << " terminated @ " << time << endl;
            tty.front()->blocked = false;
            tty.front()->running = false;
            tty.front()->ready = false;
            tty.pop();
        }
        else
        {
            if(tty.front()->actions.front()->type == "CORE") // maybe try pushing straight to core
            {
                inT.push(tty.front());
                
                tty.front()->blocked = false;
                tty.front()->running = false;
                tty.front()->ready = true;
                tty.pop();
           }
        }
    }

}


/*Goes through vector of processes and pushes them into queues at their start time, 
    and continously checks queues to find request until all queues ar empty*/
void initiateProcesses( vector<process*> &list, int nCores,
                    queue<process*> &core, queue<process*> &NI, queue<process*> &ssd, queue<process*> &tty,
                    queue<process*> &inT, float &clock, int &ssdAccesses, float &coreTime, float &ssdTime)
{
    int numFree = nCores;
    int starts = 0;
    bool initiate = false;

    while (starts < list.size()-1)
    {
        for(int i =0; i < list.size() - 1; i++)
        {
            if(list[i]->startTime == clock)
            {
                NI.push(list[i]);
                starts++;
            }
        }
        checkReady(clock, NI, inT, core, numFree, coreTime, list);
        checkRunning(clock, core, numFree, ssd, tty, ssdAccesses, ssdTime, list);
        checkBlocked(clock, ssd, tty, NI, inT);
        clock++;
    }
    do
    {
        checkReady(clock, NI, inT, core, numFree, coreTime, list);
        checkRunning(clock, core, numFree, ssd, tty, ssdAccesses, ssdTime, list);
        checkBlocked(clock, ssd, tty, NI, inT);
        clock++;
    }while(!(core.empty() && NI.empty() && inT.empty() && ssd.empty() && tty.empty()));
}
int main()
{
    float clock = 0;
    int nCores = 0;
    float coreTime = 0;
    int completed = 0;
    int ssdAccesses = 0;
    float ssdTime = 0;

    vector<process*> processes;
    readFile(processes, nCores);
    
    queue<process*> core;
    queue<process*> ssd;
    queue<process*> tty;
    queue<process*> NI;
    queue<process*> inT;

    initiateProcesses(processes, nCores, core, NI, ssd, tty, inT, clock, ssdAccesses, coreTime, ssdTime);

    cout << "\nTotal Elapsed time: " << clock << " ms" << endl;
    cout << "Number of processes that completed: " << processes.size() - 1 << endl;
    cout << "Totatl number of SSD accesses: " << ssdAccesses << endl;
    cout <<"Avergage number of busy cores: " << coreTime / clock << endl;
    cout << "SSD utilization: " << ssdTime / clock << endl;

}
