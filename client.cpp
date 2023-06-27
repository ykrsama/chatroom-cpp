#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <signal.h>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#define MAX_LEN 200
#define NUM_COLORS 6

using namespace std;

bool exit_flag=false;
thread t_send, t_recv;
int client_socket;
string def_col="\033[0m";
string colors[]={"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};
char exit_message[3] = ".q";
string m_msg_to_send;

void catch_ctrl_c(int signal);
string color(int code);
int eraseText(int cnt);
void send_message(int client_socket);
void recv_message(int client_socket);


static struct termios old, current;

/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  current = old; /* make new settings same as old settings */
  current.c_lflag &= ~ICANON; /* disable buffered i/o */
  if (echo) {
      current.c_lflag |= ECHO; /* set echo mode */
  } else {
      current.c_lflag &= ~ECHO; /* set no echo mode */
  }
  tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
};

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &old);
};

/* Read 1 character - echo defines echo mode */
char getch_(int echo) 
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
};

/* Read 1 character without echo */
char getch(void) 
{
  return getch_(0);
};

// clear char
void clear_char_array(char* str, int len )
{
    for (int i =0; i<len; i++)
    {
        str[i]='\0';
    }
};

int main()
{
	if((client_socket=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket: ");
		exit(-1);
	}

    char server_ip[] = "127.0.0.1";
    //char server_ip[MAX_LEN];
    //cout<<"Enter Server IP : ";
    //cin.getline(server_ip,MAX_LEN);

	struct sockaddr_in client;
	client.sin_family=AF_INET;
	client.sin_port=htons(64799); // Port no. of server
	client.sin_addr.s_addr=INADDR_ANY;
	client.sin_addr.s_addr=inet_addr(server_ip); // Provide IP address of server
	bzero(&client.sin_zero,0);

	if((connect(client_socket,(struct sockaddr *)&client,sizeof(struct sockaddr_in)))==-1)
	{
		perror("connect: ");
		exit(-1);
	}
	signal(SIGINT, catch_ctrl_c);
	//char name[MAX_LEN];
	//cout<<"Enter your name : ";
	//cin.getline(name,MAX_LEN);
	//send(client_socket,name,sizeof(name),0);
    string name_string = getenv("USER");
    const char* name = name_string.c_str();
    send(client_socket,name,name_string.size(),0);

	cout<<colors[NUM_COLORS-1]<<"\n\t  ====== Welcome to the chat-room ======   "<<endl<<def_col;
    cout<<colors[NUM_COLORS-1]<<"\nAppend [name] to send a private message. i.e. You: message [wangzhen]\n"<<endl<<def_col;

	thread t1(send_message, client_socket);
	thread t2(recv_message, client_socket);

	t_send=move(t1);
	t_recv=move(t2);

	if(t_send.joinable())
		t_send.join();
	if(t_recv.joinable())
		t_recv.join();
    initTermios(true);
	return 0;
}

// Handler for "Ctrl + C"
void catch_ctrl_c(int signal) 
{
	initTermios(true);
	send(client_socket,exit_message,sizeof(exit_message),0);
	exit_flag=true;
	t_send.detach();
	t_recv.detach();
	close(client_socket);
	exit(signal);
}

string color(int code)
{
	return colors[code%NUM_COLORS];
}

// Erase text from terminal
int eraseText(int cnt)
{
	char back_space=8;
	for(int i=0; i<cnt; i++)
	{
		cout<<back_space;
	}
    return 0;
}

//// Send message to everyone
//void send_message(int client_socket)
//{
//	while(1)
//	{
//		cout<<colors[1]<<"You : "<<def_col;
//		char str[MAX_LEN];
//		cin.getline(str,MAX_LEN);
//		send(client_socket,str,sizeof(str),0);
//		if(strcmp(str,exit_message)==0)
//		{
//			exit_flag=true;
//			t_recv.detach();	
//			close(client_socket);
//			return;
//		}	
//	}		
//}

void send_message(int client_socket)
{
    while(1)
    {
        cout<<colors[1]<<"You : "<<def_col;
        m_msg_to_send.clear();
        char key;
        while((key = getch()) != '\n')
        {
            if(key == 127 || key == 8)
            {
                m_msg_to_send.pop_back();
                cout << "\b \b";
            }
            else
            {
                m_msg_to_send += key;
                cout<<key;
            }
        }
        send(client_socket, m_msg_to_send.c_str(), m_msg_to_send.size() + 1,0);
        if(m_msg_to_send == exit_message)
        {
            exit_flag = true;
            t_recv.detach();
            close(client_socket);
            return;
        }
        cout<<endl;
    }
}

// Receive message
void recv_message(int client_socket)
{
	while(1)
	{
		if(exit_flag)
			return;
		char name[MAX_LEN], str[MAX_LEN];
		int color_code;
		int bytes_received=recv(client_socket,name,sizeof(name),0);
		if(bytes_received<=0)
			continue;
		recv(client_socket,&color_code,sizeof(color_code),0);
		recv(client_socket,str,sizeof(str),0);
        eraseText(6+m_msg_to_send.size());
		if(strcmp(name,"#NULL")!=0)
			cout<<color(color_code)<<name<<" : "<<def_col<<str<<endl;
		else
			cout<<color(color_code)<<str<<endl;
		cout<<colors[1]<<"You : "<<def_col<<m_msg_to_send;
		fflush(stdout);
	}	
}
