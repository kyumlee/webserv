#ifndef __RESPONSE_HPP__
# define __RESPONSE_HPP__

# include "./ResponseHeader.hpp"

class Response : public ResponseHeader
{
	public:
		Response ();
		Response (const Response& response);
		virtual ~Response ();

		Response&	operator= (const Response& response);

		int			checkAllowedMethods ();

		std::string	responseErr (Response *response);

		int			verifyMethod (int fd, Response *response, int requestEnd);

		std::string	execGET (std::string& path, int fd);
		std::string	execHEAD (std::string& path, int fd);
		std::string	execPOST (const std::string& path, int fd, const std::string& body);
		std::string	execPUT (const std::string& path, int fd, std::string& body);
		std::string	execDELETE (std::string& path, int fd);

		std::string	readHtml (const std::string& path);

		void		printResponseValue ();

	private:
		// int	_code;
};

#endif
