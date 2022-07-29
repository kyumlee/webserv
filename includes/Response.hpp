#ifndef __RESPONSE_HPP__
# define __RESPONSE_HPP__

# include "./header/ResponseHeader.hpp"

class Response : public ResponseHeader
{
	public:
		Response ();
		Response (const Response& response);
		virtual ~Response ();

		Response&		operator= (const Response& response);

		int				checkMethod ();
		std::string		responseErr ();
		int				verifyMethod (int fd, int requestEnd);
//		std::string		responseErr (Response *response);
//		int				verifyMethod (int fd, Response *response, int requestEnd);
		std::string		execGET (std::string& path);
		std::string		execHEAD (std::string& path);
		std::string		execPOST (const std::string& path, const std::string& body);
		std::string		execPUT (const std::string& path, std::string& body);
		std::string		execDELETE (std::string& path);

		std::string		readHtml (const std::string& path);

		void			printResponseValue ();

	private:
		// int	_code;
};

#endif
