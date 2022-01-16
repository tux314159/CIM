#! /usr/bin/python
import subprocess as subproc

MAX_MSG_DIGITS = 6 # Up to 999999

ran = subproc.run(["./client", "localhost", "9999"], input="GETL".encode("ascii"),
        capture_output=True)
print(ran.stderr.decode("ascii"), end="")

while True:
    msg = input("> ")
    if msg == "": continue

    ran = subproc.run(["./client", "localhost", "9999"],
            input="".join(["SEND", msg]).encode("ascii"),
            capture_output=True)
    cliret = ran.stdout.decode("ascii").split(sep="\n")[-2]
    if cliret[:4] == "RMSG":
        print(cliret[4:])
    elif cliret[:4] == "RECV":
        print("Received, message no. " + str(int(cliret[4:]))) # To get rid of leading 0"s
