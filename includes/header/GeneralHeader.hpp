#ifndef __GENERAL_HEADER_HPP__
# define __GENERAL_HEADER_HPP__

# include "../config/ServerBlock.hpp"

class GeneralHeader
{
	//요청과 응답 모두에 적용되지만 바디에서 최종적으로 전송되는 데이터와는 관련이 없는 헤더
	
	public:
		GeneralHeader ();
		GeneralHeader (const GeneralHeader& gh);
		virtual ~GeneralHeader ();

		GeneralHeader&	operator= (const GeneralHeader& gh);
		// virtual void abstract() = 0;

		void			setDate (const std::string& date = "");
		void			setConnection (const std::string& connection = "");
		void			setTransferEncoding (const std::string& transferEncoding = "");

		std::string		getDate () const;
		std::string		getConnection () const;
		std::string		getTransferEncoding () const;

		void			printGeneralHeader () const;

	// protected:
		std::string		_date; //HTTP메시지가 만들어진 시각
		std::string		_connection; //일반적으로 HTTP/1.1을 사용 ex) Connection: keep-alive
		std::string		_transferEncoding; //사용자에게 entity를 안전하게 전송하기 위해 사용하는 인코딩 형식을 지정
		//keep-alive이거나 close인데 default로 keep-alive로 한다.

		/*
		//사용안 할 거 같은 것들
		std::string	_pragma; //캐시제어
		std::string	_cache_control; //캐시를 제어할 때 사용
		std::string	_upgrade; //프로토콜 변경시 사용
		std::string	_via; //중계(프록시)서버의 이름, 버전, 호스트명
		*/
};

#endif
