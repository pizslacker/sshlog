sshlog
======

a command-line log-assembler utility, for terminal- / mobile- and NAS-devices.

sshlog is a command-line log-assembler utility, meant for Linux administrators / datacenter operators with sufficient command-line interface (CLI) experience. It was prototyped to be useful on computer-terminals and mobile devices, like Android-powered smart-phones/tablets/notebooks and similar low-power computers.

Allthough originally written for server-use, it has proven to be quite a useful log-assembler tool on any Linux-box that runs an OpenSSH-server :)

Personally, I've deployed it on all my Linux-powered devices (like: NAS-boxes, laptops, VPS', workstations, routers, etc.) to provide simple, text-based log-access throughout my network-infrastructure. Giving me complete access-history at my fingertips, wherever, whenever.

what does sshlog do?
======
sshlog generates a log of SSH connections made to a Linux system, filters results based on command-line arguments provided and pipes the results to the screen, or, into a timestamped textfile in "~/ssh-logs" in your home-directory. It can filter log-results based on accepted/failed login(s), or by authentication-method(s) used by the remote connection (password / publickey / PAM).

Compatibility
======
This version runs on Debian GNU/Linux (all versions).
