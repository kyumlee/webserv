#ifndef __RESPONSE_HPP__
# define __RESPONSE_HPP__

# include "./Cgi.hpp"

class Response : public ResponseHeader
{
	public:
		Response ();
		Response (const Response& response);
		virtual ~Response ();

		Response&	operator= (const Response& response);

		void		setRemainSend (int value);
		void		setTotalSendSize (size_t size);
		void		setTotalResponse (const std::string& response);
		void		setSendStartPos (const size_t& startPos);

		int			getRemainSend () const;
		size_t		getTotalSendSize () const;
		std::string	getTotalResponse () const;
		size_t		getSendStartPos () const;

		void		initResponseValue () const;

		int			checkAllowedMethods ();

		std::string	responseErr ();

		int			verifyMethod (int fd, int requestEnd, Cgi& cgi);

		std::string	execGET (std::string& path, int fd);
		std::string	execHEAD (std::string& path, int fd);
		std::string	execPOST (const std::string& path, int fd, const std::string& body, Cgi& cgi);
		std::string	execPUT (const std::string& path, int fd, std::string& body);
		std::string	execDELETE (std::string& path, int fd);

		std::string	readHtml (const std::string& path);

		void		printResponseValue ();

	private:
		int			_remainSend;
		size_t		_totalSendSize;
		size_t		_sendStartPos;
		std::string	_totalResponse;

		// int	_code;
};

#endif
