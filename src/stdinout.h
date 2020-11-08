#ifndef STDINOUT_H
#define STDINOUT_H
#include "blackjack.h"

class StdInOut : public Player {
  public:  
    StdInOut();
    ~StdInOut() { };
    
    int play(void) override;
    void info(Info = Info::None, int = 0) override;
    
  private:
    std::string input_buffer;

    std::string black   = "\x1B[0m";
    std::string red     = "\x1B[31m";
    std::string green   = "\x1B[32m";
    std::string yellow  = "\x1B[33m";
    std::string blue    = "\x1B[34m";
    std::string magenta = "\x1B[35m";
    std::string cyan    = "\x1B[36m";
    std::string white   = "\x1B[37m";
    std::string reset   = "\033[0m";
    
      
};
#endif
