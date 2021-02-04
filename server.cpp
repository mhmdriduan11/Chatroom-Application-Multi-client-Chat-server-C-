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
#define MAKSIMUM_PENGGUNA 8

using namespace std;

string warna_default = "\033[0m" ;
string daftar_warna[]{"\033[31m","\033[32m","\033[33m","\033[34m","\033[35m","\033[36m"};

struct terminal {
	int id ;
	string name ;
	int socket ;
	thread _thread ;
};

std::vector<terminal> clients ;
mutex cout_mtx, clients_mtx ;
int user_id ;

string color(int code){
	return daftar_warna[code % JUMLAH_WARNA];
}

void set_name(int id, char name[]){
	for (int i = 0; i < (int)clients.size(); ++i)
	{	
		if (clients[i].id == id)
		{
			clients[i].name = string(name);
			break ;
		}
	}
}

void shared_print(string str, bool status_endline=true){
	lock_guard <mutex> guard(cout_mtx);
	cout << str ;
	if (status_endline)
	{
		cout << endl ;
	}
}

void broadcast_message(string pesan, int id_sender){
	char temp[MAKSIMUM_KATA] ;
	strcpy(temp, pesan.c_str());
	for (int i = 0; i < (int)clients.size(); ++i)
	{
		if (clients[i].id != id_sender)
		{
			send(clients[i].socket, temp, sizeof(temp), 0);
		}
	}
}

void broadcast_message(int num, int id_sender){
	for (int i = 0; i < (int)clients.size(); ++i)
	{
		if (clients[i].id != id_sender)
		{
			send(clients[i].socket, &num, sizeof(num), 0);
		}
	}
}

void end_connection(int id){
	for (int i = 0; i < (int)clients.size(); ++i)
	{	
		if (clients[i].id == id)
		{
			lock_guard <mutex> guard(clients_mtx);
			clients[i]._thread.detach();
			clients.erase(clients.begin() + 1);
			close(clients[i].socket);
			break;
		}
	}
}

void handle_client(int socket_service, int id){
	char name[MAKSIMUM_KATA], str[MAKSIMUM_KATA];
	recv(socket_service, name, sizeof(name), 0);
	set_name(id, name);

	string welcome_message = string(name) + string(" has joined") ;
	broadcast_message("#NULL",id);
	broadcast_message(id,id);
	broadcast_message(welcome_message,id);
	shared_print(color(id) + welcome_message + warna_default);

	while(true){
		int bytes_received = recv(socket_service, str, sizeof(str), 0);
		if (bytes_received <= 0)
		{
			return ;
		}

		if (strcmp(str, "#exit") == 0)
		{
			string see_you_again = string(name) + string(" has leaf");
			broadcast_message("#NULL",id);
			broadcast_message(id,id);
			broadcast_message(see_you_again,id);
			shared_print(color(id) + see_you_again + warna_default);
			end_connection(id);
			return ;
		}

		broadcast_message(string(name), id);
		broadcast_message(id,id);
		broadcast_message(string(str), id);
		shared_print(color(id) + name + " : " + warna_default + str);
	}
}

int main(){

	int socket_server = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_server == GAGAL)
	{
		perror("SOCKET: ");
		exit(GAGAL);
	}

	int temp ;
	cout << "Entry your port Server :  " ;
	cin >> temp ;
	struct sockaddr_in alamat_server ;
	alamat_server.sin_family = AF_INET ;
	alamat_server.sin_port = htons(temp) ;
	alamat_server.sin_addr.s_addr = INADDR_ANY ;
	bzero(&alamat_server.sin_zero, 0);

	int enable_server = bind(socket_server, (sockaddr*)&alamat_server, sizeof(alamat_server));
	if (enable_server == GAGAL)
	{
		perror("CONNECTION: ");
		exit(GAGAL);
	}

	//jika sudah hidup maka, tinggal menunggu client terhubung
	int waiting = listen(socket_server, MAKSIMUM_PENGGUNA);
	if (waiting == GAGAL)
	{
		perror("LISTEN: ");
		exit(GAGAL);
	}

	//IF CLIENT IS CONNECTION THIS SERVER, THAT YOU HAVE MAKE A SOCKET FOR CONTAIN ALL CLIENTS
	struct sockaddr_in penampung_client ;
	unsigned int length = sizeof(penampung_client);
	cout << daftar_warna[JUMLAH_WARNA - 1] << "\n\t   ====== Welcome to the chat-room ======  " << endl << warna_default ;

	while(true){
		//lalu terima setiap client yang masuk ke dalam port
		int socket_waiting = accept(socket_server, (sockaddr*)&penampung_client, &length);
		if (socket_waiting == GAGAL)
		{
			perror("SOCKET_CLIENT_ACCEPT: ");
			exit(GAGAL);
		}
		user_id++;
		thread t(handle_client, socket_waiting, user_id);
		lock_guard <mutex> guard(clients_mtx);
		clients.push_back({user_id, string("Anonymous"), socket_waiting, move(t)});
	}

	for (int i = 0; i < (int)clients.size(); ++i)
	{
		if (clients[i]._thread.joinable())
		{
			clients[i]._thread.join();
		}
	}

	close(socket_server);
	cin.get();
	return 0;
}