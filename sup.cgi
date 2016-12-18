#!/usr/bin/env python
import sys

sys.stdout.write("Content-type: text/html\n\n")
file = open("sup.html", "r")
data = file.read()
sys.stdout.write(data)
sys.stdout.flush()