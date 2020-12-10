#include "Commands.h"
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <dirent.h>


using namespace std;

//* Added
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <list>

#define MAX_COMMANDS (50)       //* Length of command line
#define BASH_EXEC "/bin/bash"   //* Name of process
#define BUFFER_SIZE 1024


#if 0
#define FUNC_ENTRY() cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

pid_t smashPid = getpid();

int _parseExternalCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    stringstream check1(cmd_line);
    string intermediate;
    int i = 2;
    while (getline(check1, intermediate)) {
        args[i] = (char *)malloc(intermediate.length() + 1);
        memset(args[i], 0, intermediate.length() + 1);
        strcpy(args[i], intermediate.c_str());
        args[++i] = NULL;
    }
    string intermediate2(BASH_EXEC);
    args[0] = (char *)malloc(intermediate2.length() + 1);
    memset(args[0], 0, intermediate2.length() + 1);
    strcpy(args[0], intermediate2.c_str());

    string intermediate3("-c");
    args[1] = (char *)malloc(intermediate3.length() + 1);
    memset(args[1], 0, intermediate3.length() + 1);
    strcpy(args[1], intermediate3.c_str());

    return i;

    FUNC_EXIT()
}


int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    stringstream check1(cmd_line);
    string intermediate;
    int i = 0;
    while (getline(check1, intermediate, ' ')) {
        if (!intermediate.empty()) {
            args[i] = (char *)malloc(intermediate.length() + 1);
            memset(args[i], 0, intermediate.length() + 1);
            strcpy(args[i], intermediate.c_str());
            args[++i] = NULL;
        }
    }
    return i;

    FUNC_EXIT()
}


bool _isBackgroundComamnd(const char *cmd_line) {
    const string whitespace = " \t\n";
    const string str(cmd_line);
    return str[str.find_last_not_of(whitespace)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
    const string whitespace = " \t\n";
    const string str(cmd_line);
    // find last character other than spaces
    size_t idx = str.find_last_not_of(whitespace);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing
    // spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(whitespace, idx - 1) + 1] = 0;
}


//find the next alarm
int getAlarm(JobsList* comm){
    int alarm=0;
    bool first=true;
    list<JobsList::JobEntry> *alarms = comm->getJobsList();
    for (list<JobsList::JobEntry>::iterator it = (*alarms).begin(); it != (*alarms).end(); ++it) {
         string cmd_line = it->GetCommand();
         const char * c = cmd_line.c_str();
         char *argv[MAX_COMMAND_LENGTH];
         _parseCommandLine(c, argv);
         int seconds= atoi(argv[1]);
        if(first){
            first = false;
            alarm = seconds;
        }
        else{
            if(alarm > seconds){
               alarm = seconds;
            }
        }
    }
    return alarm;
}

// Convert the command timeout line to an aligned command line
char* getTOcommand (const char* cmd_line_){
    char *argv[MAX_COMMAND_LENGTH];
    int const argc = _parseCommandLine(cmd_line_, argv);
    string comm = string(argv[2]);
    for(int k=3;k< argc;k++){
        comm = comm + " "+ string(argv[k]);
    }
    int n= comm.length();
    //Do free after
    char* cmd_Line=(char*)malloc(n+1);
    strcpy(cmd_Line, comm.c_str());
    return cmd_Line;
}

int GetAlarm(JobsList* comm){
    int alarm=0;
    bool first=true;
    list<JobsList::JobEntry> *alarms = comm->getJobsList();
    for (list<JobsList::JobEntry>::iterator it = (*alarms).begin(); it != (*alarms).end(); ++it) {
         string cmd_line = it->GetCommand();
         const char * c = cmd_line.c_str();
         char *argv[MAX_COMMAND_LENGTH];
         _parseCommandLine(c, argv);
         int sec1= atoi(argv[1]);
          time_t sec2 = it->GetTime();
          int seconds = sec1 - sec2;
        if(first){
            first = false;
            alarm = seconds;
        }
        else{
            if(alarm > seconds){
               alarm = seconds;
            }
        }
    }
    return alarm;
}

//////////////////////////////////////////////////////* #2 TimeOutCommand
void TimeOutCommand::execute(){
    pid_t pid = this->getTOpid();
    SmallShell &smash = SmallShell::getInstance();
    JobsList* TO = smash.getTimedOut();
    TO->addJob(cmd_line_,pid);
    char *argv[MAX_COMMAND_LENGTH];
    _parseCommandLine(cmd_line_, argv);
   // int seconds= atoi(argv[1]);
    int seconds = getAlarm(TO);
    alarm(seconds);
}

//////////////////////////////////////////////////////* #2 chpromptCommand
void chpromptCommand::execute(){
    string prmpt = "smash";
    char *argv[MAX_COMMAND_LENGTH];
    int const argc = _parseCommandLine(cmd_line_, argv);
    SmallShell &smash = SmallShell::getInstance();
    if (argc == 1){
        smash.Setprompt(prmpt);
        return;
    }
    else{
        smash.Setprompt(argv[1]);
    }
}

//////////////////////////////////////////////////////* #3 lsCommand
void lsCommand::execute(){
    vector<string> files;
    DIR *dr;
    int i=0;
    struct dirent *en;
    dr = opendir("."); //open all directory
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            i++;
           //cout<<" \n"<<en->d_name; //print all directory name
            files.push_back(en->d_name);
        }
    closedir(dr); //close all directory
    }
    sort(files.begin(),files.end());
    int it=0;
    while(i>0){
        i--;
        cout<<files.at(it) +"\n";
        it++;
    }
}

////////////////////////////////////////////////////// *#4 GetCurrDir Command
void GetCurrDirCommand::execute() {

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd))) {
        printf("%s\n", cwd);
        return;
    }
    perror("smash error: getcwd failed");
}
////////////////////////////////////////////////////////* #5 ChangeDir Command
string ChangeDirCommand::history_ = "";
void ChangeDirCommand::execute() {

    char *argv[MAX_COMMAND_LENGTH];
    int const argc = _parseCommandLine(cmd_line_, argv);
    //* Error handling
    if (argc == 1)  //? Not sure if we should check this?
    {
        return;
    }
    if (argc > 2) {
        cout << "smash error: cd: too many arguments" << endl;
        return;
    }
    if (*argv[1] == '-' && ChangeDirCommand::history_.length() == 0) {
        cout << "smash error: cd: OLDPWD not set" << endl;
        return;
    }
    //* No errors
    //* Get curr dir
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    string currentDir(cwd);

    if (*argv[1] == '-') {
        if (chdir(ChangeDirCommand::history_.c_str()) != 0) {
            perror("smash error: chdir failed");
            return;
        }
    } else if (chdir(argv[1]) != 0) {
        perror("smash error: chdir failed");
        return;
    }
    ChangeDirCommand::history_ = currentDir;
}


void JobsCommand::execute() { jobsList_->printJobsList(); }
////////////////////////////////////////////////////////////* #6 Kill command
void KillCommand::execute() {
    jobsList_->removeFinishedJobs();
    if (!jobsList_->GetJobsListSize()) {
        jobsList_->ResetJobsIDs();
    }
    else{
        jobsList_->UpdateMaxCurrID(jobsList_->getLastJob()->GetJobID());
    }
    char *argv[MAX_COMMAND_LENGTH];
    int const argc = _parseCommandLine(cmd_line_, argv);
    //* Error handling
    if (argc != 3) {
        cout << "smash error: kill: invalid arguments" << endl;
        return;
    }
    // check if digit
    string sigNumDash(argv[1]);
    string jobId(argv[2]);
    if (sigNumDash[0] != '-') {
        cout << "smash error: kill: invalid arguments" << endl;
        return;
    }
    string sigNum;
    for (unsigned int i = 1; i < sigNumDash.length(); i++) {
        if (!isdigit(sigNumDash[i])) {
            cout << "smash error: kill: invalid arguments" << endl;
            return;
        }
        sigNum.push_back(sigNumDash[i]);
    }
    int sig = stoi(sigNum);
    for (unsigned int i = 0; i < jobId.length(); i++) {
        if (!isdigit(jobId[i])) {
            cout << "smash error: kill: invalid arguments" << endl;
            return;
        }
    }
    int id = stoi(jobId);
    auto *job = jobsList_->getJobById(id);
    if (job == NULL) {
        cout << "smash error: kill: job-id " << id << " does not exist" << endl;
        return;
    }
    pid_t pid = job->GetPid();
    //* No errors
    if (kill(pid, sig) < 0) {
        perror("smash error: kill failed");
    } else {
        cout << "signal number " << sig << " was sent to pid " << pid << endl;
    }
}
//////////////////////////////////////////////////* #7 ShowPid  Command
void ShowPidCommand::execute() {
       cout << "smash pid is " << smashPid << endl;

}
//////////////////////////////////////////////////* #8 Fg Command
void ForegroundCommand::execute() {
    jobsList_->removeFinishedJobs();
    if (!jobsList_->GetJobsListSize()) {
        jobsList_->ResetJobsIDs();
    }else{
        jobsList_->UpdateMaxCurrID(jobsList_->getLastJob()->GetJobID());
    }

    char *argv[MAX_COMMAND_LENGTH];
    int const argc = _parseCommandLine(cmd_line_, argv);
    //* Error handling
    if (argc == 1 && jobsList_->getLastJob() == NULL) {
        cout << "smash error: fg: jobs list is empty" << endl;
        return;
    }
    if (argc > 2) {
        cout << "smash error: fg: invalid arguments" << endl;
        return;
    }
    pid_t pid = 0;
    int id = 0;
    JobsList::JobEntry *job = NULL;
    if (argc == 2) {
        string jobId(argv[1]);
        for (unsigned int i = 0; i < jobId.length(); i++) {
            if (!isdigit(jobId[i])) {
                cout << "smash error: fg: invalid arguments" << endl;
                return;
            }
        }
        id = stoi(jobId);
        if ((job = jobsList_->getJobById(id)) == NULL) {
            cout << "smash error: fg: job-id " << id << " does not exist"
                << endl;
            return;
        }
    } else if (argc == 1) {
        job = jobsList_->getLastJob();
        id = job->GetJobID();
    }
    pid = job->GetPid();
    if (job->IsStopped()) {
        if (kill(pid, SIGCONT) < 0) {
            perror("smash error: kill failed");
        }
    }

    cout << job->GetCommand() << " : " << pid << endl;
    job->Run();
    SmallShell &smash = SmallShell::getInstance();
    smash.FgUpdate(pid, job->GetCommand());
    jobsList_->UpdateFgCommand(job);
    jobsList_->removeJobById(id);
    if (waitpid(pid, NULL, WUNTRACED) < 0) {
        perror("smash error: waitpid failed");
    }
}

//////////////////////////////////////////////////* #9 Bg Command
void BackgroundCommand::execute() {
    jobsList_->removeFinishedJobs();
    if (!jobsList_->GetJobsListSize()) {
        jobsList_->ResetJobsIDs();
    }
    else{
        jobsList_->UpdateMaxCurrID(jobsList_->getLastJob()->GetJobID());
    }
    char *argv[MAX_COMMAND_LENGTH];
    int const argc = _parseCommandLine(cmd_line_, argv);
    //* Error handling
    if (argc == 1 && jobsList_->getLastStoppedJob() == NULL) {
        cout << "smash error: bg: there is no stopped jobs to resume" << endl;
        return;
    }
    if (argc > 2) {
        cout << "smash error: bg: invalid arguments" << endl;
        return;
    }
    pid_t pid = 0;
    int id = 0;
    JobsList::JobEntry *job = NULL;
    if (argc == 2) {
        string jobId(argv[1]);
        for (unsigned int i = 0; i < jobId.length(); i++) {
            if (!isdigit(jobId[i])) {
                cout << "smash error: bg: invalid arguments" << endl;
                return;
            }
        }
        id = stoi(jobId);
        job = jobsList_->getJobById(id);
        if (job == NULL) {
            cout << "smash error: bg: job-id " << id << " does not exist"
                << endl;
            return;
        } else if (!job->IsStopped()) {
            cout << "smash error: bg: job-id " << id
                << " is already running in "
                    "the background"
                << endl;
            return;
        }

    } else if (argc == 1) {
        job = jobsList_->getLastStoppedJob();
        id = job->GetJobID();
    }
    pid = job->GetPid();
    if (kill(pid, SIGCONT) < 0) {
        perror("smash error: kill failed");
    }
    job->Run();
    cout << job->GetCommand() << " : " << pid << endl;
}
//////////////////////////////////////////////////* #10 Quit Command

void QuitCommand::execute() {
    jobsList_->removeFinishedJobs();
    if (!jobsList_->GetJobsListSize()) {
        jobsList_->ResetJobsIDs();
    }
    else{
        jobsList_->UpdateMaxCurrID(jobsList_->getLastJob()->GetJobID());
    }
    int numOfJobs = 0;
    char *argv[MAX_COMMAND_LENGTH];
    int const argc = _parseCommandLine(cmd_line_, argv);
    //* Error handling
    if (argc == 2) {
        string Arg1(argv[1]);
        string KillArg("kill");
        if (Arg1.compare(KillArg) == 0) {
            numOfJobs = jobsList_->GetJobsListSize();
            cout << "smash: sending SIGKILL signal to " << numOfJobs
                << " jobs:" << endl;
            jobsList_->printJobsList(true);
            jobsList_->killAllJobs();
            jobsList_->removeJobs();
        }
    }
    exit(0);
}
//////////////////////////////////////////////////* #11 Cp Command

void CopyCommand::execute() {
    char *argv[MAX_COMMAND_LENGTH];
    _parseCommandLine(cmd_line_, argv);
    char buffer[BUFFER_SIZE];
    int files[2];
    ssize_t count;
    files[0] = open(argv[1], O_RDONLY);
    if (files[0] < 0) { // Check if file opened
        perror("smash error: open failed");
        return;
    }
    files[1] =
        open(argv[2], O_WRONLY | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, 0644);
    if (files[1] == -1) // Check if file opened (permissions problems ...)
    {
        close(files[0]);
        perror("smash error: open failed");
        return;
    }
    while ((count = read(files[0], buffer, sizeof(buffer))) != 0) {
        if (count == -1) {
            close(files[0]);
            perror("smash error: read failed");
        } else if (write(files[1], buffer, count) == -1) {
            close(files[0]);
            perror("smash error: write failed");
        }
    }
    cout << "smash: " << string(argv[1]) << " was copied to " << string(argv[2])
        << endl;
}

//////////////////////////////////////////////////* Command
Command::Command(const char *cmd_line) : cmd_line_(cmd_line) {
    SmallShell &small = SmallShell::getInstance();
    jobsList_ = small.getJobList();
    timedoutJobs =small.getTimedOut();
}
//////////////////////////////////////////////////* External Command

void ExternalCommand::execute() {
    char *argv[MAX_COMMAND_LENGTH + 2];
    bool isBgCommand = _isBackgroundComamnd(cmd_line_);
    int argc;


    if (isBgCommand) {
        string intermediate(cmd_line_);
        char *s;
        s = (char *)malloc(intermediate.length() + 1);
        memset(s, 0, intermediate.length() + 1);
        strcpy(s, intermediate.c_str());
        _removeBackgroundSign(s);
        argc = _parseExternalCommandLine(s, argv);
        free(s);
    } else {
        argc = _parseExternalCommandLine(cmd_line_, argv);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("smash error: fork failed");
        return;
    }
    if (!pid) {
        setpgrp();
        if (execv(BASH_EXEC, argv) < 0) {
            perror("smash error: execv failed");
            return;
        }
    } else {
        if (!isBgCommand) {
            SmallShell::getInstance().FgUpdate(pid, cmd_line_);
            if (waitpid(pid, NULL, WUNTRACED) < 0) {
                perror("smash error: waitpid failed");
            } else {
                SmallShell::getInstance().FgUpdate(NO_FG_COMMAND);
            }
        } else {
            jobsList_->addJob(cmd_line_, pid, false);
            jobsList_->UpdateMaxCurrID((jobsList_->getLastJob()->GetJobID()));
        }
    }
    // Free memory
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
}

//////////////////////////////////////////////////* JobList

void JobsList::addJob(std::string command, const pid_t pid, bool isStopped) {
    removeFinishedJobs();
    if (!GetJobsListSize()) {
        ResetJobsIDs();
    }else{
        UpdateMaxCurrID((getLastJob()->GetJobID()));
    }

    bool Added = false;
    if (fgCommand != NULL) {
        if (fgCommand->GetPid() == pid) {
            if (fgCommand->GetJobID() >= (GetMaxCurrID())) {
                if (isStopped) {
                    fgCommand->Stop();
                }
                jobsList_.push_back(*fgCommand);
                Added = true;
            } else {
                for (list<JobEntry>::iterator it = jobsList_.begin();
                    it != jobsList_.end(); ++it) {
                    if (it->GetJobID() > fgCommand->GetJobID()) {
                        if (isStopped) {
                            fgCommand->Stop();
                        }
                        jobsList_.insert(it, *fgCommand);
                        Added = true;
                        break;
                    }
                }
            }
        }
    }
    if (!Added) {
        JobEntry j(command, pid, GetMaxCurrID()+1);
        if (isStopped) {
            j.Stop();
        }
        jobsList_.push_back(j);
        IncMaxCurrID();
    }
}

void JobsList::killAllJobs() {
    removeFinishedJobs();
    if (!GetJobsListSize()) {
        ResetJobsIDs();
    }else{
        UpdateMaxCurrID((getLastJob()->GetJobID()));
    }
    for (auto &i : jobsList_) {
        pid_t pid = i.GetPid();
        if (kill(pid, SIGKILL) < 0) {
            perror("smash error: kill failed");
        }
    }
    return;
}

void JobsList::printJobsList(bool quit) {
    removeFinishedJobs();
    if (!GetJobsListSize()) {
        ResetJobsIDs();
    }else{
        UpdateMaxCurrID((getLastJob()->GetJobID()));
    }
    if (!quit) {
        for (auto &i : jobsList_) {
            cout << "[" << i.GetJobID() << "] " << i.GetCommand() << " : "
                << i.GetPid() << " " << i.GetTime() << " secs"
                << (i.IsStopped() ? " (stopped)" : "") << endl;
        }
    } else {
        for (auto &i : jobsList_) {
            cout << i.GetPid() << ": " << i.GetCommand() << endl;
        }
    }
}

void JobsList::removeFinishedJobs() {
    int status;
    for (list<JobEntry>::iterator it = jobsList_.begin(); it != jobsList_.end();
        ++it) {
        int res = waitpid(it->GetPid(), &status, WNOHANG);
        if (res > 0 && (WIFEXITED(status) || WIFSIGNALED(status))) {
            jobsList_.erase(it);
            this->removeFinishedJobs();
            return;
        }
    }
}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    for (auto &i : jobsList_) {
        if (i.GetJobID() == jobId) {
            return &i;
        }
    }
    return NULL;
}

void JobsList::removeJobById(int jobId) {
    for (list<JobEntry>::iterator it = jobsList_.begin(); it != jobsList_.end();
        ++it) {
        if (it->GetJobID() == jobId) {
            jobsList_.erase(it);
            if (!GetJobsListSize()) {
                ResetJobsIDs();
            }
            return;
        }
    }
    return;
}

void JobsList::removeJobByPid(pid_t jobPid) {
    for (list<JobEntry>::iterator it = jobsList_.begin(); it != jobsList_.end();
        ++it) {
        if (it->GetPid() == jobPid) {
            jobsList_.erase(it);
            return;
        }
    }
    return;
}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {
    if (jobsList_.size()) {
        if (lastJobId) {
            *lastJobId = jobsList_.back().GetJobID();
        }
        return &jobsList_.back();
    }
    return NULL;
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    for (list<JobEntry>::iterator it = jobsList_.begin(); it != jobsList_.end();
        ++it) {
        if (it->IsStopped()) {
            if (jobId) {
                *jobId = it->GetJobID();
            }
            return &*it;
        }
    }
    return NULL;
}

void JobsList::removeJobs() {
    for (list<JobEntry>::iterator it = jobsList_.begin(); it != jobsList_.end();
         ++it) {
        jobsList_.erase(it);
        removeJobs();
        return;
    }
    return;
}
//////////////////////////////////////////////////* SmallShell

void SmallShell::executeCommand(const char *cmd_line) {
    Command *cmd = CreateCommand(cmd_line);
    if (cmd != nullptr) {
        cmd->execute();
    }
    // Please note that you must fork smash process for some commands (e.g.,
    // external commands....)
}

void ExternalCommand::execute2(const char *cmd_line,char* comm){
    char *argv[MAX_COMMAND_LENGTH + 2];
    bool isBgCommand = _isBackgroundComamnd(comm);
    int argc;
    if (isBgCommand) {
        string intermediate(comm);
        char *s;
        s = (char *)malloc(intermediate.length() + 1);
        memset(s, 0, intermediate.length() + 1);
        strcpy(s, intermediate.c_str());
        _removeBackgroundSign(s);
        argc = _parseExternalCommandLine(s, argv);
        free(s);
    } else {
        argc = _parseExternalCommandLine(comm, argv);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("smash error: fork failed");
        return;
    }

    if (!pid) { // son
        setpgrp();
        if (execv(BASH_EXEC, argv) < 0) {
            perror("smash error: execv failed");
            return;
        }
    }

    else {
        TimeOutCommand* c = new TimeOutCommand(cmd_line,pid);
        c->execute();
        if (!isBgCommand) {
            SmallShell::getInstance().FgUpdate(pid, comm);
            if (waitpid(pid, NULL, WUNTRACED) < 0) {
                perror("smash error: waitpid failed");
            } else {
                SmallShell::getInstance().FgUpdate(NO_FG_COMMAND);
            }
        } else {
            jobsList_->addJob(comm, pid, false);
            jobsList_->UpdateMaxCurrID((jobsList_->getLastJob()->GetJobID()));
        }
    }
    // Free memory
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
}


Command* SmallShell::CreateCommand(const char *cmd_line) {
    char *argv[MAX_COMMAND_LENGTH];
    _parseCommandLine(cmd_line, argv);
    string cmd_(argv[0]);
    string fullCmd(cmd_line);

    if (fullCmd.find(string(">")) != string::npos){
        return new RedirectionCommand(cmd_line);
    }
    else if (fullCmd.find(string("|")) != string::npos ){
    return new PipeCommand(cmd_line);
    }
    else if (!cmd_.compare(string("timeout"))){
        char* comm= getTOcommand(cmd_line);
        ExternalCommand* exc = new ExternalCommand(comm);
        exc->execute2(cmd_line,comm);
    }
    else if (!cmd_.compare(string("chprompt"))){
        return new chpromptCommand(cmd_line);
    }
    else if (!cmd_.compare(string("ls"))){
        return new lsCommand(cmd_line);
    }
    else if (!cmd_.compare(string("cp"))){
        return new CopyCommand(cmd_line);
    }
    else if (!cmd_.compare(string("pwd")) || !cmd_.compare(string("pwd&"))) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (!cmd_.compare(string("cd"))) {
        return new ChangeDirCommand(cmd_line);
    }
    else if (!cmd_.compare(string("showpid")) || !cmd_.compare(string("showpid&"))) {
        return new ShowPidCommand(cmd_line);
    }
    else if (!cmd_.compare(string("kill"))) {
        return new KillCommand(cmd_line);
    }
    else if (!cmd_.compare(string("jobs")) || !cmd_.compare(string("jobs&"))) {
        return new JobsCommand(cmd_line);
    }
    else if (!cmd_.compare(string("fg")) || !cmd_.compare(string("fg&"))) {
        return new ForegroundCommand(cmd_line);
    }
    else if (!cmd_.compare(string("bg")) || !cmd_.compare(string("bg&"))) {
        return new BackgroundCommand(cmd_line);
    }
    else if (!cmd_.compare(string("quit")) || !cmd_.compare(string("quit&"))) {
        return new QuitCommand(cmd_line);
    }
    else {
        return new ExternalCommand(cmd_line);
    }
    return nullptr;
}

//////////////////////////////////////////////////*  RedirectionType
string getRedirectionType(char **argv, int *index) {
    int i = 0;
    while (argv[i]) {
        string s(argv[i]);
        if (!s.compare(">")) {
            *index = i;
            return string(">");
        }
        if (!s.compare(">>")) {
            *index = i;
            return string(">>");
        }
        i++;
    }
    return string("err");
}

////////////////////////////////////////////////////* #12 RedirectionCommand
void RedirectionCommand::execute() {
    char *argv[MAX_COMMAND_LENGTH];
    int const argc = _parseCommandLine(cmd_line_, argv);
    int index;
    string type = getRedirectionType(argv, &index);

    // error check
    if (!type.compare("err")) {
        return;
    }

    if (index == 0 || (index + 1 == argc)) {
        return;
    }
    // check that only one arg is after > or >>
    if (index + 2 != argc) {
        return;
    }

    string cmd;
    for (int i = 0; i < index; i++) {
        cmd += argv[i];
        cmd += " ";
    }
    string fileName = argv[index + 1];

    //* Idea :
    //* First fork and the son will close the stdout and put in it the
    //* fptr of the opened file.
    //* then execute the command

    pid_t pid = fork();
    if (pid < 0) {
        perror("smash error: fork failed");
        return;
    }
    if (!pid) {
        if (close(STDOUT_FILENO) < 0) {
            perror("smash error: close failed");
            return;
        }
        FILE *fptr;
        if (!type.compare(">>")) {
            fptr = fopen(fileName.c_str(), "a");
        }
        if (!type.compare(">")) {
            fptr = fopen(fileName.c_str(), "w");
        }
        if (fptr == NULL) {
            perror("smash error: fopen failed");
            return;
        }
        SmallShell &smash = SmallShell::getInstance();
        smash.executeCommand(cmd.c_str());
        fclose(fptr);
        exit(0);
    }
}
//////////////////////////////////////////////////////////* PipeType
string getPipeType(char **argv, int *index) {
    int i = 0;
    while (argv[i]) {
        string s(argv[i]);
        if (!s.compare("|")) {
            *index = i;
            return string("|");
        }
        if (!s.compare("|&")) {
            *index = i;
            return string("|&");
        }
        i++;
    }
    return string("err");
}

////////////////////////////////////////////////////* #13 PipeCommand
void PipeCommand::execute(){
    char *argv[MAX_COMMAND_LENGTH];
    int const argc = _parseCommandLine(cmd_line_, argv);
    int index;
    string type = getPipeType(argv, &index);

    // error check
    if (!type.compare("err")) {
        return;
    }
    //check if the pipe character either at the beginnig or the end
    if (index == 0 || (index + 1 == argc)) {
        return;
    }

    // first command
    string cmd1;
    for (int i = 0; i < index; i++) {
        cmd1 += argv[i];
        cmd1 += " ";
    }
    // second command
    string cmd2;
    for (int i = index+1; i < argc; i++){
        cmd2 += argv[i];
        cmd2 += " ";
    }
    // create a pipe
    int fd[2];
    pipe(fd);
    if (pipe(fd) < 0){
        perror("smash error: pipe failed");
        return;
    }
    // checks type of the pipe
    bool OP1 = false, OP2 = false;
    if (!type.compare("|")){
        OP1 =true;
    }
    if (!type.compare("|&")){
        OP2 = true;
    }
    //first fork to do command1
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("smash error: fork failed");
        return;
    }
    if(!pid1){ // first son
        if (OP1){ // redirect the stdout of command1
            dup2(fd[1],1);
            close(fd[0]);
            close(fd[1]);
        }else if (OP2){ // redirect the stderr of command1
            dup2(fd[1],2);
            close(fd[0]);
            close(fd[1]);
        }
    SmallShell::getInstance().executeCommand(cmd1.c_str());
    exit(0);
    }

    //second fork to do command2
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("smash error: fork failed");
        return;
    }
    if(!pid2){ // second son
        dup2(fd[0],0);
        close(fd[0]);
        close(fd[1]);
    SmallShell::getInstance().executeCommand(cmd2.c_str());
    exit(0);
    }
    close(fd[0]);
    close(fd[1]);
}
