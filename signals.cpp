#include "signals.h"
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include "Commands.h"
using namespace std;

SmallShell &smash = SmallShell::getInstance();
JobsList *jobList = smash.getJobList();

void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z" << endl;
    pid_t pid = smash.GetFgPid();
    string cmd = smash.GetFgCmd();
    if (pid == NO_FG_COMMAND) {
        return;
    }
    if (kill(pid, SIGSTOP) < 0) {
        perror("smash error: kill failed");
        return;
    }
    jobList->addJob(cmd, pid, true);
    jobList->UpdateFgCommand(NULL);
    cout << "smash: process " << pid << " was stopped" << endl;
    smash.FgUpdate(NO_FG_COMMAND);
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    pid_t pid = smash.GetFgPid();
    if (pid == NO_FG_COMMAND) {
        return;
    }
    if (kill(pid, SIGINT) < 0) {
        perror("smash error: kill failed");
        return;
    }
    cout << "smash: process " << pid << " was killed" << endl;
    smash.FgUpdate(NO_FG_COMMAND);
}
