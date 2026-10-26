# Starling-Shell

A custom command-line shell implemented in C, built as part of an Operating Systems coursework project. Designed for Linux environments, this shell serves as a functional proof-of-concept for understanding core OS principles.

## Features

- **Command Execution:** Parses and executes standard system commands using process forks and execution lifecycles.
- **Output Redirection:** Supports standard output redirection (`>`) using file descriptors and system calls.
- **Custom Built-ins:** Implements custom internal commands including `cd`, `touch`, `echo`, and `cat`.
- **Custom Prompt:** Features a styled, colored command prompt via ANSI escape sequences.

## Technical Details

- **Language:** C
- **Platform:** Linux
- **Key Concepts:** Process creation (`fork`, `exec`, `waitpid`), file descriptor duplication (`dup2`), and string tokenization.

## Getting Started

### Prerequisites
- A Linux environment
- A C compiler (e.g., `gcc`)

### Building and Running
```bash
# Clone the repository
git clone https://github.com/KericAnela/Starling-Shell.git
cd Starling-Shell

# Compile the source code
gcc -o starling main.c

# Run the shell
./starling
