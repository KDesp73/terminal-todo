# todo

A modern TUI todo app for your terminal with truecolor styling.

## Install

```console
$ git clone https://github.com/KDesp73/terminal-todo
$ cd terminal-todo
$ sudo make install
```

A different `TODO.txt` is created per-directory.

## Keys

| Key                | Functionality        |
|--------------------|----------------------|
| `j` / `↓`         | Navigate down        |
| `k` / `↑`         | Navigate up          |
| `a`                | Add new task         |
| `q` / `Ctrl+C`    | Quit                 |
| `Tab` / `Shift+Tab` | Switch tab         |
| `h` / `←`         | Move task left (prev status) |
| `l` / `→`         | Move task right (next status) |
| `Shift+↑`         | Increase priority    |
| `Shift+↓`         | Decrease priority    |

## Statuses

| Tab    | Meaning        |
|--------|----------------|
| TODO   | Planned tasks  |
| PROG   | In progress    |
| TEST   | Testing        |
| DONE   | Completed      |

Tasks in each tab are sorted by priority (highest first).

## Build

```console
$ make all
```

Install to a custom prefix:

```console
$ make install PREFIX=~/.local
$ make uninstall
```

## License

[The Unlicense](./LICENSE)

## Author

[KDesp73](https://github.com/KDesp73)