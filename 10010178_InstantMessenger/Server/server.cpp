//server.cpp - The code for the UDP server

//------------ Kashish Babbar 10010178--------------------// 

//--------------------------------------------------------//

#include "message.h"								//Including the message.h file for im_messsage struct 
#include <iostream>

#include <stdio.h>
#include <stdlib.h>								//Include all the necessary header files
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <map>									//map library for storing the registered users using a map
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT_NO 8888								//The port number on which the server is running
#define MAX_LEN 256								//the maximum length of the user names

using namespace std;

int server_socket=-1;								//server socket descriptor
struct sockaddr_in server_addr;							//struct for the address of the server


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc,char *argv[])
{
	
	// Map to store the registered users
	map<string,struct sockaddr_in> user_table;				//Map between user(string) and it's socket address(sockaddr_in)
	map<string,struct sockaddr_in> :: iterator it;					

	server_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);			//Creating UDP socket --> int socket(int domain, int type, int protocol);

	if(server_socket<0)
	{
		error("Error Creating Socket");					//socket returns a socket descriptor value > 0 is able to create a socket
	}									// or -1 in case of error

	//cout << "Server Socket Created Successfully\nSocket FD : " << server_socket << endl;
	
	memset(&server_addr,0,sizeof(server_addr));				//clearing the struct server_addr

										
										//	struct sockaddr_in {
										// 	      short   sin_family; // should be AF_INET 
										//  	      u_short sin_port;
										//  	      struct  in_addr sin_addr;
										//  	      char    sin_zero[8]; // not used, must be zero 	};
	server_addr.sin_family = AF_INET;					// AF_INET-> 2 and AF_INET6 -> 10
	server_addr.sin_addr.s_addr = INADDR_ANY;	//to bind to the local IP address			
	server_addr.sin_port = htons(PORT_NO);
										//	struct in_addr {
										//   		unsigned long s_addr;
										//	};	
	
	//Binding (Associating the socket created with a port on the local machine)
	
	if(bind(server_socket,(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		close(server_socket);
		error("Error While Binding");
	}

	cout << "Server is up and Running on PORT 8888" << endl; 		//If everything goes well the server is running on port 8888

	while(1)
	{

		cout << endl;
		string from_client,to_client,msg;				//strings used as map is between 'string' and 'sockaddr_in'

		/*
		for(it=user_table.begin();it!=user_table.end();++it)
		{
			cout << it->first << " => " <<  inet_ntoa((it->second).sin_addr) << " " << htons((it->second).sin_port) << endl;		//To see who all are registered
		}		
		*/

		//The server is now Waiting for data

		im_message received_msg, to_send_msg;				//im_message structs for received message and the message to be sent
		memset(&received_msg, 0, sizeof(received_msg));			//clear the buffers
		memset(&to_send_msg, 0, sizeof(to_send_msg));		

		struct sockaddr_in sending_client, to_send_client;		//structs for storing the address info
		memset(&sending_client, 0, sizeof(sending_client));
		memset(&to_send_client, 0, sizeof(to_send_client));

		socklen_t len_struct=sizeof(sending_client);


		//Receiving data from the clients
		int received_bytes=recvfrom(server_socket, &received_msg, sizeof(received_msg), 0, (struct sockaddr *) &sending_client, &len_struct);

		if(received_bytes==-1)
		{
			continue;						//if nothing is received yet continue to wait				
		}	
		else
		{
				
			from_client=received_msg.from;	
			to_client=received_msg.to;
			msg=received_msg.message;
		
			//Used for checking
			//cout << "Receiving Data from " << from_client << " address:" << inet_ntoa(sending_client.sin_addr) << " port: " << ntohs(sending_client.sin_port) << endl;
			
			

			//If the message received is a REGISTRATION MESSAGE check for the user in the map
			if(received_msg.type==REGISTRATION_MESSAGE)				
			{
				im_message ack_msg;
				memset(&ack_msg,0,sizeof(ack_msg));

				strcpy(ack_msg.from,"ims");
				

				//Search for the user, if user not found add a new entry otherwise update the address information
				it = user_table.begin();
				it = user_table.find(from_client);
				
				//User already registered (Update it's info)
				if(it!=user_table.end())
				{
					strcpy(ack_msg.message,"You have Already been Registered..Updating your Address Info.");
					//cout << it->first << endl;

					user_table[from_client]=sending_client;						//Update the address information
					cout << "User Already Exists.\nUpdated User Info "<< from_client << endl;	//wont occur in our case though
				}								//user never sends a msg with REGISTRATION_MESSAGE type except at start
				else								
				{
					//User not found, so register him
					
					user_table[from_client]=sending_client;				//User is registered successfully
					cout << "Registered User "<< from_client << endl;
					
					strcpy(ack_msg.message,"You have been Registered Successfully.\nWelcome..!!");	
				}

				//Send an Acknowledgement Message back to the user
				int sent_bytes=sendto(server_socket, &ack_msg, sizeof(ack_msg), 0, (struct sockaddr *) &sending_client, sizeof(sending_client));
				if(sent_bytes<0)
				{
					cout << "Error sending Acknowledgement..!\n";
				}
				
			
			}
			else if(received_msg.type==DEREGISTRATION_MESSAGE)			//The user has typed "exit" i.e. a DEREGISTRATION MESSAGE has been sent
			{
				im_message ack_msg;
				memset(&ack_msg,0,sizeof(ack_msg));

				strcpy(ack_msg.from,"ims");				

				//Delete User from the table (map)
				it = user_table.begin();
				it = user_table.find(from_client);
				
				if(it!=user_table.end())					//user is found
				{
					//Deleting user from the table
					user_table.erase(it);					//user successfully deleted			
					cout << "User " << from_client << " Deregistered.(Entry deleted from map)" << endl;	
					strcpy(ack_msg.message,"Dereg_Success");		//send an acknowledgement that the user is deregistered
				}
					
				else								//if(it==user_table.end()) i.e. if user not found
				{
					cout << "User " << from_client << " doesn't exist." << endl;
					strcpy(ack_msg.message,"Dereg_Unsuccessful");
				}
		
				//Send an acknowledgement message for deregistration
				int sent_bytes=sendto(server_socket, &ack_msg, sizeof(ack_msg), 0, (struct sockaddr *) &sending_client, sizeof(sending_client));
				
				if(sent_bytes<0)
				{
					cout << "Error sending Acknowledgement..!\n";
				}
				
			
			}
			else if(received_msg.type==INSTANT_MESSAGE)
			{
				cout << "Instant_Message From '" << received_msg.from << "' to '" << received_msg.to << "' Message: " << received_msg.message << endl; //Message from and message to
				
				//Check whether the user to whom the message is sent is registered ot not
				//If registerd -> send him the message
				//If not registered -> send a message to the sender that the user [name] is not registered
				
				it=user_table.begin();
				it=user_table.find(to_client);
				
				if(it!=user_table.end())				//the user to whom message is to be sent is present
				{
					to_send_client=it->second;			//get the address of the user to whom the message is to be sent from the map

					//Send the message to the intended user
					int sent_bytes=sendto(server_socket, &received_msg, sizeof(received_msg), 0, (struct sockaddr *) &to_send_client, sizeof(to_send_client));
		
					if(sent_bytes==-1)
					{
						cout << "Error Sending Message to " << to_client << endl;
					}
					else
					{
						//cout << "Message Sent Succesfully to " << to_client << endl; 
					}			
				}
				else							//the user to whom message is to be sent doesn't exist
				{
					//If not registered -> send a message to the sender that the user [name] is not registered
					to_send_client=sending_client;
					
					string mesg="User '";
					mesg+= to_client;				//The message from ims that the user is not registered
					mesg+= "' not registered.";
			
					cout << mesg << endl;

					strcpy(to_send_msg.message,mesg.c_str());
					strcpy(to_send_msg.from,"ims");
					strcpy(to_send_msg.to,from_client.c_str());
					to_send_msg.type=INSTANT_MESSAGE;
					
					//Sending the message to the user that the user to whom the message was sent is not registered
					int sent_bytes=sendto(server_socket, &to_send_msg, sizeof(to_send_msg), 0, (struct sockaddr *) &to_send_client, sizeof(to_send_client));
		
					if(sent_bytes==-1)
					{
						cout << "Error Sending Message to " << from_client << endl;
					}

					/*else
					{
						cout << "User not exists Message sent to " << from_client << endl;
					}*/		

				}				

			}
		}

	}
							

}

