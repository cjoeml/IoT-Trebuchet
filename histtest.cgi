#!/usr/bin/env python
import subprocess


format = "fdsakfjda"

plot = subprocess.Popen(["gnuplot"], stdin=subprocess.PIPE)
output = plot.communicate(input=format)
print output