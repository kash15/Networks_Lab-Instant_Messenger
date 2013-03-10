//client.cpp -> The code for the UDP client

//------------ Kashish Babbar 10010178--------------------// 

//--------------------------------------------------------//

#include "message.h"
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>								//Include all the necessary header files
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_LEN 256								//MACRO for Maximum length of user names

using namespace std;

char user_name[MAX_LEN], host[MAX_LEN], port[6] ;				//ports go upto 65535
int client_socket=-1;								//client socket descriptor
struct sockaddr_in server_addr;							//struct for the address of the server to which the message is to be sent 


void error(const char *msg)
{
    perror(msg);
    exit(1);
}


bool register_message(im_message msg)						//function to send a register message
{
	int msg_len=sizeof(msg);
	//socklen_t msg_len = sizeof(msg)					//Both Work

	int sent_bytes=sendto( client_socket, &msg, msg_len, 0, (struct sockaddr *) &server_addr, sizeof(server_addr));		//sendto() function for UDP

	if(sent_bytes!=msg_len)							//in case of error returns -1
	{
		cout << "Unable to Register user " << user_name << endl;
		//printf("Error sending packet: %s\n", strerror(errno));
		//exit(1);
		return false;
	}
	return true;

}



void deregister_message()							//function for deregistering the user (called on exit)) 
{	
	im_message dereg_msg;
	int msg_len = sizeof(dereg_msg);
	memset(&dereg_msg,0,sizeof(dereg_msg));					//clear the im_message buffer
	
	dereg_msg.type=DEREGISTRATION_MESSAGE;					//message type is DEREGISTRATION MESSAGE
	strcpy(dereg_msg.from,user_name);					//the user who needs to be deregistered
	
	int sent_bytes=sendto( client_socket, &dereg_msg, msg_len, 0, (struct sockaddr *) &server_addr, sizeof(server_addr));

	if(sent_bytes < 0)							//in case of error returns -1
	{
		cout << "Unable to deregister user " << user_name << endl;
		printf("Error sending packet: %s\n", strerror(errno));
		//exit(1);
		return;
	}
	
	//cout << "Deregistration message sent..!!" << endl;	

}


void send_instant_message(string msg)
{
	int pos_colon=msg.find(':');
	if(pos_colon == -1)							//checking whether the instant message format is valid or not
	{
		cout << "Invalid instant message format" << endl;
		cout << "Format :[user_name:instant_message]" << endl;		//the correct format of the client message
		return; 											
	}

	string to_user = msg.substr(0,pos_colon);				//getting the name from the message
	string msg_content=msg.substr(pos_colon+1);				//getting the message content

	//used for checking(debugging)
	//cout << "Sending Message to : " << to_user << endl;
	//cout << "Message Content : " << msg_content << endl;	

	im_message ins_msg;
	int msg_len=sizeof(ins_msg);
	memset(&ins_msg,0,sizeof(ins_msg));

	ins_msg.type= INSTANT_MESSAGE;
	strncpy(ins_msg.to,to_user.c_str(),sizeof(ins_msg.to));			//be careful while copying string to char array (use c_str() to convert it to C-style string
	strncpy(ins_msg.from,user_name,sizeof(ins_msg.from));		
	strncpy(ins_msg.message,msg_content.c_str(),sizeof(ins_msg.message));	// sizeof() use as third argument to avoid overflow

	int sent_bytes=sendto( client_socket, &ins_msg, msg_len, 0, (struct sockaddr *) &server_addr, sizeof(server_addr));	//send the instant message

	if(sent_bytes < 0)							//in case of error returns -1
	{
		cout << " Unable to send the message to " << to_user << endl;
		printf("Error sending message: %s\n", strerror(errno));
		return;
	}
	
	//cout << "Message Sent..!!" << endl;					//if everything goes well Message Sent
	

	return;
}


int main(int argc, char *argv[])
{
	

	if(argc < 4)
	{
		cout << "Incorrect Argument Number\n" ;
		cout << "USAGE : " << argv[0] << " user_name server_addr server_port" << endl;		//correct format for running the client
		return 1;
	}
				//avoiding overflow
	strncpy(user_name,argv[1],sizeof(user_name));				//getting the name of the user from command line input					
	strncpy(host,argv[2],sizeof(host));					//the server to which the message is to be sent
	strncpy(port,argv[3],sizeof(port));	

	//for checking
		//cout << "User " << user_name << endl;
		//cout << "Host Name " << host << endl;
		//cout << "Server Port " << port << endl;

	client_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);			//Creating UDP socket --> int socket(int domain, int type, int protocol);
	
	if(client_socket < 0)							//socket returns a socket descriptor value > 0 is able to create a socket
	{									// or -1 in case of error
		error("Error Creating Client Socket");				
	}	
	
	//cout << "Client Socket Created Succesfully" << endl;

	memset(&server_addr,0,sizeof(server_addr));				//Zero out socket address

	server_addr.sin_family=AF_INET;						//The address is ipv4
	
	//server_addr.sin_addr.s_addr = inet_addr(host);			// in inet_pton ('p' stands for presentation and 'n' for numeric)
	inet_pton(AF_INET,host,&server_addr.sin_addr);				//converts the C character string pointed to by 'host' into its 32-bit binary network
										//byte ordered value, which is stored through the pointer sin_addr .
	server_addr.sin_port = htons(atoi(port));


	//Sending Registration message when the client starts
	im_message reg_msg;
	int msg_length = sizeof(reg_msg);

	memset(&reg_msg,0,sizeof(reg_msg));

	reg_msg.type = REGISTRATION_MESSAGE;					// it is a registration message
	strcpy(reg_msg.from,user_name);

	int sent_bytes = sendto(client_socket, &reg_msg, msg_length, 0, (struct sockaddr *) &server_addr, sizeof(server_addr));	

	if(sent_bytes < 0)							//in case of error returns -1
	{
		printf("Error sending packet: %s\n", strerror(errno));
		exit(1);
	}
	
	//if server successfully registers the user an acknowledgement is sent (message from ims)
	

	fd_set readfds;								//the file descriptor set for reading (will later include socket and standard input)
	int readSocket=client_socket;						//file descriptor for the socket
	int ready_fds;								//count for number of ready descriptors

	while(1)
	{
		cout << "imc > " << flush ;						//The prompt that the user needs to write after
		//cout.flush();								//nice thing (see function of flush)
		
/*	
	FD_CLR(fd, &fdset)  
	Clears the bit for the file descriptor fd in the file descriptor set fdset.
	
	FD_ISSET(fd, &fdset)  
	Returns a non-zero value if the bit for the file descriptor fd is set in the file descriptor set pointed to by fdset, and 0 otherwise.
	
	FD_SET(fd, &fdset)  
	Sets the bit for the file descriptor fd in the file descriptor set fdset.

	FD_ZERO(&fdset)  
	Initializes the file descriptor set fdset to have zero bits for all file descriptors.
*/
		

		/* Initialize the file descriptor set to include our socket  */
  		/* and the file descriptor for standard input (0).  Then, we */
  		/* can listen to both at once.                               */

		FD_CLR(readSocket,&readfds);
		FD_CLR(0,&readfds);
		FD_ZERO(&readfds);
		FD_SET(readSocket,&readfds);
		FD_SET(0,&readfds);

	/*int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, const struct timeval *timeout);	*/
	      /* Returns: positive count of ready descriptors, 0 on timeout, –1 on error */
		/* We now use select to wait for input at either location. */
  		/* We don't need to timeout, so we leave that part NULL.   */
		
		ready_fds = select(readSocket+1, &readfds, NULL, NULL, NULL);
		
		
		if((ready_fds==1) && (FD_ISSET(readSocket,&readfds)) )
		{
				// We have Input message
			/* We have socket input waiting, and can now do a recvfrom(). */
			
			struct sockaddr_in sending_server_addr;

			im_message received_msg;
			
			//int msg_len=sizeof(sending_server_addr);				//instead of int -> socklen_t required
			socklen_t len_struct=sizeof(sending_server_addr);

			//Receive the message using recvfrom() function 
			int received_bytes = recvfrom(client_socket, &received_msg, sizeof(received_msg), 0, (struct sockaddr *) &sending_server_addr, &len_struct);
			
			if(received_bytes < 0)							
			{
				printf("Error sending packet: %s\n", strerror(errno));		// -1 returned in case of error
			}
			else
			{
				cout << endl;							//to skip past the current client prompt
								
				//If the user sent "exit" the ims sends an acknowledgement for the DEREGISTRATION MESSAGE
				if(strcmp(received_msg.from,"ims")==0 && (strcmp(received_msg.message,"Dereg_Success")==0)) 
				{
					cout << "You have Logged Out Successfully..!!\nBbye..!! :)" << endl;
					close(client_socket);
					exit(0);						//exit(0) and the user exits normally
				}
				else if(strcmp(received_msg.from,"ims")==0 && (strcmp(received_msg.message,"Dereg_Unsuccessful")==0))
				{
					cout << "Sorry But you are not registered..!!\nBbye..!" << endl;
					close(client_socket);
					exit(0);
				}				

											
				cout << received_msg.from << ": " << received_msg.message << endl;	// Printing the received message
							
			}
		}

		if((ready_fds==1) && (FD_ISSET(0,&readfds)) )
		{
				//Have a message to send
			/* We have input waiting on standard input, so we can go get it. */
			string input_text;
			
			/*
			char input[1024];
			cin.getline(input,1024);						//Old way
			input_text = input; 
			*/
												//Better way
			getline(cin,input_text);						// Getting the input from the standard input (i.e. message ready to be sent)
			
			if(input_text == "exit")
			{
				deregister_message();						//if the user types exits send a DEREGISTRATION MESSAGE before exiting		
			}
			else
			{
				send_instant_message(input_text);				//else if it is a normal instant message send it to the destination client through the server
			}

			
			//ready_fds=0;
		}
		
		if(ready_fds < 0)
		{
			error("Error in select()");
		}

	}

	return 0;

}



