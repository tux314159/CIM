#! /usr/bin/python
import subprocess as subproc

ran = subproc.run(['./client', 'localhost', '9999'], capture_output=True)

cliret = ran.stderr.decode('ascii')
if cliret == '':
    print(cliret)

cliret = ran.stdout.decode('ascii').split(sep='\n')[-2]
if cliret[:4] == 'RMSG':
    print(cliret[4:])
