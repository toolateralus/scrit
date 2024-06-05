# System Functions

## fexists
Checks if a file exists.
fexists(filename)
- `filename`: String, the name of the file to check.

## fcreate
Creates a new file.
fcreate(filename)
- `filename`: String, the name of the file to create.

## fwrite
Writes content a file.
fwrite(filename, content)
- `filename`: String, the name of the file.
- `content`: String, the content to write to the file.

## fread
Reads content from a file.
fread(filename)
- `filename`: String, the name of the file to read from.

## fdelete
Deletes a file.
fdelete(filename)
- `filename`: String, the name of the file to delete.

## dir_exists
Checks if a directory exists.
dir_exists(dirname)
- `dirname`: String, the name of the directory to check.

## dir_create
Creates a new directory.
dir_create(dirname)
- `dirname`: String, the name of the directory to create.

## cwd
Returns the current working directory.
cwd()

## dir_getfiles
Returns a list of files in a directory.
dir_getfiles(dirname)
- `dirname`: String, the name of the directory to get files from.

## time
Returns the current time in seconds since the epoch.
time()

## sleep
Pauses execution for a specified number of seconds.
sleep(seconds)
- `seconds`: Integer, the number of seconds to sleep.

## syscall
Executes a system command.
syscall(command)
- `command`: String, the system command to execute.

## exit
Exits the program.
exit()