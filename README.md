sshlog v2.0
===========

**sshlog** is a server-side log processing tool to assemble ssh-connection logs, meant firstly for Linux-administrators / datacenter-operators with sufficient command-line interface (CLI) experience, or mostly anyone who can benefit from using it ;)

It was prototyped to be useful on small computer-terminals and mobile devices, like Android smart-phones / -tablets / -netbooks.

It comes in various versions: Bash, C, Go, Python, Rust (located under: `src/`).

what does sshlog do?
======
**sshlog** generates a log of SSH connections made to a Linux system, filters results based on command-line arguments provided and pipes the results to the screen, or, into a timestamped textfile in "~/ssh-logs" in your home-directory.

It can filter log-results based on accepted/failed login(s), or by authentication-method(s) used by the remote connection (password / publickey / PAM).

<img src="https://lh3.googleusercontent.com/pw/AL9nZEWqZeFp91CRyGOixcrKjv8pI-vVJKmzpsxORyVR3jgaRgXi-uwBZHENn6IWJ56X8gcag6f4MQ38KQYoOVr5GxtzUferNzJ6wu8P6XtEWFU_EbtDWT9S2_yE-3QFl__ndJvVzV4cEn-cNuyoQVow_UueUw=w867-h632-no" width="500px" title="sshlog (cli) @PuTTY" />

Although originally written for bigrig- / server-use, it has proven to be quite a useful little log-tool on any Linux-box that runs an `OpenSSH`-server :) both for account-auditing and **p4r4n0|4!**.

I mainly made it to work on Ubuntu Server and certain derivatives  (Ubuntu / Debian).

But have since refactored it to portable shell script to work on any unices: Linux, MacOS, BSD.

installing
=======

- Copy the shell-script(s) / C binary to `/usr/local/sbin`:

- Then copy (select) man1-files (< program-name >.1.gz) to `/usr/local/share/man/man1`.

```
sudo cp man/sshlog.1.gz /usr/local/share/man/man1
```
