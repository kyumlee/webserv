#include "Server.hpp"

Server::Server() {}
Server::Server(const Server& server) { (void)server; }
Server::~Server() {}

Server&	Server::operator=(const Server& server) { (void)server; return *this; }

void	Server::change_events(std::vector<struct kevent>& change_list, uintptr_t ident,
			int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata)
{
	struct kevent	temp_event;
	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	change_list.push_back(temp_event);
}

void	Server::disconnect_request(int request_fd)
{
	std::cout << "request disconnected: " << request_fd << std::endl;
	close(request_fd);
	this->_request.erase(request_fd);
	this->_request_end = 0;
	this->_body_condition = No_Body;
	this->_response.resetRequest();
	this->_is_check_request_line = 0;
	this->_body_start_pos = 0;
	this->_body_end = 0;
	this->_body_vec_start_pos = 0;
	this->_body_vec_size = 0;
	this->_rn_pos = 0;
}

void	Server::check_connection(int request_fd)
{
	if (this->_response._connection == "close")
		this->disconnect_request(request_fd);
}

int	Server::init_listen(const std::string& host_port)
{
	if (this->_response.setListen(host_port) == 1)
	{
		std::cerr << "init listen error\n";
		return (1);
	}
	this->_listen.host = this->_response._listen.host;
	this->_listen.port = this->_response._listen.port;
	std::cout << "init listen host : " << this->_listen.host;
	std::cout << ", port : " << this->_listen.port << std::endl;
	return (0);
}

int	Server::init_server_socket()
{//에러가 발생했을 때 에러 메시지를 출력하고 1을 리턴, 정상 작동이면 0을 리턴
//server socket을 만들고, bind, listen, kqueue를 하고,
//바로 change_events를 통해 event를 등록한다.
	int	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (server_socket == -1)
	{
		std::cerr << "init server socket error\n";
		return (1);
	}
	std::memset(&this->_server_addr, 0, sizeof(this->_server_addr));
	this->_server_addr.sin_family = AF_INET;
	this->_server_addr.sin_addr.s_addr = this->_listen.host;
	this->_server_addr.sin_port = this->_listen.port;
	
	if (bind(server_socket, (struct sockaddr*)&this->_server_addr,
		sizeof(this->_server_addr)) == -1)
	{
		std::cerr << "bind socket error\n";
		return (1);
	}
	if (listen(server_socket, LISTEN_BUFFER_SIZE) == -1)
	{
		std::cerr << "listen socket error\n";
		return (1);
	}
	fcntl(server_socket, F_SETFL, O_NONBLOCK);
	this->_server_socket = server_socket;

	int	kq = kqueue();
	if (kq == -1)
	{
		std::cerr << "init kqueue error\n";
		return (1);
	}
	this->_kq = kq;
	this->change_events(this->_change_list, this->_server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	return (0);
}

int	Server::event_error(int fd)
{
	if (fd == this->_server_socket)
	{//server_socket이 에러라면 server를 종료하기 위해 -1을 리턴한다.
		std::cerr << "server start but server socket error\n";
		return (1);
	}
	else
	{//request socket이 에러라면 request의 연결을 끊는다.
		std::cerr << "server start but client socket error\n";
		disconnect_request(fd);
	}
	return (0);
}

void	Server::request_accept()
{
	int	request_socket;
	if ((request_socket = accept(this->_server_socket, NULL, NULL)) == -1)
	{//accept이 실패했을 때
		std::cerr << "server start but request socket accept error\n";
		return ;
	}
	std::cout << "accept new request: " << request_socket << std::endl;
	fcntl(request_socket, F_SETFL, O_NONBLOCK);

	change_events(this->_change_list, request_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	change_events(this->_change_list, request_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	std::cout << "request is readable and writable\n";
	this->_request[request_socket] = "";
}

