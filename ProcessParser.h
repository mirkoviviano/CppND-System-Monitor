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
        static std::string getCmd(std::string pid);
        static std::vector<std::string> getPidList();
        static std::string getVmSize(std::string pid);
        static std::string getCpuPercent(std::string pid);
        static long int getSysUpTime();
        static std::string getProcUpTime(std::string pid);
        static std::string getProcUser(std::string pid);
        static std::vector<std::string> getSysCpuPercent(std::string coreNumber = "");
        static float getSysRamPercent();
        static std::string getSysKernelVersion();
        static int getTotalThreads();
        static int getTotalNumberOfProcesses();
        static int getNumberOfRunningProcesses();
        static std::string getOSName();
        static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
        static bool isPidExisting(std::string pid);
};

std::string ProcessParser::getCmd(std::string pid){
    std::string line; 

    std::ifstream stream = Util::getStream(Path::basePath() + pid + Path::cmdPath());
    std::getline(stream, line);

    return line; 
}

std::vector<string> ProcessParser::getPidList(){
    DIR* dir;
    std::vector<string> container; 

    if(!(dir = opendir("/proc")))
        throw std::runtime_error(std::strerror(errno));

    while(dirent* dirp = readdir(dir)){
        // is this a directory?
        if(dirp->d_type != DT_DIR)
            continue;

        // Is every character of the name a digit?
        if(all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), [](char c){ return std::isdigit(c); })){
            container.push_back(dirp->d_name);
        }
    }

    //Validating process of directory closing
    if(closedir(dir))
        throw std::runtime_error(std::strerror(errno));

    return container;
}

std::string ProcessParser::getVmSize(std::string pid){
    std::string line; 

    std::string name = "VmData";
    std::string value; 
    float result; 

    std::ifstream stream = Util::getStream((Path::basePath() + pid + Path::statusPath());
    while(std::getline(stream, line)){
        if(line.compare(0, name.size(), name) == 0){
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            std::vector<string> values(beg, end);

            result = (stof(values[1])/flaot(1024*1024));
            break;
        }
    }
    return to_string(result);
}

std::string ProcessParser::getCpuPercent(std::string pid){
    std::string line; 
    std::string value; 
    float result;

    std::ifstream stream = Util::getStream((Path::basePath() + pid + Path::statPath());
    std::getline(stream, line);
    
    std::string nString = line; 
    std::istringstream buf(nString);
    std::istream_iterator<string> beg(buf), end;
    std::vector<string> values(beg, end);
    
    float utime     = stof(ProcessParser::getProcUpTime(pid));
    float stime     = stof(values[14]);
    float cutime    = stof(values[15]);
    float cstime    = stof(values[16]);
    float starttime = stof(values[21]);
    float uptime    = ProcessParser::getSysUpTime();
    float freq      = sysconf(_SC_CLK_TCK);

    float total_time = utime + stime + cutime + cstime; 
    float seconds    = uptime - (starttime/freq);
    
    result = 100.0*((total_time/freq)/seconds);
    return to_string(result);
}

long int getSysUpTime(){
    std::string line; 

    std::ifstream stream = Util::getStream((Path::basePath() + Path::upTimePath());
    std::getline(stream, line);

    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    std::vector<string> values(beg, end);

    return stoi(values[0]);

}

std::string ProcessParser::getProcUpTime(std::string pid){
    std::string line;
    std::string value;
    float result;

    std::ifstream stream = Util::getStream((Path::basePath() + pid + Path::statPath());
    std::getline(stream, line);

    std::istringstream buf(nString);
    std::istream_iterator<string> beg(buf), end;
    std::vector<string> values(beg, end);

    return to_string(float(stof(values[13])/sysconf(_SC_CLK_TCK)));
}

std::string ProcessParser::getProcUser(std::string pid){
    std::string line;
    std::string name = "Uid:"; 
    std::string value;
    std::string result = "";
    
    std::ifstream stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
    while(std::getline(stream, line)){
       if(line.compare(0, name.size(), name) == 0){
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            std::vector<string> values(beg, end);

            result = values[1];
        }
    }

    stream = Util::getStream("/etc/passwd");
    while(std::getline(stream, line)){
        if(line.find(name) != std::string::npos){
            result = line.substr(0, line.find(":"));
            return result;
        }
    }
    return "";
}

int ProcessParser::getNumberOfCores(){
    // Get the number of host cpu cores
    std::string line;
    std::string name = "cpu cores";
    std::ifstream stream = Util::getStream((Path::basePath() + "cpuinfo"));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            std::vector<string> values(beg, end);
            return stoi(values[3]);
        }
    }
    return 0;
}

std::vector<string> ProcessParser::getSysCpuPercent(std::string coreNumber){
    std::string line; 
    std::string name = "cpu" + coreNumber;
    std::ifstream stream = Util::getStream(Path::basePath() + Path::statPath());
    
    while(std::getline(stream, line)){
        if(line.compare(0, name.size(), name) == 0){
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            std::vector<string> values(beg, end);
            
            return values;
        }
    }

    return std::vector<string>();
}

float ProcessParser::getSysRamPercent(){
    std::string memAvailable = "MemAvailable:";
    std::string memFree      = "MemFree:";
    std::string buffers      = "Buffers:";
    std::string line;

    float totalMem = 0.0;
    float freeMem  = 0.0;
    float buffers  = 0.0;

    std::ifstream stream = Util::getStream(Path::basePath() + Path::memInfoPath());
    while(std::getline(stream, line)){
        if(line.compare(0, memAvailable.size(), memAvailable) == 0){
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            std::vector<string> values(beg, end);
            totalMem = stof(values[1]);
        }
        if(line.compare(0, memFree.size(), memFree) == 0){
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            std::vector<string> values(beg, end);
            freeMem = stof(values[1]);
        }
        if(line.compare(0, buffers.size(), buffers) == 0){
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            std::vector<string> values(beg, end);
            buffers = stof(values[1]);
        }
    }

    return float(100.0 * (1 - (freeMem / (totalMem - buffers))));
}

std::string ProcessParser::getSysKernelVersion(){
    std::string name = "Linux Version ";
    std::string line;

    std::ifstream stream = Util:getStream(Path::basePath() + Path::versionPath());

    while(std::getline(stream, line)){
        if(line.compare(0, name.size(), name) == 0){
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            std::vector<string> values(beg, end);
            return values[2];
        }
    }
    return "";
}

int ProcessParser::getTotalThreads(){
    std::string name = "Threads:";
    std::string line; 
    int result = 0; 

    for(auto pid : ProcessParser::getPidList()){
        std::ifstream stream = Util::getStream(Path::basePath() + pid + Path::statusPath());
        while(std::getline(stream, line)){
            if(line.compare(0, name.size(), name) == 0){
                std::istringstream buf(line);
                std::istream_iterator<string> beg(buf), end; 
                std::vector<string> values(beg, end);

                result += stoi(values[1]);
                break;
            }
        }
    }

    return result;
}

int ProcessParser::getTotalNumberOfProcesses(){
    std::string name = "processes";
    std::string line; 
    int result = 0;

    std::ifstream stream = Util::getStream(Path::basePath() + Path::statPath());
    while(std::getline(stream, line)){
        if(line.compare(0, name.size(), name) == 0){
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            std::vector<string> values(beg, end);

            result += stoi(values[1]);
            break;
        }
    }

    return result;
}

int ProcessParser::getNumberOfRunningProcesses(){
    std::string name = "procs_running";
    std::string line;
    int result = 0;

    std::istream stream = Util::getStream(Path::basePath() + Path::statPath());
    while(std::getline(stream, line)){
        if(line.compare(0, name.size(), name) == 0){
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            std::vector<string> values(beg, end);

            result += stoi(values[1]);
            break;
        }
    }

    return result;
}

std::string ProcessParser::getOSName(){
    std::string name = "PRETTY_NAME=";
    std::string line;

    std::ifstream stream = Util::getStream(("/etc/os-release"));
    while(std::getline(stream, line)){
        if(line.compare(0, name.size(), name) == 0){
            std::size_t found = line.find("=");
            found++;
            string result = line.substr(found);
            result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
            return result;
        }
    }
}

float get_sys_active_cpu_time(std::vector<string> values){
    return (stof(values[S_USER]) +
            stof(values[S_NICE]) +
            stof(values[S_SYSTEM]) +
            stof(values[S_IRQ]) +
            stof(values[S_SOFTIRQ]) +
            stof(values[S_STEAL]) +
            stof(values[S_GUEST]) +
            stof(values[S_GUEST_NICE]));
}

float get_sys_idle_cpu_time(std::vector<string>values){
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

std::string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2){
    float activeTime = get_sys_active_cpu_time(values2) - get_sys_active_cpu_time(values1);
    float idleTime   = get_sys_idle_cpu_time(values2)   - get_sys_idle_cpu_time(values1);
    float totalTime  = activeTime + idleTime;
    float result     = 100.0 * (activeTime / idleTime);
    
    return to_string(result);
}

bool ProcessParser::isPidExisting(std::string pid){
    std::vector<std::string> v = ProcessParser::getPidList();

    if (std::find(v.begin(), v.end(), pid) != v.end()){
        return true;
    }

    return false;
}