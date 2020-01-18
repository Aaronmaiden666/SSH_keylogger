# SSH_keylogger (WIP)

This is project for implementing featurable keylogger for Linux-systems. I will be glad to ideas, suggestions, criticism, best practices, and help in the development of architecture, the choice of libraries and new features.
I would like to combine in one project all features of keylogging:
 - logging keyboard usage (pressing buttons on the real keyboard, connected to your PC)
 - logging remote SSH-connections to your host: both incoming and outgoing
 -- this will be implemented by stracing (ptracing in the future) SSH processes on the host machine (it needs administrative privilege)
 -- hope I’ll come up with logging output of pty's while using SSH for getting full-duplex user interaction
 - I want to implement the way to send log information to administrator of local network or to owner of the PC by email, by some specific protocol over tcp or something else. I am still not sure. Perhaps it will have a way to indicate remote connection to PC by SSH. We will see...

I use C++14/17 for coding but I am still studying. I do it in out of work time and the speed is not such rapid as I dream. 
#### All progress in dev-branch now. I am working with strace now.

You are welcome for contributing together.
