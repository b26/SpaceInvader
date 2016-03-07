#Name: Bashir Osman
#UNIX: bashir1@ualberta.ca
#CCID: 1174263
#Lecture Section: A2
#Lab Section: D02
#TA Name: Jincheng Mei

#Description
#Creates executable file named lander

lander: lander.c helper.c helper.h
	gcc -Wall -std=c99  -o lander lander.c helper.c -lm
clean:
	rm -rf *.o lander core
