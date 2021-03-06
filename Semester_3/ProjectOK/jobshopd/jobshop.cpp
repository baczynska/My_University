#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <limits>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <cstdlib>

using namespace std;

#define NR 10

string fileName;

int max (int a, int b)
{
    return a > b ? a : b;
}

struct Instance
{
    int jobs, machines, activities;

    vector<vector<int>> order, pTime;
};

/*struct Ans
{

};*/

void printInstance(Instance &instance)
{
    int jobs = instance.jobs;
    int machines = instance.machines;

    vector<vector<int>> order = instance.order;
    vector<vector<int>> pTime = instance.pTime;

    cout << "Jobs: " << jobs << "\nMachines: " << machines << endl;

    for (int i = 0; i < order.size(); i++)
    {
    	cout << "Job " << i << ":\t";
    	for (int j = 0; j < order[i].size(); j++)
    	{
    		cout<< order[i][j] <<"/" << pTime[i][j] <<"\t";
    	}
        cout << endl;
    }
}

Instance initBeasley(string fileName)
{
    ifstream file;
    int jobs, machines;
    int x, y;

    file.open(fileName.c_str());
    if (!file.good())
    {
    	cout << "ERROR: File not opened" << endl;
    }

    file >> jobs >> machines;
    
    vector<vector<int>> order(jobs); // for Jobs
    vector<vector<int>> pTime(jobs); // for Jobs

    for (int i = 0; i < jobs; i++)
    {
    	pTime[i].resize(machines);
        order[i].resize(machines);

        for (int j = 0; j < machines; j++)
    	{
    		file >> order [i][j] >> pTime [i][j];
    	}
    }
    file.close();

    int activities = jobs * machines;

    Instance instance;
    instance.jobs = jobs;
    instance.machines = machines;
    instance.pTime = pTime;
    instance.order = order;
    instance.activities = activities;

    return instance;
}

Instance initTaillard(string fileName)
{
    ifstream file;
    int jobs, machines;
    
    file.open(fileName.c_str());
    if (!file.good())
	{
        cout<<"ERROR: File not opened"<<endl;
    }
    
    file >> jobs >> machines;
    file.ignore(numeric_limits<streamsize>::max(), '\n');

    file.ignore(numeric_limits<streamsize>::max(), '\n');
    
    vector<vector<int>> pTime(jobs);
    
    for (int i = 0; i < jobs; i++)
    {
        pTime[i].resize(machines);
        for (int j = 0; j < machines; j++)
        {
            file >> pTime[i][j];
        }
    }

    file.ignore(numeric_limits<streamsize>::max(), '\n');
    file.ignore(numeric_limits<streamsize>::max(), '\n');

    vector<vector<int>> order(jobs);
    for (int i = 0; i < jobs; i++)
    {
        order[i].resize(machines);
        for (int j = 0; j < machines; j++)
        {
            int x;
            file >> x;
            order[i][j] = x - 1;
        }
    }

    int activities = jobs * machines;
    
    Instance instance;
    instance.jobs = jobs;
    instance.machines = machines;
    instance.pTime = pTime;
    instance.order = order;
    instance.activities = activities;

    return instance;
}

class Solver
{
    const Instance * instance;

    vector<int> jProgT, jProg, machProgT;

    vector<int> sol, bestSol, finalSol;


public:
    
    Solver(const Instance * inst) : instance(inst) 
    {
        if (jProgT.size() == 0)
            jProgT.resize(instance->jobs);
        if (jProg.size() == 0)
            jProg.resize(instance->jobs);
        if (machProgT.size() == 0)
            machProgT.resize(instance->machines);
    }

    void schedule()
    {
        for (int i = 0; i < instance->activities; i++)
        {
            int job = chooseJob();
            scheduleNext(job);
        }
    }

    int schedule2()
    {
        for (int i = 0; i < instance->activities; i++)
        {
            int job = rand() % instance->jobs;
            sol.push_back(job);
            scheduleNext(job);
        }
        int temp = makeSpan();
        cout << temp << endl;
        return temp;
    }

    void solve2()
    {
        int bestMakeSpan;
        for (int i = 0; i < instance->machines; i++)
        {
            bestMakeSpan = numeric_limits<int>::max();
            for (int j = 0; j < NR; j++)
            {
                int temp = schedule2();
                if (temp < bestMakeSpan)
                {
                    bestMakeSpan = temp;
                    bestSol = sol;
                }
                restart();
            }
            for (int k = 0; k < instance->jobs; k++)
            {
                finalSol.push_back(bestSol[finalSol.size()+k]);
            }
        }

        if (finalSol.size() != instance->activities) throw string("ERROR: Saving final solution failed.");

        saveAns(finalSol, bestMakeSpan);
    }

    void saveAns(vector<int> finalSol, int bestMakeSpan)
    {
        restart();
        vector<vector<int>> solution;
        solution.resize(instance->jobs);
        
        for (int i = 0; i < finalSol.size(); i++)
        {
            int job = finalSol[i];
            int machine = nextMachine(job);
            int start = max (jProgT[job], machProgT[machine]);

            solution[job].push_back(start);
            scheduleNext(job);
        }

        int mkspn = makeSpan();

        if (mkspn != bestMakeSpan)
        {
            throw string("ERROR: Makespans don't match.");
        }
        //file << mkspn << endl;
        cout << mkspn << endl;

        for (int j = 0; j < solution.size(); j++)
        {
            for (int k = 0; k < solution[j].size(); k++)
            {
                //file << solution [j][k];
                cout << solution [j][k];
                if (k < (solution[j].size()-1))
                {
                    //file << "\t";
                    cout << "\t";
                }
            }
            //file << endl;
            cout << endl;
        }

        //file.close();
    }

    void restart()
    {
        jProgT.clear();
        jProgT.resize(instance->jobs);
        jProg.clear();
        jProg.resize(instance->jobs);
        machProgT.clear();
        machProgT.resize(instance->machines);
        sol.clear();
    }

    int makeSpan()
    {
        return *max_element(machProgT.begin(), machProgT.end());
    }

    void solve()
    {
        ofstream file; //file.open("output.txt");
        //fstream results; results.open("results.txt", ios::app)
        vector<vector<int>> solution;
        solution.resize(instance->jobs);

        int index = fileName.find_last_of("/\\");
        string fileName2 = fileName.substr(index+1);
        string str = "output/out_";
        str.append(fileName2);
        file.open(str.c_str());

        for (int i = 0; i < instance->activities; i++)
        {
            int job = chooseJob();
            int machine = nextMachine(job);
            int start = max (jProgT[job], machProgT[machine]);
            //cout << job << " " << start << endl;
            solution[job].push_back(start);
            scheduleNext(job);
        }
        
        int mkspn = makeSpan();
        file << mkspn << endl;
        cout << mkspn << endl;

        for (int j = 0; j < solution.size(); j++)
        {
            for (int k = 0; k < solution[j].size(); k++)
            {
                file << solution [j][k];
                cout << solution [j][k];
                if (k < (solution[j].size()-1))
                {
                    file << "\t";
                    cout << "\t";
                }
            }
            file << endl;
            cout << endl;
        }

        file.close();
    }

private:

    bool hasNextActivity(int job)
    {
        return jProg [job] < instance->machines;
    }
    
    int nextMachine(int job)
    {
        int nextActivity = jProg[job];
        if (nextActivity == instance->machines) return -1;
        return instance->order[job][nextActivity];
    }

    int chooseJob()
    {
        int earliestStart = numeric_limits<int>::max();
        int selectedJob;
        for (int job = 0; job < instance->jobs; job++)
        {
            if (!hasNextActivity(job)) continue;
            int machine = nextMachine(job);
            int start = max(jProgT[job], machProgT[machine]);
            if (start < earliestStart)
            {
                earliestStart = start;
                selectedJob = job;
            }
        }
        return selectedJob;
    }

    int nextActivityEnd(int job)
    {
        int nextActivity = jProg[job];
        int machine = instance->order[job][nextActivity];
        int duration = instance->pTime[job][nextActivity];
        int start = max (jProgT[job], machProgT[machine]);
        return start + duration;
    }

    void scheduleNext(int job)
    {
        int machine = nextMachine(job);
        int end = nextActivityEnd(job);
        jProg[job]++;
        machProgT[machine] = end;
        jProgT[job] = end;
        return;
    }
};

int main(int argc, char ** argv)
{
    srand(time(NULL));
    
    if (argc != 3)
    {
        printf("Usage: %s <format> <inputfile>\nWhere <format> is b for beasley and t for tailard\n", argv[0]);
    }
    int format = *argv[1];
    fileName = argv[2];
    Instance instance;
    
    if (format == 'b')
    {
        instance = initBeasley(fileName);
    }
    else if (format == 't')
    {
        instance = initTaillard(fileName);
    }
    else
    {
        printf("ERROR: Wrong format!\n");
    }

    Solver solver(&instance);
    /*try
    {
        solver.solve2();
    }
    catch(string e)
    {
        cout<<e<<endl;
        return -1;
    }*/
    //solver.solve();
}