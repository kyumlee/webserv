#ifndef __REQUEST_HEADER_HPP__
# define __REQUEST_HEADER_HPP__

# include "./EntityHeader.hpp"

//HTTP요청에서 사용되지만 메시지의 컨텐츠와 관련이 없는 패치될 리소스나 클라리언트 자체에 대한 자세한 정보를 포함하는 헤더
class RequestHeader : public EntityHeader
{
	public:
		RequestHeader ();
		RequestHeader (const RequestHeader& rh);
		virtual ~RequestHeader ();

		RequestHeader&	operator= (const RequestHeader& rh);

		int				checkRequestLine (std::string requestLine);
		int				checkHeader (std::vector<std::string> header);
		int				checkAvailableHeader (const std::string& header, char colon = ':', char space = ' ');
		int				checkEssentialHeader ();

		int				splitRequest (std::string request, int bodyCondition);
		
		void			setHost (const std::string& host = "");
		void			setUserAgent (const std::string& userAgent = "");
		void			setAccept (const std::string& accept = "");
		void			setAcceptCharset (const std::string& charset = "");
		void			setAcceptLanguage (const std::string& lang = "");
		void			setOrigin (const std::string& origin = ""); 
		void			setAuthorization (const std::string& authorization = "");
		void			setMethod (const std::string& method = "");
		void			setPath (const std::string& path = "");
		void			setHttpVersion (const std::string& httpVersion = "");
		void			setBody (const std::string& body = "");
		void			setBodySize (const size_t& bodySize = 0);
		void			setRoot (const std::string& root = "");

		t_listen		getListen () const;
		std::string		getHost () const;
		std::string		getUserAgent () const;
		std::string		getAccept () const;
		std::string		getAcceptCharset () const;
		std::string		getAcceptLanguage () const;
		std::string		getAcceptEncoding () const;
		std::string		getOrigin () const;
		std::string		getAuthorization () const;
		std::string		getMethod () const;
		std::string		getPath () const;
		std::string		getHttpVersion () const;
		std::string		getBody () const;
		size_t			getBodySize () const;
		std::string		getRoot () const;

		void			printRequestHeader ();

	public:
	//일단 임시로 public으로 바꾼다.
		t_listen					_listen;
		std::string					_host; //요청하려는 서버 호스트 이름과 포트 번호
		std::string					_userAgent; //현재 사용자가 어떤 클라리언트(운영체제, 브라우저 등)을 통해 요청을 보냈는지 알 수 있다
		std::string					_accept; //클라이언트가 허용할 수 있는 파일 형식(MIME TYPE)
		std::string					_acceptCharset; //클라이언트가 지원가능한 문자열 인코딩 방식
		std::string					_acceptLanguage; //클라이언트가 지원가능한 언어 나열
		std::string					_acceptEncoding; //클라이언트가 해석가능한 압축 방식 지정
		std::string					_origin; //POST같은 요청을 보낼 때, 요청이 어느 주소에서 시작되었는지를 나타냄, 경로 정보는 포함하지 않고 서버 이름만 포함
		std::string					_authorization; //인증 토큰을 서버로 보낼 때 사용하는 헤더
		//형식 -> Authorization: <auth-scheme> <authorization-parameters>
		//basic authentication -> Authorization: Basic <credentials>

		//내가 만든 것 request line을 파시할 때 사용
		std::string					_method; //request method를 저장
		std::string					_path; //request의 path를 저장
		std::string					_httpVersion; //HTTP버전을 확인
		std::string					_body;
		size_t						_bodySize;
		std::string					_root;
		std::vector<std::string>	_bodyVec;
		std::string					_xHeader;

		/*
		//사용안할 것 같은 것
		std::string	_referer; //현재 페이지로 연결되는 링크가 있던 이전 웹 페이지의 주소
		std::string	_cookie; //Set-Cookie헤더와 함께 서버로부터 이전에 전송됐던 저장된 HTTP 쿠키 포함
		std::string	_if_modified_since; //여기에 쓰여진 시간 이후로 변경된 리소스를 취득하고 캐시가 만료되었을 때에만 데이터를 전송하는데 사용
		*/
};

#endif
