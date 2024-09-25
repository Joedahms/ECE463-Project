# ECE463 Intro to Computer Communication Networks Project (Stage 2)

## IMPORTANT - PLEASE LOOK AT THE FLOWCHARTS IN THE FLOWCHART DIRECTORY

## Directories/files
src:        Directory containing all source code
.gitignore: File listings all files and directories to ignore when committing changes via Git
makefile:   See Compilation section below
Flowcharts: Directory containing flowcharts that illustrate the logical flow of our project.  
The flowcharts were made and can be modified with [draw.io](https://app.diagrams.net/).

## Usage
$ %put <file name>
Send a file to the server.

$ %get <file name>
Request a file stored on the remote server.

$ <any plain text message>
Will transmit message to any other clients connected to the server.

## Compilation
This project uses make for compilation. Enter "make" to compile the program, "make clean" to remove all object  
files and executables. Any files/directories that are created when compiling are listed in the gitignore.

## Goal (Stage 2)
Our goal for stage 2 is to expand on stage 1 to implement UDP messaging between clients through the server.  
The client should send commands and plain text messages to server via UDP. If the server receives a plain text  
message, it should relay that message to all connected clients. Any file contents exchanged through put and   
get commands should be done over a TCP connection.

