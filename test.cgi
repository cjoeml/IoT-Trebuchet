#!/usr/bin/env python
if __name__ == '__main__':
	import subprocess
	import os

    ## print("this is a test script")

    out = subprocess.check_output("./ftw8 ", shell=True)
    pwd = os.getcwd()

    ## This is for testing purposes
    ## cmd = "./ftw8 " + pwd

    ## Create popen
    a = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
	
	## Find the output (b)
	b, err = a.communicate()
	
	## Encode it back to a string (was bytes before)
	out = b.decode(encoding='UTF-8'))

    ## out = check_output(["gnuplot"])

    ## popen and subprocess gnuplot (a = subprocess.popen(cmd, std=subprocess.pipe)
    ## a.communicate(
    ## subprocess.check_output(histogram)
    ## write E at the end for end of file 
