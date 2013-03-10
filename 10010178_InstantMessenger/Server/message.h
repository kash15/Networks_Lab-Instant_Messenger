// message.h --> Header file containing the message types and structure of the message

//------------ Kashish Babbar 10010178--------------------// 

//--------------------------------------------------------//

typedef enum im_message_type {
	REGISTRATION_MESSAGE,			// message type for registering a client
	DEREGISTRATION_MESSAGE,			// message for deregistering a user
	INSTANT_MESSAGE				// a general chat message between two users
} im_message_type;

typedef struct im_message{			// structure defining the contents of the message
	im_message_type type;			// type of the message
	char to[256];				// the receiver of the message
	char from[256];				// the sender of the message
	char message[1024];			// the actual chat message  
} im_message;
