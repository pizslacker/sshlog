sshlog v1.6
===========

A command-line Secure SHell log-assembler utility, for client-side terminal- / mobile- and NAS-use.

sshlog is a command-line log-assembler utility, meant for Linux administrators / datacenter operators with sufficient command-line interface (CLI) experience. It was prototyped to be useful on computer-terminals and mobile devices, like Android-powered smart-phones/tablets/notebooks and similar low-power computers.

<img src="https://lh4.googleusercontent.com/-MBfaih-yCwU/UZDFuvdFf7I/AAAAAAAACeM/zyx4jH4ZR9k/s1000/sshlog-failed-using-less-root.png" width="500px" title="sshlog (cli) @PuTTY" />

Allthough originally written for server-use, it has proven to be quite a useful log-assembler tool on any Linux-box that runs an OpenSSH-server :)

Personally, I've deployed it on all my Linux-powered devices (like: NAS-boxes, laptops, VPS', workstations, routers, etc.) to provide simple, text-based log-access throughout my network-infrastructure. Giving me complete access-history at my fingertips, wherever, whenever.

<img src="https://lh4.googleusercontent.com/-7c8yB44g20M/US0NYA0T5iI/AAAAAAAABbY/VJKxdmC8HJ8/w867-h632-no/sshlog-viewer-1.5-20130226-2.png" width="500px" title="sshlog-viewer (gui) @PuTTY" />

I mainly made it to work on Ubuntu Server and certain derivatives  (Ubuntu Desktop and Linux Mint).

There are also branches for: Debian GNU/Linux and Netgear RAIDiator (ReadyNAS),

If you get a mod of your own to run on a specific distribution, please, feel free to mail a copy to me and I'll add it to the repo (and give appropriate accredidation on the website, of course)

<img src="https://lh4.googleusercontent.com/-hWHfIMKKQkI/UqmeloDDAAI/AAAAAAAAGAY/8lmCDmqil7o/w483-h805-no/Screenshot_2013-12-11-02-23-18.png" width="250px" title="sshlog running through ConnectBot on Android v4.x" />

what does sshlog do?
======
sshlog generates a log of SSH connections made to a Linux system, filters results based on command-line arguments provided and pipes the results to the screen, or, into a timestamped textfile in "~/ssh-logs" in your home-directory. It can filter log-results based on accepted/failed login(s), or by authentication-method(s) used by the remote connection (password / publickey / PAM).
