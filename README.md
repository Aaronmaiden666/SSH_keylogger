# SSH_keylogger for GNU/Linux (WIP)

This is project for implementing featurable keylogger for Linux-systems. I will be glad to ideas, suggestions, criticism, best practices, and help in the development of architecture, the choice of libraries and new features.
I would like to combine in one project all features of keylogging:
 - logging keyboard usage (pressing buttons on the real keyboard, connected to your PC)
 - logging remote SSH-connections to your host: both incoming and outgoing
 -- this will be implemented by stracing (ptracing in the future) SSH processes on the host machine (it needs administrative privilege)
 -- hope I’ll come up with logging output of pty's while using SSH for getting full-duplex user interaction
 - I want to implement the way to send log information to administrator of local network or to owner of the PC by email, by some specific protocol over tcp or something else. I am still not sure. Perhaps it will have a way to indicate remote connection to PC by SSH. We will see...

I use C++14/17 for coding but I am still studying. I do it in out of work time and the speed is not such rapid as I dream. You are welcome for contributing together.

### How to compile and run

```bash
cd SSH_kelloger/
mkdir build && cd build
cmake .. && cmake --build .
sudo ./keylogger
```

### All progress in dev-branch now. Keylogger SSH with features, described futher, is ready.

#### Requirements:
 - CMake
 - GCC
 - Boost (regex, filesystem)
 - Administrative privilege (Ha-Ha)


### Already implemented features:
 - Automatically scan (every 3 seconds) all SSH connections (incoming and outgoing) and log them into ```/tmp/.keylogger/**.log```.
 - Detect new connections (incoming or outgoing) by ssh and start logging them.
 - Detect closed connections and stop logging them.
 - Pressing ```"ENTER"``` and ```"BACKSPACE"``` is logging as physical pressing. It means, wrong command entered by user and backspaced then will be not logged.
 In log file we will see only last version of command. Backspaced letters will be deleted and will not appear in log files.

**NOTE:** This code uses strace for logging, that's why it is not enough hide. You can see ```strace``` command in ```ps```
