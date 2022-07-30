#include "../../includes/Utils.hpp"

#define	BUF_LEN 1024

int	main(int argc, char**argv)
{
	if (argc != 3)
	{
		std::cout << "./a.out server_addr port\n";
		return (1);
	}
	struct sockaddr_in	server_addr;
	int					client_socket;
	std::string			host(argv[1]);
	int					port = atoi(argv[2]);

	if ((client_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cout  << "can not create socket\n";
		return (1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(host.c_str());
	server_addr.sin_port = htons(port);

	if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		std::cout << "can not connect server\n";
		return (1);
	}

	std::string	line;
	std::string	send_msg;
	// std::string	recv_msg;
	char		buf[BUF_LEN] = {};
	while (1)
	{
		while (1)
		{
			std::getline(std::cin, line);
			// std::cout << "line : " << line << std::endl;
			if (line == "end")
			{
				// send_msg += line;
				break ;
			}
			else if (line == "exit")
				exit(0);
			else
			{
				if (line != "\n")
					send_msg += line;
				send_msg += "\r\n";
			}
			line.clear();
		}
		if (send_msg.length() == 0)
		{
			std::cout << "msg has no line please input the line\n";
			continue ;
		}
		write(client_socket, send_msg.c_str(), send_msg.length());
		send_msg.clear();
		while (read(client_socket, buf, BUF_LEN) > 0)
		{
			std::string	tmp(buf);
			size_t		end_pos = 0;
			std::string	end_str;
			std::cout << buf << std::endl;
			std::cout << "recv size : " << tmp.size() << std::endl;
			memset(buf, 0, sizeof(buf));
			if (tmp.size() >= 3)
				end_pos = tmp.size() - 3;
			else
				break ;
			end_str = tmp.substr(end_pos, tmp.size());
			std::cout << "end_str : " << end_str << std::endl;
			if (end_str == "end")
			{
				std::cout << "Read finish\n";
				break ;
			}
		}
	}
	return (0);
}