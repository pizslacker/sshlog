.PHONY: clean

# Rebuild sshlog whenever any .go file or go.mod changes
sshlog: sshlog.go
	go build -trimpath -ldflags="-s -w" -o $@ .

clean:
	rm -f sshlog
