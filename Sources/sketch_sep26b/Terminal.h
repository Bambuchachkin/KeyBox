#ifndef TERMINAL_H
#define TERMINAL_H

#include <map>
#include <string>

class Terminal {
  private:
    // std::map<std::string, std::string> HELP;
  public:
    Terminal();
    ~Terminal();

    void process_command(std::string command);
    std::string read_command();
};

#endif