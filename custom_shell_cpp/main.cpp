#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
using namespace std;

string trim(const string &str) {
    int start = str.find_first_not_of(' ');
    int end = str.find_last_not_of(' ');
    return (start == string::npos || end == string::npos) ? "" : str.substr(start, end - start + 1);
}

void executeCommand(const vector<string> &command, int input_fd = -1, int output_fd = -1) {
    vector<char*> argv;
    for (const auto &arg : command) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid == 0) {
        if (input_fd != -1) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != -1) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        execvp(argv[0], argv.data());
        cerr << "failed to execute " << argv[0] << endl;
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        cerr << "failed to fork" << endl;
    }
}

int main() {
    while (true) {

        char buffer[PATH_MAX];
        string path =  (getcwd(buffer, sizeof(buffer)) ? string(buffer) : string(""));
        string new_path = "~";
        if (path.find("/home/vigneshvembar") == 0) new_path += path.substr(19);

        cout << "" << new_path << " 🐱< ";
        string input;
        getline(cin, input);

        vector<string> commands;
        string temp_command;
        for (const char &c : input) {
            if (c == '|') {
                temp_command = trim(temp_command);
                commands.push_back(temp_command);
                temp_command = "";
            } 
            else {
                temp_command += c;
            }
        }
        temp_command = trim(temp_command);
        if (!temp_command.empty()) {
            commands.push_back(temp_command);
        }

        int input_fd = -1;
        int output_fd[2];

        for (size_t i = 0; i < commands.size(); ++i) {
            vector<string> parts;
            string temp_part;
            for (char &c : commands[i]) {
                if (c == ' ') {
                    if (!temp_part.empty()) {
                        parts.push_back(temp_part);
                        temp_part = "";
                    }
                } 
                else {
                    temp_part += c;
                }
            }
            if (!temp_part.empty()) {
                parts.push_back(temp_part);
            }

            if (parts.empty()) continue;

            string command = parts[0];
            vector<string> args;
            if (parts.size() > 1) {
                args = vector<string>(parts.begin() + 1, parts.end());
            }

            if (command == "meow") command = "ls";

            if (command == "cd") {
                if (args.size() == 0) {
                    cout << "cd: missing argument\n";
                } 
                else if (args.size() > 1) {
                    cout << "cd: too many arguments\n";
                } 
                else {
                    string path = args[0];
                    if (path == "~") {
                        chdir(getenv("HOME"));
                    } 
                    else {
                        if (chdir(path.c_str()) != 0) {
                            perror("cd failed");
                        }
                    }
                }
            } 
            else if (command == "exit") exit(0); 
            else {
                if (i < commands.size() - 1) {
                    pipe(output_fd);
                } 
                else {
                    output_fd[1] = -1;
                }
                vector<string> full_command = {command};
                full_command.insert(full_command.end(), args.begin(), args.end());
                executeCommand(full_command, input_fd, output_fd[1]);
                if (input_fd != -1) {
                    close(input_fd);
                }
                if (output_fd[1] != -1) {
                    close(output_fd[1]);
                }
                input_fd = output_fd[0];
            }
        }

        if (input_fd != -1) {
            close(input_fd);
        }
    }
    return 0;
}
