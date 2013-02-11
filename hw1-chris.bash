#!/bin/bash

#---------------------------------------------------------------------
# CSci 4061 Spring 2013
# Assignment# 1
# Student name: Christian Drews, Matthew Lewis
# Student id: 4368668,  <id2 (optional)>
# x500 id: drews032, <id2 (optional)>
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

#  echo $option	
  
  case "$option" in

	1)  echo "Searchng for files"
	    #here you should implement for search files
	    # Begin Code
	    output="Enter directory name: "
	    echo -n $output
	    read directory
	    if test -d "$directory"
		then 
		    path=find ~ -name $directory
		    output="Enter file name: "
		    echo -n $output
		    read file
		    if test -f file
			then
			    find $path -name $file
			else
			    echo "$file can not be found in specified directory"
		    fi
		else
		    echo "$directory does not exist or is not a directory file"
	    fi
	    #End Code
	    ;;
	2)  echo "Calculating total of the size of all files in the directory tree"
	    #here you should implement the code to calculate the size of all files in a folder
	    # Begin Code
	    output="Enter directory name: "
	    echo -n $output
	    read directory
	    if test -d "$directory"
		then 
		    ls $directory > stat -c %s $directory
		else
		    echo "$directory does not exist or is not a directory file"
	    fi
	    #End Code
	    ;;
	    
	3) echo "Finding zero length files"
	    #here you should implement the code to find empty files
	    # Begin Code
	    output="Enter directory name: "
	    echo -n $output
	    #End Code
	    ;;
	    
	 4) echo  "Creating backup files"
	    #here you should implement the backup code  
	    # Begin Code
	    
	    #End Code
	    ;;
	    
	5) echo "Exiting the program. Thanks"
	  exit
	  ;;
   esac
  
  
  
  
