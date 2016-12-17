#!/usr/bin/env python
import sys
import os
#generate new image
os.system("raspistill -o last_img.jpg -n --timeout 1000")

sys.stdout.write("Content-type: text/html\n\n")
file = open("trebuchet.template", "r")
data = file.read()
sys.stdout.write(data)
sys.stdout.flush()

