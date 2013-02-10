#!/bin/bash

#---------------------------------------------------------------------
# CSci 4061 Spring 2013
# Assignment# 1
# Student name: Matthew Lewis, Chris Drews
# Student id: 4237332, id2
# x500 id: lewi0486, id2
# Operating system on which you tested your code: Linux, Unix, Solaris
# CSELABS machine: <machine you tested on eg: xyz.cselabs.umn.edu>
#---------------------------------------------------------------------

echo " "

######################
# Directives for usage

  output="SELECT THE FUNCTION YOU WANT TO EXECUTE: \n" 
  echo -e $output
  output="1. Search for files" 
  echo -e $output
  output="2. Calculate total length" 
  echo -e $output
  output="3. Find zero length files" 
  echo -e $output
  output="4. Create backup files" 
  echo -e $output
  output="5. Exit" 
  echo -e $output
       

  echo -e "\nEnter option "
  read option
  
  case "$option" in

  1)  echo "Searching for files"
      #here you should implement for search files
      #Begin Code
      
      #End Code
      ;;
  2)  echo "Calculating total of the size of all files in the directory tree"
      #here you should implement the code to calculate the size of all files in a folder
      #Begin Code
      
      #End Code
      ;;
      
  3) echo "Finding zero length files"
      #here you should implement the code to find empty files
      #Begin Code
      
      echo -e "\nEnter directory name: "
      read dirname
      find $dirname -size 0
      
      #End Code
      ;;
      
   4) echo  "Creating backup files"
      #here you should implement the backup code  
      #Begin Code

      # do something for each file in the directory
      echo -e "\nEnter directory name: "
      read dirname
      for file in $dirname/*
      do
        # if $file is a file and not a directory
        if [ -f $file ]
        then
          echo "File: $file"
        fi
      done

      #End Code
      ;;
      
  5) echo "Exiting the program. Thanks"
    exit
    ;;
   esac