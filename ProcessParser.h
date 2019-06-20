#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"


using namespace std;

class ProcessParser{
    private:
        std::ifstream stream;
        
    public:
        static string getCmd(string pid);
        static vector<string> getPidList();
        static std::string getVmSize(string pid);
        static std::string getCpuPercent(string pid);
        static long int getSysUpTime();
        static std::string getProcUpTime(string pid);
        static string getProcUser(string pid);
        static vector<string> getSysCpuPercent(string coreNumber = "");
        static float getSysActiveCpuTime(vector<string> values);
        static float getSysIdleCpuTime(vector<string> values);
        static float getSysRamPercent();
        static string getSysKernelVersion();
        static int getTotalThreads();
        static int getNumberOfCores();
        static int getTotalNumberOfProcesses();
        static int getNumberOfRunningProcesses();
        static string getOSName();
        static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
        static bool isPidExisting(string pid);
};


string ProcessParser::getCmd(string pid) {
    string line;

    ifstream stream;
    Util::getStream((Path::basePath() + pid + Path::cmdPath()), stream);
    getline(stream, line);
    return line;
}

vector<string> ProcessParser::getPidList() {
    DIR* dir;

    // scan /proc for all directories with numbers as their names and if found store it in a container
    vector<string> container;
    if(!(dir = opendir("/proc")))
        throw std::runtime_error(std::strerror(errno));
    
    while(dirent* dirp = readdir(dir)) {

        // check if not directory
        if(dirp->d_type != DT_DIR)
            continue;
        
        // check every character of the name to be digit
        if(all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), [](char c){ return std::isdigit(c); })) {
            container.push_back(dirp->d_name);
        }
    }

    // check if directory is closed
    if(closedir(dir))
        throw std::runtime_error(std::strerror(errno));
    
    return container;
}

string ProcessParser::getVmSize(string pid) {
    string line;

    // declare name to search in file
    string name = "VmData"; // in case of doubt, verify it from "cat /proc/pid#/status"

    float result;

    // open stream for specific file
    std::ifstream stream;
    Util::getStream((Path::basePath() + pid + Path::statusPath()), stream);

    while(getline(stream, line)) {

        // searching line by line
        if(line.compare(0, name.size(), name) == 0) {

            //slicing string line on ws for values using sstream
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);

            // conversion of kB -> MB
            result = stof(values[1])/float(1024);
            break;
        }
    }
    return to_string(result);
}

std::string ProcessParser::getCpuPercent(string pid) {
    string line;
    float result;

    // open stream for specific file
    std::ifstream stream;
    Util::getStream((Path::basePath() + pid + "/" + Path::statPath()), stream);

    getline(stream, line);

    string str = line;

    //slicing string line on ws for values using sstream
    istringstream buf(str);        
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    // acquring revelant time for calculation of CPU usage for selected process
    float utime = stof(ProcessParser::getProcUpTime(pid));

    float stime = stof(values[14]);
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float starttime = stof(values[21]);

    float uptime = ProcessParser::getSysUpTime();
    float freq = sysconf(_SC_CLK_TCK);

    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime/freq);
    result = 100.0*((total_time/freq)/seconds);

    return to_string(result);
}

long int ProcessParser::getSysUpTime() {
    string line;

    ifstream stream;
    Util::getStream((Path::basePath() + Path::upTimePath()), stream);
    getline(stream, line);
    string str = line;

    //slicing string line on ws for values using sstream
    istringstream buf(str);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    return stoi(values[0]);
}

std::string ProcessParser::getProcUpTime(string pid) {
    string line;
    float result;

    ifstream stream;
    Util::getStream((Path::basePath() + pid + "/" + Path::statPath()), stream);
    getline(stream, line);
    string str = line;

    //slicing string line on ws for values using sstream
    istringstream buf(str);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    // sysconf returns clock ticks of the host machine
    return to_string(float(stof(values[13])/sysconf(_SC_CLK_TCK)));
}

std::string ProcessParser::getProcUser(string pid) {
    string line;
    string result = "";
    string name = "Uid:";   // Uid = User ID

    ifstream stream;
    Util::getStream((Path::basePath() + pid + Path::statusPath()), stream);

    while(getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {

            //slicing string line for using sstream
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result = values[1];
            break;
        }
    }  
    // for getting user name
    Util::getStream("/etc/passwd", stream);
    name = ("x:" + result);

    // searching for the username
    while(getline(stream, line)) {
        if (line.find(name) != string::npos) {
            result = line.substr(0, line.find(":"));
            return result;
        }
    }
    return "";
}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber){
    // function gives CPU percent for entire CPU when no argument is passed, else specific CPU core
    string line;
    string name = "cpu" + coreNumber;
    int result;

    ifstream stream;
    Util::getStream((Path::basePath() + Path::statPath()), stream);

    while(getline(stream, line)) {
        if(line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);

            return values;
        }
    }
    return (vector<string> ());
}

float ProcessParser::getSysActiveCpuTime(vector<string> values) {
    return (stof(values[S_USER]) +
            stof(values[S_NICE]) +
            stof(values[S_SYSTEM]) +
            stof(values[S_IRQ]) +
            stof(values[S_SOFTIRQ]) +
            stof(values[S_STEAL]) +
            stof(values[S_GUEST]) +
            stof(values[S_GUEST_NICE]));
}

float ProcessParser::getSysIdleCpuTime(vector<string> values) {
    return (stof(values[S_IDLE]) + stod(values[S_IOWAIT]));
}

float ProcessParser::getSysRamPercent() {
    string line;
    string name1 = "MemAvailble:";
    string name2 = "MemFree:";
    string name3 = "Buffers:";
    int result;
    ifstream stream;

    Util::getStream((Path::basePath() + Path::memInfoPath()), stream);

    float total_mem = 0;
    float free_mem = 0;
    float buffers = 0;

    while(getline(stream, line)) {
        if(total_mem != 0 && free_mem != 0)
            break;
        
        if(line.compare(0, name1.size(), name1) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            total_mem = stof(values[1]);
        }

        if (line.compare(0, name2.size(), name2) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            free_mem = stof(values[1]);
        }

        if (line.compare(0, name3.size(), name3) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            buffers = stof(values[1]);
        }
    }

    // calculate usage
    return (float)(100.0 * (1 - (free_mem / (total_mem - buffers))));
}

string ProcessParser::getSysKernelVersion() {
    string line;
    string name = "Linux version";
    ifstream stream;
    Util::getStream((Path::basePath() + Path::versionPath()), stream);

    while (getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return values[2];
        }
    }
    return "";
}

int ProcessParser::getTotalThreads() {
    string line;
    int result = 0;
    string name = "Threads:";

    vector<string>_list = ProcessParser::getPidList();
    
    for (int i=0 ; i<_list.size();i++) {
        string pid = _list[i];

        // Get every process and read their number of threads
        ifstream stream;
        Util::getStream((Path::basePath() + pid + Path::statusPath()), stream);

        while (std::getline(stream, line)) {
            if (line.compare(0, name.size(), name) == 0) {
                istringstream buf(line);
                istream_iterator<string> beg(buf), end;
                vector<string> values(beg, end);
                result += stoi(values[1]);
                break;
            }
        }
        return result;
    }
}

int ProcessParser::getNumberOfCores() {
    // Get number of host cpu cores
    string line;
    string name = "cpu cores";
    ifstream stream;
    Util::getStream((Path::basePath() + "cpuinfo"), stream);

    while(getline(stream, line)) {

        // searching line by line
        if(line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);

            return stoi(values[3]);
        }
    }
    return 0;
}

int ProcessParser::getTotalNumberOfProcesses() {
    string line;
    int result = 0;
    string name = "processes";
    ifstream stream;
    Util::getStream((Path::basePath() + Path::statPath()), stream);

    while (getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;
}

int ProcessParser::getNumberOfRunningProcesses() {
    string line;
    int result = 0;
    string name = "procs_running";
    ifstream stream;
    Util::getStream((Path::basePath() + Path::statPath()), stream);

    while (getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;
}

string ProcessParser::getOSName() {
    string line;
    string name = "PRETTY_NAME=";

    ifstream stream;
    Util::getStream(("/etc/os-release"), stream);

    while (getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
              std::size_t found = line.find("=");
              found++;
              string result = line.substr(found);
              result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
              return result;
        }
    }
    return "";
}

string ProcessParser::PrintCpuStats(vector<string> values1, vector<string>values2) {
    float active_time = getSysActiveCpuTime(values2) - getSysActiveCpuTime(values1);
    float idle_time = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);
    float total_time = active_time + idle_time;
    float result = 100.0*(active_time / total_time);
    return to_string(result);
}

bool ProcessParser::isPidExisting(string pid) {
    vector<string> pidList = ProcessParser::getPidList();
    
    for(int i=0; i < pidList.size(); i++) {
        if(pidList[i] == pid)
            return true;
    }

    return false;
}