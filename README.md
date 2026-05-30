# MINISHELL
Lightweight shell implemented in C that supports builtin commands, external command execution, pipes, job control and signal handling

---

## Supported Commands

### Builtin Commands
| Command | Description |
| --- | --- |
| cd | Change working directory |
| pwd | Present working directory |
| echo $$ | Prints shell PID |
| echo $? | Print last exit status |
| echo $SHELL | Print shell path |
| jobs | List stopped process |
| fg | Resume most recent job in foreground |
| bg | Resume most recent job in background |
| exit | Exit the shell |

### External Commands
Loaded at startup from file ext.txt 

---

## Features 

- **Built-in Command Execution** — Core commands run directly in shell process
- **External Command Execution** — Forked child process with execvp
- **Multi-pipe Support** — Chains any number of commands with '|'
- **Job Control** — Stop (Ctrl+Z),list (jobs), resume(fg/bg) jobs
- **Signal Handling** — Ctrl+C and Ctrl+Z behave correctly for both parent and child processes
- **Custom Prompt** — Change prompt at runtime with (PS1=yourprompt)

---

## Usage

**Run the shell:**
```bash
./minishell
```

**Change prompt:**
```bash
Minishell$: PS1=myshell
myshell$: 
```

**Pipe commands:**
```bash
Minishell$: ls | grep .c | wc -l
```

**Job control:**
```bash
Minishell$: jobs          # list stopped jobs
Minishell$: fg            # resume in foreground
Minishell$: bg            # resume in background
```

---








