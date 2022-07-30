#include <fcntl.h>
#include <iostream>
#include <string>
#include <cstdio>

void	input_header(FILE* file, std::string header)
{
	char	header_value[100];
	std::string	str_header_value(header);
	std::cout << "input " << header << ": ";
	fscanf(stdin, "%s", header_value);
	str_header_value += header_value;
	str_header_value += "\r\n";
	fprintf(file, "%s", str_header_value.c_str());
}

void	input_body(FILE* file)
{
	std::string	line;
	std::string	str_body = "stdheader\r\n\r\n";
	std::cout << "input body=================\n";
	while (1)
	{
		std::getline(std::cin, line);
		if (line == "exit")
		{
			break ;
		}
		else
		{
			if (str_body != "stdheader\r\n\r\n")
				str_body += "\n";
			str_body += line;
		}
		line.clear();
	}
	if (str_body != "stdheader\r\n\r\n")
		str_body += "\r\n";
	std::cout << "body length : " << str_body.length() << ", body size : " << str_body.size() << std::endl;
	fprintf(file, "%s", str_body.c_str());
	std::cout << "===========body=========\n" << str_body << std::endl;
}

void	input_request_line(FILE* file)
{
	char	method[100];
	char	path[100];
	char	http_version[100];
	std::string	str_request_line = "";
	std::cout << "input request line: ";
	fscanf(stdin, "%s %s %s", method, path, http_version);
	str_request_line += method;
	str_request_line += " ";
	str_request_line += path;
	str_request_line += " ";
	str_request_line += http_version;
	str_request_line += "\r\n";
	fprintf(file, "%s", str_request_line.c_str());
}

/*
int	main()
{
	FILE*	file = fopen("testbody", "w");
	input_body(file);
	fclose(file);
}*/


int	main(int argc, char** argv)
{//인자를 받아서 만들어주자
	if (argc != 2)
	{
		std::cout << "please input file_name";
		return (1);
	}
	FILE*	file = fopen(argv[1], "w");
	if (file == NULL)
	{
		std::cout << "file open fail\n";
		return (1);
	}
	std::cout << "please input method\n";
	input_request_line(file);
	input_header(file, "Host: ");
	input_header(file, "User-Agent: ");
	input_header(file, "Accept: ");
	input_header(file, "Accept-Charset: ");
	input_header(file, "Accept-Language: ");
	input_header(file, "Accept-Encoding: ");
	input_header(file, "Origin: ");
	input_header(file, "Authorization: ");
	input_header(file, "Transfer-Encoding: ");
	input_header(file, "Content-Length: ");
	input_header(file, "Content-Type: ");
	input_header(file, "Content-Language: ");
	input_header(file, "Content-Location: ");
	input_header(file, "Content-Encoding: ");
	// input_body(file);
	fclose(file);
	return (1);
}