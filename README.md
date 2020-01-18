# janet-rlrepl

A jpm package that contains a repl using readline.

# Usage

```
janet -e '(import rlrepl) (rlrepl/rlrepl)'
```

Don't pass `-s` or `-r` to janet because those options start built-in REPL.

# Configuration

Before running `(rlrepl)`, you can set ```rlrepl/*history-file*``` to a path
you want to save your history.

