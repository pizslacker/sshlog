sshlog v1.7
===========

A command-line **S**ecure **SH**ell **LOG**-utility, for client-side terminal- / mobile- and NAS-use.

**sshlog** is a Bash-script to assemble ssh-connection logs, meant firstly for Linux-administrators / datacenter-operators with sufficient command-line interface (CLI) experience, or mostly anyone who can benefit from using it ;)

It was prototyped to be useful on small computer-terminals and mobile devices, like Android smart-phones / -tablets / -netbooks, and nowadays usually on low-end Windows 10 hardware and similar low-power computers.

what does sshlog do?
======
**sshlog** generates a log of SSH connections made to a Linux system, filters results based on command-line arguments provided and pipes the results to the screen, or, into a timestamped textfile in "~/ssh-logs" in your home-directory. It can filter log-results based on accepted/failed login(s), or by authentication-method(s) used by the remote connection (password / publickey / PAM).

<img src="https://lh3.googleusercontent.com/pw/AL9nZEWqZeFp91CRyGOixcrKjv8pI-vVJKmzpsxORyVR3jgaRgXi-uwBZHENn6IWJ56X8gcag6f4MQ38KQYoOVr5GxtzUferNzJ6wu8P6XtEWFU_EbtDWT9S2_yE-3QFl__ndJvVzV4cEn-cNuyoQVow_UueUw=w867-h632-no" width="500px" title="sshlog (cli) @PuTTY" />

Although originally written for bigrig- / server-use, it has proven to be quite a useful little log-tool on any Linux-box that runs an **OpenSSH**-server :) both for account-auditing and **p4r4n0|4!**.

```
I highly recommend installing "Fail2Ban" to defend against SSH-bruteforcing!
```

Personally, I've deployed it on all my Linux-powered devices (like: NAS-boxes, laptops, VPS', workstations, routers, etc.) to provide simple, text-based log-access throughout my network-infrastructure. Giving me complete access-history at my fingertips, wherever, whenever.

<img src="https://lh3.googleusercontent.com/pw/AL9nZEUQZQKYLv0uthTG3sHWerm7N-o4E7YL5y5mI6t7OmhHbamAqnEs4oO64pzPHLOazSkh3K8I7jGoe8Y7VXyaVPTyGXQiIrFfCw8rqzNB1mQ9JMvvc4sAnE2raZceH5zZ2iCL_e2q8IB0E3uYCcLmMUBYzQ=w867-h632-no" width="500px" title="sshlog-viewer (gui) @PuTTY" />

I mainly made it to work on Ubuntu Server and certain derivatives  (Ubuntu Desktop and Linux Mint).

There are also branches for: Debian GNU/Linux and Netgear RAIDiator (ReadyNAS),

If you get a mod of your own to run on a specific distribution, please, feel free to mail a copy to me and I'll add it to the repo (and give appropriate accredidation on the website, of course)

<img src="https://lh3.googleusercontent.com/yn4P5e8RSGjHems3ZiejL1Hh14M8U65amp2oiUDe0Qprs4zlplqqpY3uM1ps715VtiCO-77uPynFBo4RwKf1ge7OwAWEsyujyYknpQExJEQdiVVE-UZWaeOK2H_x0VvUjEYMkwgQXOPaxlAQOF6YMQNwE2OjtE4eegWDVn60m0ALkbAXYchAJYhF3CsiWSSStRTiEoZYh7iZ_vhhWzsS0TYW32dGkDgvTvyci1xt9hyLOGbuacg1Ad0QkQOtSbUxS601uop6n_R3aj8EFMyRpnfUwogpUnZtBqjMpvQhJHKTLWtaX2USi4S0_Z7aSjo-8OQCpsvGhxzjgOnAp4l6w41ks8H-zNgVHZnwNy6x5GdjissKjcda2_PPVF3_iNl3SruNJOsPBACQsBAMNNffDhMIbXQ4WitA1e_pkIAZTv2crKIQ9nfTkp8DL7LI3_VemBUA05mKxw1_Wt-fkv6kUl6Z0aQP9cdz9p4xWKQXp-KHFdOgOQFbdpl0Of8GLwGOqkzIb7OXeZQOUpIvyzFvBmWQxBEMEkqfURj_fSc_a0x0uTArZyLZHUoCmINIocpecL8BQ3wbyx2j_hVOO5Yy9o62Zad2VQfX2BoFj_41zVfo74B9sdl1chwBndy2Bj-M2rSakuFOamB-hPaVSoUATamFuaKTX3ezRBUWVTS5C3zGYeRhEunX1-FXnHarV6OGeMMfvglNr5azbaB92zHK7TnU_72L3Ru-tMebTch7q81KBaG7ADGf3hLhEbGv29w=w509-h848-no" width="250px" title="sshlog running through ConnectBot on Android v4.x" />

installing
=======

- Copy the shell-script(s) to **/usr/local/sbin**:

```
sudo cp sshlog-x.x/bash/sshlog /usr/local/sbin
```

- Then copy (select) man1-files (< program-name >.1.gz) to **/usr/local/share/man/man1**.

```
sudo cp sshlog-x.x/man/sshlog.1.gz /usr/local/share/man/man1
```
