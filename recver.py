#! /usr/bin/python
import subprocess as subproc
from time import sleep
import sys

MAX_MSG_DIGITS = 6 # Up to 999999

ran = subproc.run(["./client", sys.argv[1], sys.argv[2]], input="GETL".encode("ascii"),
        capture_output=True)
print(ran.stderr.decode("ascii"), end="")

last = -1
while True:
    ran = subproc.run(["./client", sys.argv[1], sys.argv[2]],
            input="GETL".encode("ascii"),
            capture_output=True)
    cliret = ran.stdout.decode("ascii").split(sep="\n")[-2]
    if cliret[:4] == "RMSG":
        if last != int(cliret[4:4+MAX_MSG_DIGITS]):
            last = int(cliret[4:4+MAX_MSG_DIGITS])
            print(cliret[4+MAX_MSG_DIGITS:])

    sleep(0.2)
