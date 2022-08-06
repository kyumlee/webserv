#ifndef __REQUEST_HEADER_HPP__
# define __REQUEST_HEADER_HPP__

# include "./EntityHeader.hpp"

class RequestHeader : public EntityHeader
{
	public:
		RequestHeader();
		RequestHeader(const RequestHeader& rh);
		virtual ~RequestHeader();

		RequestHeader&				operator=(const RequestHeader& rh);

		int							checkRequestLine(std::string requestLine);
		int							checkHeader(std::vector<std::string> header);
		int							checkAvailableHeader(const std::string& header, char colon = ':', char space = ' ');
		int							checkEssentialHeader();

		int							splitRequest(std::string request, int bodyCondition);

		t_listen					getListen() const;
		std::string					getHost() const;
		std::string					getUserAgent() const;
		std::string					getAccept() const;
		std::string					getAcceptCharset() const;
		std::string					getAcceptLanguage() const;
		std::string					getAcceptEncoding() const;
		std::string					getOrigin() const;
		std::string					getAuthorization() const;
		std::string					getMethod() const;
		std::string					getPath() const;
		std::string					getHttpVersion() const;
		std::string					getBody() const;
		size_t						getBodySize() const;
		std::string					getRoot() const;
		std::string					getXHeader() const;
		std::vector<std::string>	getBodyVec();
		
		void						setHost(const std::string& host = "");
		void						setUserAgent(const std::string& userAgent = "");
		void						setAccept(const std::string& accept = "");
		void						setAcceptCharset(const std::string& charset = "");
		void						setAcceptLanguage(const std::string& lang = "");
		void			setOrigin(const std::string& origin = ""); 
		void						setAuthorization(const std::string& authorization = "");
		void						setMethod(const std::string& method = "");
		void						setPath(const std::string& path = "");
		void						setHttpVersion(const std::string& httpVersion = "");
		void						setBody(const std::string& body = "");
		void						setBodySize(const size_t& bodySize = 0);
		void						setRoot(const std::string& root = "");

		void						addBodyVec(const std::string& body);
		void						addBody(const std::string& body);
		void						resetBodyVec();

		void						printRequestHeader();

	protected:
		t_listen					_listen;
		std::string					_host;
		std::string					_userAgent;
		std::string					_accept;
		std::string					_acceptCharset;
		std::string					_acceptLanguage;
		std::string					_acceptEncoding;
		std::string					_origin;
		std::string					_authorization;

		std::string					_method;
		std::string					_path;
		std::string					_httpVersion;
		std::string					_body;
		size_t						_bodySize;
		std::string					_root;
		std::vector<std::string>	_bodyVec;
		std::string					_xHeader;
};

#endif
