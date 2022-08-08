#ifndef __RESPONSE_HEADER_HPP__
# define __RESPONSE_HEADER_HPP__

# include "./RequestHeader.hpp"

class ResponseHeader : public RequestHeader
{
	public:
		ResponseHeader();
		ResponseHeader(const ResponseHeader& rh);
		virtual ~ResponseHeader();

		ResponseHeader&				operator=(const ResponseHeader& rh);

		std::string					getServer() const;
		std::string 				getRetryAfter() const;
		std::string					getLastModified() const;
		std::string					getLocation() const;
		std::map<int, std::string>	getErrorMap() const;
		std::map<int, std::string>	getErrorHtml() const;

		std::string					getStatusMessage(int code);

		std::string					getHeader();
		std::string					writeHeader();

		void						setErrorHtml(std::map<int, std::string> html);
		void						changeHtmlRelativePath();
		void						initErrorMap();

		void						resetRequest();

		void						setHeader();
		void						setServer(const std::string& server = "");
		void						setLastModified(const std::string& path);
		void						setLocation(int code, const std::string& path);

		void						printResponseHeader();
	
	protected:
		std::string					_server;
		std::string					_wwwAuthenticate;
		std::string					_retryAfter;
		std::string					_lastModified;
		std::string					_location;
		
		std::map<int, std::string>	_errorMap;
		std::map<int, std::string>	_errorHtml;
};

#endif
