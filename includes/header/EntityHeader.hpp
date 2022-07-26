#ifndef __ENTITY_HEADER_HPP__
# define __ENTITY_HEADER_HPP__

# include "GeneralHeader.hpp"

class EntityHeader : public GeneralHeader
{
	//컨텐츠 길이나 MIME 타입과 같이 entity 바디에 대한 자세한 정보를 포함하는 헤더

	public:
		EntityHeader ();
		EntityHeader (const EntityHeader& eh);
		virtual ~EntityHeader ();

		EntityHeader&			operator= (const EntityHeader& eh);

		void					setContentLength (const size_t& size = 0);

		void					setContentLength (const std::string& path = "", const std::string& contentLength = "");

		void					setContentTypeLocation (const std::string& path, std::string type = "", std::string contentLocation = "");
		void					setContentLanguage (const std::string& contentLanguage = "en");
		void					setContentEncoding (const std::string& contentEncoding = "");
		//둘 다 쓰는지 확인해보자
		void					setAllow (const std::string& allow);
		void					setAllow (const std::set<std::string>& methods);
		void					setCode (const int& code = 0);

		void					initPossibleMethods();
		void					setAllowedMethods (const std::string& method);
		void					initAllowedMethods (std::vector<std::string> allow_method);

		std::string				getContentLength () const;
		std::string				getContentType () const;
		std::string				getContentLanguage () const;
		std::string				getContentLocation () const;
		std::string				getContentEncoding () const;
		std::string				getAllow () const;
		int						getCode () const;
		std::set<std::string>	getAllowedMethods () const;
		std::set<std::string>	getPossibleMethods () const;

		void					printEntityHeader();

	// protected:
		std::string				_contentLength; //메시지의 크기
		std::string				_contentType; //컨텐츠의 타입(MIME)과 문자열 인코딩(utf-8등)을 명시
		//ex) Content-Type: text/html; charset=utf-8
		std::string				_contentLanguage; //사용자의 언어
		std::string				_contentLocation; //반환된 데이터 개체의 실제 위치
		std::string				_contentEncoding; //컨텐츠가 압축된 방식
		std::string				_allow; //지원되는 HTTP 메소드, 만약 비어있다면 모든 메소드가 금지라는 뜻
		
		int						_code;
		std::set<std::string>	_allowedMethods; //configuration file에서 얻어온다.
		std::set<std::string>	_possibleMethods;
		/*
		//사용안할 것 같은 것
		std::string	_content_disposition; //응답 본문을 브라우저가 어떻게 표시해야할 지 알려주는 헤더
		std::string	_content_security_policy; //다른 외부 파일들을 불러오는 경우, 차단할 소스와 불러올 소스를 명시
		std::string	_expires; //자원의 만료 일자
		std::string	_etag; //리소스의 버전을 식별하는 고유한 문자열 검사기(주로 캐시 확인용으로 사용)
		*/
};

#endif
