sshlog v1.6
===========

A command-line (<b>S</b>)ecure (<b>SH</b>)ell (<b>LOG</b>)-utility, for client-side terminal- / mobile- and NAS-use.

<b>sshlog</b> is a Bash-script to assemble ssh-connection logs, meant firstly for Linux-administrators / datacenter-operators with sufficient command-line interface (CLI) experience, or mostly anyone who can benefit from using it ;)

It was prototyped to be useful on small computer-terminals and mobile devices, like Android smart-phones / -tablets / -netbooks, and nowadays usually on low-end Windows 10 hardware and similar low-power computers.

what does sshlog do?
======
sshlog generates a log of SSH connections made to a Linux system, filters results based on command-line arguments provided and pipes the results to the screen, or, into a timestamped textfile in "~/ssh-logs" in your home-directory. It can filter log-results based on accepted/failed login(s), or by authentication-method(s) used by the remote connection (password / publickey / PAM).

<img src="https://lh4.googleusercontent.com/-MBfaih-yCwU/UZDFuvdFf7I/AAAAAAAACeM/zyx4jH4ZR9k/s1000/sshlog-failed-using-less-root.png" width="500px" title="sshlog (cli) @PuTTY" />

Although originally written for bigrig/server-use, it has proven to be quite a useful little log-script tool on any Linux-box that runs an <b>OpenSSH</b>-server :) both for account-auditing and <b>p4r4n0|4</b>.
<b>
```
I highly recommend installing "Fail2Ban" to defend against SSH-bruteforcing!
```
</b>
Personally, I've deployed it on all my Linux-powered devices (like: NAS-boxes, laptops, VPS', workstations, routers, etc.) to provide simple, text-based log-access throughout my network-infrastructure. Giving me complete access-history at my fingertips, wherever, whenever.

<img src="https://lh4.googleusercontent.com/-7c8yB44g20M/US0NYA0T5iI/AAAAAAAABbY/VJKxdmC8HJ8/w867-h632-no/sshlog-viewer-1.5-20130226-2.png" width="500px" title="sshlog-viewer (gui) @PuTTY" />

I mainly made it to work on Ubuntu Server and certain derivatives  (Ubuntu Desktop and Linux Mint).

There are also branches for: Debian GNU/Linux and Netgear RAIDiator (ReadyNAS),

If you get a mod of your own to run on a specific distribution, please, feel free to mail a copy to me and I'll add it to the repo (and give appropriate accredidation on the website, of course)

<img src="https://lh4.googleusercontent.com/-hWHfIMKKQkI/UqmeloDDAAI/AAAAAAAAGAY/8lmCDmqil7o/w483-h805-no/Screenshot_2013-12-11-02-23-18.png" width="250px" title="sshlog running through ConnectBot on Android v4.x" />

installing
=======
Copy the shell-script(s) to '/usr/local/sbin':
```
sudo cp sshlog-x.x/bash/sshlog /usr/local/sbin
```

Then copy (select) man1-files (< program-name >.1.gz) to '/usr/local/share/man/man1'.
```
sudo cp sshlog-x.x/man/sshlog.1.gz /usr/local/share/man/man1
```
