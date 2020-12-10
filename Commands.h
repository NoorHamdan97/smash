#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_
#include<signal.h>
#include <list>
#include <string>
#include <vector>
#include <iterator>
#include <bits/stdc++.h>
using namespace std;

#define COMMAND_ARGS_MAX_LENGTH (200)  //! How is this possible
#define COMMAND_MAX_ARGS (20)          //* Number of arguments
#define HISTORY_MAX_RECORDS (50)
#define NO_FG_COMMAND ((pid_t)-99)
#define MAX_COMMAND_LENGTH (80)



class JobsList;
class Command {
protected:
    // TODO: Add your data members
    const char *cmd_line_;
    JobsList *jobsList_;
    JobsList* timedoutJobs;

public:
    Command(const char *cmd_line);
    virtual ~Command() {}
    virtual void execute() = 0;
    // virtual void prepare();
    // virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}
    virtual ~BuiltInCommand() {}
};

//class JobsList;
class ExternalCommand : public Command {

public:
    ExternalCommand(const char *cmd_line) : Command(cmd_line) {}
    virtual ~ExternalCommand() {}
    void execute() override;
    void execute2(const char *cmd_line,char* comm);
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line): Command(cmd_line) {}
    virtual ~PipeCommand() {}
    void execute() override;
};


class RedirectionCommand : public Command {

    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line) : Command(cmd_line) {}
    virtual ~RedirectionCommand() {}
    void execute() override;
    // void prepare() override;
    // void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {

    // TODO: Add your data members public:
    static string history_;

public:
    ChangeDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {

public:
    GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {

public:
    ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~ShowPidCommand() {}
    void execute() override;
};

class lsCommand : public BuiltInCommand {
public:
    lsCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~lsCommand() {}
    void execute() override;
};

//class JobsList;
class QuitCommand : public BuiltInCommand {

    // TODO: Add your data members public:
public:
    QuitCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~QuitCommand() {}
    void execute() override;

};

class TimeOutCommand : public BuiltInCommand{
pid_t TOpid;
    // TODO: Add your data members
public:
    TimeOutCommand(const char *cmd_line,pid_t p): BuiltInCommand(cmd_line) {TOpid=p;}
    virtual ~TimeOutCommand() {}
    void execute() override;
    pid_t getTOpid(){return TOpid;}
    void setTOpid(pid_t p){TOpid=p;}
};


class JobsList{

public:
    class JobEntry {
        // TODO: Add your data members
        string command_;
        const pid_t pid_;
        time_t stoppedTime_;
        // const int id_ = ++JobsList::idCounter_;
        const int id_;  //! ADDED
        const time_t timeStart_ = time(NULL);
        bool isStopped_ = false;

    public:
        string GetCommand() { return command_; }
        const pid_t GetPid() { return pid_; }
        const int GetJobID() { return id_; }
        const time_t GetTime() {
            return difftime(time(NULL), timeStart_);
        }
        const time_t getStartTime(){return timeStart_;}
        bool IsStopped() { return isStopped_; }
        void Stop() {
            stoppedTime_ = GetTime();
            isStopped_ = true;
        }
        void Run() { isStopped_ = false; }

        JobEntry(const string command, const pid_t pid, int id)
            : command_(command), pid_(pid), id_(id) {}
    };
    // TODO: Add your data members
private:
    list<JobEntry> jobsList_;
    JobsList::JobEntry *fgCommand;
    int maxCurrID_ = 0;

public:
    JobsList() { fgCommand = NULL; }
    ~JobsList() {}
    void addJob(const string command, const pid_t pid, bool isStopped = false);
    void printJobsList(bool quit = false);
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry *getJobById(int jobId);
    void removeJobById(int jobId);
    void removeJobByPid(pid_t jobPid);
    JobEntry *getLastJob(int *lastJobId = NULL);
    JobEntry *getLastStoppedJob(int *jobId = NULL);
    void removeJobs();
    int GetJobsListSize() { return jobsList_.size(); }
    void UpdateFgCommand(JobEntry *ent) { fgCommand = ent; }
    // void ResetCounter() { JobsList::idCounter_ = 0; }
    void IncMaxCurrID() { maxCurrID_++; }
    int GetMaxCurrID() { return maxCurrID_; }
    void ResetJobsIDs() { maxCurrID_ = 0; }
    void UpdateMaxCurrID(int i){maxCurrID_ = i;}
    list<JobEntry>* getJobsList(){return &jobsList_;}
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {

    // TODO: Add your data members

public:
    JobsCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~JobsCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {

    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~KillCommand() {}
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {

    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {

    // TODO: Add your data members
public:
    BackgroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~BackgroundCommand() {}
    void execute() override;
};

class CopyCommand : public BuiltInCommand {

    const char *cmd_line_;

public:
    CopyCommand(const char *cmd_line)
        : BuiltInCommand(cmd_line), cmd_line_(cmd_line) {}
    virtual ~CopyCommand() {}
    void execute() override;
};

class chpromptCommand : public BuiltInCommand {
    const char *cmd_line_;

public:
    chpromptCommand(const char *cmd_line)
        : BuiltInCommand(cmd_line), cmd_line_(cmd_line) {}
    virtual ~chpromptCommand() {}
    void execute() override;
};


class SmallShell {
private:
    // TODO: Add your data members

    JobsList jobList_;
    JobsList timedOut;
    pid_t fgPid_ = NO_FG_COMMAND;
    string fgCmd_ = "";
    string prompName = "smash"; //defauld prompt is smash
    SmallShell() {}

public:
    Command *CreateCommand(const char *cmd_line);
    SmallShell(SmallShell const &) = delete;      // disable copy ctor
    void operator=(SmallShell const &) = delete;  // disable = operator
    void FgUpdate(pid_t newPid, string newCmd = "") {
        fgPid_ = newPid;
        fgCmd_ = newCmd;
    }

    pid_t GetFgPid() { return fgPid_; }
    string GetFgCmd() { return fgCmd_; }
    string Getprompt(){return prompName;}
    void Setprompt(string Name){prompName= Name;}
    static SmallShell &getInstance()  // make SmallShell singleton
    {
        static SmallShell instance;  // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell() {}
    void executeCommand(const char *cmd_line);
    JobsList* getJobList(){return &jobList_;}
    JobsList* getTimedOut(){return &timedOut;}

    // TODO: add extra methods as needed
    //
};

int _parseCommandLine(const char *cmd_line, char **args);


//SmallShell &smash = SmallShell::getInstance();

#endif  // SMASH_COMMAND_H_
