#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <thread>
#include <mutex>

#define GAGAL -1
#define JUMLAH_WARNA 6
#define MAKSIMUM_KATA 200

using namespace std ;
string warna_default = "\033[0m" ;
string daftar_warna[]{"\033[31m","\033[32m","\033[33m","\033[34m","\033[35m","\033[36m"} ;
thread t_send, t_recv ;
bool status_socket = false ;
int socket_server ; //server

string color(int code){
	return daftar_warna[code % JUMLAH_WARNA];
}

void ctrl_c(int signal){
	char penampung[MAKSIMUM_KATA] = "#exit" ;
	send(socket_server, penampung, sizeof(penampung), 0);
	status_socket = true ;
	t_send.detach();
	t_recv.detach();
	close(socket_server);
	exit(signal);
}

void hapus_text(int in){
	char temp = 8 ;
	for (int i = 0; i < in; ++i)
	{
		cout << temp ;
	}
}

void kirim_pesan(int socket_server){
	while(true){
		char penampung[MAKSIMUM_KATA];
		cout << daftar_warna[1] << "You: " ;
		cin.getline(penampung,MAKSIMUM_KATA);
		send(socket_server, penampung, sizeof(penampung), 0);

		if (strcmp(penampung, "#exit") == 0)
		{
			status_socket = true ;
			t_recv.detach();
			close(socket_server);
			return ;
		}
	}
}

void terima_pesan(int socket_server){
	while(true){
		if (status_socket)
		{
			return ;
		}
		char name[MAKSIMUM_KATA],str[MAKSIMUM_KATA];
		int code_color ;

		int status_receiver = recv(socket_server, name, sizeof(name), 0);
		if (status_receiver <= 0)
		{
			continue ;
		}
		recv(socket_server, &code_color, sizeof(code_color), 0);
		recv(socket_server, str, sizeof(str), 0);
		hapus_text(6);

		if (strcmp(name, "#NULL") != 0)
		{
			cout << color(code_color) << name << " : " << warna_default << str << endl ;
		}else{
			cout << color(code_color) << str << endl ;
		}

		cout << daftar_warna[1] << "You: " << warna_default ;
		fflush(stdout) ;
	}
}
int main(){

	socket_server = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_server == GAGAL)
	{
		perror("SOCKET: ");
		exit(GAGAL);
	}

	int temp ;
	cout << "Entry your port Server :  " ;
	cin >> temp ;
	cin.ignore(1,'\n');

	struct sockaddr_in alamat_socket ;
	alamat_socket.sin_family = AF_INET ;
	alamat_socket.sin_port = htons(temp);
	alamat_socket.sin_addr.s_addr = INADDR_ANY ;
	bzero(&alamat_socket.sin_zero,0);

	//sambungkan ke alamat
	int connection = connect(socket_server, (sockaddr*)&alamat_socket, sizeof(alamat_socket));
	if (connection == GAGAL)
	{
		perror("CONNECTION: ");
		exit(GAGAL);
	}

	//Jika sudah terhubung, lalu cek signal
	signal(SIGINT, ctrl_c);
	char penampung[MAKSIMUM_KATA];
	cout << "Entry your name: " ;
	cin.getline(penampung, MAKSIMUM_KATA);
	send(socket_server, penampung, sizeof(penampung), 0);

	cout << daftar_warna[1] << "\n\t   ====== Welcome to the Chat-Room ======   " << endl << warna_default ;

	thread s1(kirim_pesan, socket_server);
	thread s2(terima_pesan, socket_server);

	t_send = move(s1);
	t_recv = move(s2);

	if (t_send.joinable())
	{
		t_send.join();
	}
	if (t_recv.joinable())
	{
		t_recv.join();
	}

	cin.get();
	return 0 ;
}