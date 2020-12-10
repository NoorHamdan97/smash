#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"




int NextAlarm(JobsList* comm){
    int alarm = 0;
    bool first = true;
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

void alrmHandler(int signum){
    SmallShell &smash = SmallShell::getInstance();
    char *argv[MAX_COMMAND_LENGTH];
    cout << "smash: got an alarm" << endl;
    JobsList* TO= smash.getTimedOut();
    list<JobsList::JobEntry> entries= *(TO->getJobsList());
    for(auto& entry : entries){
        string cmdLine = entry.GetCommand();
        int n= cmdLine.length();
        char* cmd_Line=(char*)malloc(n+1);
        strcpy(cmd_Line, cmdLine.c_str());
        _parseCommandLine(cmd_Line, argv);
        int seconds= atoi(argv[1]);
        time_t passed_time = entry.GetTime();
        pid_t pid = entry.GetPid();
        if(passed_time >= seconds){
            TO->removeJobByPid(pid);
            cout << "smash: " +cmdLine+" timed out!" << endl;
            if (getpid()!= pid) {
              //external command
              if (kill(pid, SIGKILL) < 0) {
                perror("smash error: kill failed");
              }
            }
        }
    }
    if(TO->GetJobsListSize()>0){
      int sec = NextAlarm(TO);
      alarm(sec);
    }
}


// void prepare(){
//     struct sigaction sa;
//     sa.sa_handler = alrmHandler;
//     sigemptyset(&sa.sa_mask);
//     sa.sa_flags = SA_RESTART;
//     if (sigaction(SIGINT, &sa, NULL) == -1) // ??
//     {
//       perror("smash error: failed to set sig_alrm handler"); //! ADDED
//     }
// }

int main(int argc, char *argv[])
{
  if (signal(SIGTSTP, ctrlZHandler) == SIG_ERR)
  {
    perror("smash error: failed to set ctrl-Z handler");
  }
  if (signal(SIGINT, ctrlCHandler) == SIG_ERR)
  {
    perror("smash error: failed to set ctrl-C handler");
  }


  struct sigaction sa;
  sa.sa_handler = alrmHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGALRM, &sa, NULL) == -1)
  {
    perror("smash error: failed to set sig_alrm handler"); //! ADDED
  }
  SmallShell &smash = SmallShell::getInstance();
  while (true)
  {
    string Prompt = smash.Getprompt();
    std::cout << (Prompt + "> ");
    std::string cmd_line;
    std::getline(std::cin, cmd_line);
    smash.executeCommand(cmd_line.c_str());
  }
  return 0;
}

