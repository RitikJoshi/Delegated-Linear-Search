/*                                                 

Problem Statement  - Delegated Linear Search

Objective:  Implementation of Distributed Linear Search using Linux System calls - An array is
split up into sub-arrays and searching the sub-arrays for an integer is delegated to other processes.

Input: The program should take two arguments as input - an array of integers to be read from a
file (first argument is the path to the file) and an integer to be searched (second argument).


Output: Print index of the number to be searched if found, or ‘Number not found’.


Method:
The main program reads the complete input array as a global array. The main program creates 2
processes and gives one half of the input array to the 2 child processes to search the required
number. If the size of the segment is small enough (say <= 5), then it searches the number in the
segment by itself. Otherwise, the task is delegated by creating more child processes wherein each
process is given a segment of the array to handle and the number to find.
If any process finds the required number, then it sends a signal to the main process and also returns
the index. After this, the main process kills all the other processes and prints the index. The code
should also handle the case when the number is not present in the array.

Assume PIPE as the chosen method of IPC mechanism.

TO RUN: 
$ gcc assignment.c
$ ./a.out file_name target_value(int)

*/











#include<stdio.h>                  //header file to perform standard I/O functions
#include<unistd.h>                 //header file to use NULL,pipe,fork and sizeof functions
#include<stdlib.h>                 //header file to perform functions involving memory allocation, process control, conversions and others
#include<sys/wait.h>               //header file to perform wait system call 
#include<string.h>                 //header file to perform string operations
#include<signal.h>                 //header file to perform signals
pid_t wpid;                        //process id integer type
int arr[100];                      //global array where integers to be stored from file
int readfile(char argv[])          
{
/*
objective : to read file and store its data into a array
input     : file name
output    : no of integers in file and array with integers that were in file
*/
	FILE*fptr;
	char str[5000];
	

	fptr=fopen(argv,"r");  													//opening file in read mode and saving its fd in fptr   
	if(fptr==NULL)  														//if file cannot be opened due to some reason
	{
		printf("cannot open file\n");  
		exit(0);
	}
	char ch=fgetc(fptr); 													//getting the starting chatracter in ch of file using the fd fptr
	int count=0; 															// count for storing no of elements in file 
	
	while(ch!=EOF) 															//for storing all characters in str array of char type
	{
		str[count++]=ch;
		ch=fgetc(fptr);
	}
	char* pt;																//string definition
	count=0;
	pt=strtok(str,"\n");													// seperating string into tokens on basis of \n
	while(pt!=NULL)
	{
		arr[count++]=atoi(pt); 												// converting token to integer and storing it into array 
		pt=strtok(NULL,"\n");  
	}
	fclose(fptr);															//close file 
	return count-1;															//return count of integers in array

}


int delegated_linear_search(int start,int end,int target)
{
/*
objective : to search the target no if the range of send indexes is less than equal to 5 otherwise creates the child processes and send    the range of array that is to be searched in the array in that child process.
input : start and end index of range of array in which target is to be searched, target no which is to be searched.
output : send index to parent process through pipe and print it there and send signal to all processes to terminate if target is searched,and returns to main process if not found.  
*/
	if((end-start+1) <= 5)     												//terminating case and linear searching
	{
		for (int i = start; i <= end; ++i)
			{
				if(arr[i] == target)
				{
					return (i+1); 											// return index if found to parent process

				}
			}
		return (0);        													// return zero if not found
	}
	int fd[2]; 																// file descriptor for pipe
	pid_t left,right;														//pid type initialization for 2 child processes
	
	if(pipe(fd) == -1)    													//creation and checking of validity of pipe 
	{
		printf("Pipe failed");
		return 0;
	}
	
	left = fork();  														// creation of first child process
	
	if (left == 0)   														// working in first child process
	{	
		int ret_val; 
		close(fd[0]); // closing the read file descriptor of pipe
		printf("Left(r) : pid=%d pgrp=%d	start index :%d	end index : %d\n",getpid(),getpgrp(),start,((start+end)/2));	
		ret_val = delegated_linear_search(start,(start+end)/2,target); //recursive calling of delegated linear search
		
		if(ret_val != 0)
			write(fd[1],&ret_val,sizeof(ret_val)); 							//if found the target then writing its index to pipe
		close(fd[1]); 														// closing write file descriptor
		exit(0);   															//exit system call for normal termination
	}
	if (left > 0)   														// working in parent process
	{
		right = fork();  													// creating second child process
	}
	if (right == 0)  														// working in second child process
	{
		int ret_val;
		close(fd[0]);    													//closing read fd of pipe
		printf("Right(r) : pid=%d pgrp=%d	start index :%d	end index : %d\n",getpid(),getpgrp(),(((start+end)/2)+1),end);	
		ret_val = delegated_linear_search((start+end)/2+1,end,target); 		// recursive calling of delegated linear search
		if(ret_val != 0)	
			write(fd[1],&ret_val,sizeof(ret_val)); 							//if found target value write it to pipe for parent process
		close(fd[1]);   													// closing write fd of pipe 
		exit(0);															//exit 
	}
	
    if(left > 0 && right > 0) 												// working in parent process
	{
		int count,ret_val2;		
		wait(NULL); 														//waiting for child process to exit
		wait(NULL); 														//waiting for child process to exit
		close(fd[1]); 														// write fd closed of pipe
		printf("Parent(r) : pid=%d pgrp=%d	start index :%d	end index : %d\n",getpid(),getpgrp(),start,end);	
		if((count = read(fd[0],&ret_val2,sizeof(ret_val2))) > 0)			// checking if pipe contains some data
		{
			printf("index = %d\n",ret_val2); 								//printing the index of the searched no
			kill(0,SIGKILL);												//sending signal to all the processes to terminate
			
		}
		else 
			return 0;  														//return 0 to parent process if not found
		close(fd[0]);														// close read fd of pipe
	}
}

int main(int argc, char *argv[]) 											// main function to run the delegated search
{
	char *fname; 															// char variable for filename
	int count,target; 
	if(argc!=3)  															// checking for valid arguments
	{
		printf("Invalid arguments\n");
		return 0;
	}	
    
	if(argv[1] != NULL)	//for readfile		
	{
        printf("Searching in file: %s\n", argv[1]);
		count=readfile(argv[1]); 										// calling the readfile function to read data from file to array & store the number of 		elements in file into count
		target=atoi(argv[2]);   										//storing the no. to be searched into target
   	}

    


	
	int fd[2],start=0,end=count;
	int ret_val2;
	
	if(pipe(fd) == -1) 														// creating and validating pipe
	{
		printf("Pipe failed");
		return 0;
	}
	pid_t left,right; 
	left = fork();  														//creating first child process
	if (left == 0)   														//inside first child process
	{	
		int ret_val; 
		close(fd[0]); 														//close read fd of pipe
		printf("Left(m) : pid=%d pgrp=%d	start index :%d	end index : %d\n",getpid(),getpgrp(),start,((start+end)/2));	
		ret_val = delegated_linear_search(start,(start+end)/2,target);   	//calling delegated linear search function
		if(ret_val != 0)     
			write(fd[1],&ret_val,sizeof(ret_val));     						//if found the target then writing its index to pipe
		close(fd[1]);  														// closing write fd of pipe 
		exit(0);  															//exit
	}
	
	if (left > 0)    														// working in parent process
	{
		right = fork();    													// creating second child process
	}
	
	if (right == 0)      													// working in second child process
	{
		int ret_val;
		close(fd[0]);   													//closing read fd of pipe
		printf("Right(m) : pid=%d pgrp=%d	start index :%d	end index : %d\n",getpid(),getpgrp(),(((start+end)/2)+1),end);	
		ret_val = delegated_linear_search((start+end)/2+1,end,target);   	// calling of delegated linear search
		
		if(ret_val != 0)	
			write(fd[1],&ret_val,sizeof(ret_val)); 							//if found the target then writing its index to pipe
		close(fd[1]); 														// closing write fd of pipe 
		exit(0);   															//exit system call for normal termination	
	}
    	
	if(left > 0 && right > 0)   											// working in parent process
	{
		wait(NULL);															//waiting for child process to exit
		wait(NULL); 														//waiting for child process to exit
		close(fd[1]); 														// write fd closed of pipe
		printf("Parent(m) : pid=%d pgrp=%d	start index :%d	end index : %d\n",getpid(),getpgrp(),start,end);		
		if((count = read(fd[0],&ret_val2,sizeof(ret_val2))) > 0)   			// checking if pipe contains some data
		{
			printf("Index = %d\n ",ret_val2);								//printing the index of the searched no
			kill(0,SIGKILL);												//sending signal to all the processes to terminate
		}
		else 
			printf("Not found\n");											//print not found if target not found
		close(fd[0]);														// close read fd of pipe
	}
	return 0;
}
/*
REFERENCES:
-> https://www.geeksforgeeks.org/
-> http://pubs.opengroup.org/onlinepubs/009696699/basedefs/signal.h.html
-> https://unix.stackexchange.com/questions/99112/default-exit-code-when-process-is-terminated?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa

*/