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

		int				checkAllowedMethods ();
		std::string		responseErr (Response *response);
		int				verifyMethod (int fd, Response *response, int requestEnd);
		std::string		getMethod (std::string& path);
		std::string		headMethod (std::string& path);
		std::string		postMethod (const std::string& path, const std::string& body);
		std::string		putMethod (const std::string& path, std::string& body);
		std::string		deleteMethod (std::string& path);

		std::string		readHtml (const std::string& path);

		void			printResponseValue ();

	private:
		// int	_code;
};

#endif
