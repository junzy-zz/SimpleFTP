#!/bin/bash
 
# clear the screen
tput clear
 
# Move cursor to screen location X,Y (top left is 0,0)
tput cup 3 15
 
# Set a foreground colour using ANSI escape
tput setaf 3
echo "File Sharing Protocol"
tput sgr0
 
tput cup 5 17
# Set reverse video mode
tput rev
echo "Instructions"
tput sgr0
 
tput cup 7 15
echo "1. IndexGet - Displays the Files on the server side"
 
tput cup 8 15
echo "2. FileUpload - Allows you to upload a file from current directory to the server"
 
tput cup 9 15
echo "3. FileDownload - Allows you to download a file from server"
 
tput cup 10 15
echo "4. quit - Exits the application"
 
# Set bold mode 
tput bold
tput cup 12 15
read -p "Press [ENTER] to continue" choice
 
tput clear
tput sgr0
tput rc
