# Compile to binary
gcc -O2 -o sshlog sshlog.c

# Strip binary
strip sshlog
