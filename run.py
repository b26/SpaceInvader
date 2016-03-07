import os
from sys import argv

os.system("make")
if len(argv) >= 2:
    if argv[1] == "v":
        os.system("valgrind ./lander landscape.txt")
else:
    os.system("./lander landscape.txt")
