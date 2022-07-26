#ifndef __RESPONSE_HEADER_HPP__
# define __RESPONSE_HEADER_HPP__

# include "./RequestHeader.hpp"

class ResponseHeader : public RequestHeader
{
	public:
		ResponseHeader ();
		ResponseHeader (const ResponseHeader& rh);
		virtual ~ResponseHeader ();

		ResponseHeader&				operator= (const ResponseHeader& rh);

		void						initErrorHtml ();
		void						setErrorHtml (int code, std::string html);
		void						setErrorHtml (std::map<int, std::string> html);

		void						changeHtmlRelativePath ();
	
		void						initErrorMap ();

		std::string					getHeader ();
		std::string					writeHeader ();
		std::string					getStatusMessage (int code);
		void						initRequest ();
		void						resetRequest ();

		void						setHeader ();
		void						setServer (const std::string& server = "");
		void						setWwwAuthenticate (int code);
		void						setRetryAfter (int code, int sec);
		void						setLastModified (const std::string& path);
		void						setLocation (int code, const std::string& path);

		std::string					getServer () const;
		std::string 				getRetryAfter () const;
		std::string					getLastModified () const;
		std::string					getLocation () const;
		std::map<int, std::string>	getErrorMap () const;
		std::map<int, std::string>	getErrorHtml () const;

		void						printResponseHeader ();
	
	// protected:
		std::string					_server; //웹서버 정보
		std::string					_wwwAuthenticate; //사용자 인증이 필요한 자원을 요구할 시, 서버가 제공하는 인증 방식
		std::string					_retryAfter; //다시 접속하라고 알릴 때
		std::string					_lastModified; //리소스의 마지막 수정 날짜
		std::string					_location; //300번대 응답이나 201 (created)응답일 때 어느 페이지로 이동할 지 알려주는 헤더
		
		std::map<int, std::string>	_errorMap;
		std::map<int, std::string>	_errorHtml;

		//사용안할 것 같은 것
		// std::string	_access_control_allow_origin; //요청 Host와 응답 Host가 다를 때 CORS에러를 막기 위해 사용
		// std::string	_age; //시간이 얼마나 흘렀는지 초 단위로 알려줌
		// std::string	_referrer_policy; //서버 referrer정책을 알려줌
		// std::string	_proxy_authenticate; //요청한 서버가 프록시 서버인 경우 유저 인증을 위한 값
		// std::string	_set_cookie; //서버측에서 클라이언트에게 세션 쿠키 정보를 설정
};

#endif
