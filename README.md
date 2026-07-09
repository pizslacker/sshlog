sshlog v2.0
===========

**sshlog** is a server-side log processing tool to assemble ssh-connection logs, meant firstly for Linux-administrators / datacenter-operators with sufficient command-line interface (CLI) experience, or mostly anyone who can benefit from using it ;)

At first it was a Bash script, but as of version 1.8 it also comes as a C-implementation (`src/c/`).

It was prototyped to be useful on small computer-terminals and mobile devices, like Android smart-phones / -tablets / -netbooks.

what does sshlog do?
======
**sshlog** generates a log of SSH connections made to a Linux system, filters results based on command-line arguments provided and pipes the results to the screen, or, into a timestamped textfile in "~/ssh-logs" in your home-directory. It can filter log-results based on accepted/failed login(s), or by authentication-method(s) used by the remote connection (password / publickey / PAM).

<img src="https://lh3.googleusercontent.com/pw/AL9nZEWqZeFp91CRyGOixcrKjv8pI-vVJKmzpsxORyVR3jgaRgXi-uwBZHENn6IWJ56X8gcag6f4MQ38KQYoOVr5GxtzUferNzJ6wu8P6XtEWFU_EbtDWT9S2_yE-3QFl__ndJvVzV4cEn-cNuyoQVow_UueUw=w867-h632-no" width="500px" title="sshlog (cli) @PuTTY" />

Although originally written for bigrig- / server-use, it has proven to be quite a useful little log-tool on any Linux-box that runs an `OpenSSH`-server :) both for account-auditing and **p4r4n0|4!**.

```
I highly recommend installing "Fail2Ban" to defend against SSH-bruteforcing!
```

Personally, I've deployed it on all my Linux-powered devices (like: NAS-boxes, laptops, VPS', workstations, routers, etc.) to provide simple, text-based log-access throughout my network-infrastructure. Giving me complete access-history at my fingertips, wherever, whenever.

<img src="https://lh3.googleusercontent.com/pw/AL9nZEUQZQKYLv0uthTG3sHWerm7N-o4E7YL5y5mI6t7OmhHbamAqnEs4oO64pzPHLOazSkh3K8I7jGoe8Y7VXyaVPTyGXQiIrFfCw8rqzNB1mQ9JMvvc4sAnE2raZceH5zZ2iCL_e2q8IB0E3uYCcLmMUBYzQ=w867-h632-no" width="500px" title="sshlog-viewer (gui) @PuTTY" />

I mainly made it to work on Ubuntu Server and certain derivatives  (Ubuntu Desktop and Linux Mint).

installing
=======

- Copy the shell-script(s) / C binary to `/usr/local/sbin`:

- Then copy (select) man1-files (< program-name >.1.gz) to `/usr/local/share/man/man1`.

```
sudo cp man/sshlog.1.gz /usr/local/share/man/man1
```
