#ifndef GENERALHEADER_HPP
# define GENERALHEADER_HPP

# include "../includes/ServerBlock.hpp"

class GeneralHeader
{//요청과 응답 모두에 적용되지만 바디에서 최종적으로 전송되는 데이터와는 관련이 없는 헤더
	public:
		GeneralHeader() {}
		GeneralHeader(const GeneralHeader& gh) { (void)gh; }
		virtual ~GeneralHeader() {}

		GeneralHeader&	operator=(const GeneralHeader& gh) { (void)gh; return (*this); }
		// virtual void abstract() = 0;

		void	setDate(const std::string& date = "")
		{
			if (date.length() != 0)
			{
				this->_date = date;
				return ;
			}
			char	buf[100];
			struct timeval	tv;
			struct tm*		tm;

			gettimeofday(&tv, NULL);
			tm = gmtime(&tv.tv_sec);
			strftime(buf, 100, "%a, %d %b %Y %H:%M:%S GMT", tm);
			std::string	str_date(buf);
			this->_date = str_date;
		}

		void	 setConnection(const std::string& connection = "")
		{//connection이 없다면 default로 keep-alive
			if (connection.length() != 0)
			{
				this->_connection = connection;
				return ;
			}
			this->_connection = "keep-alive";
		}

		void	setTransferEncoding(const std::string& transfer_encoding = "")
		{//transfer_encoding가 없다면 압축이나 수정이 없는 버전으로 초기화
			if (transfer_encoding == "")
				this->_transfer_encoding = "identity";
		}

		std::string	getDate() { return (this->_date); }
		std::string	getConnection() { return (this->_connection); }
		std::string	getTransferEncoding() { return (this->_transfer_encoding); }

		void	printGeneralHeader()
		{
			std::cout << "date: " << this->getDate() << std::endl;
			std::cout << "connection: " << this->getConnection() << std::endl;
			std::cout << "transfer encoding: " << this->getTransferEncoding() << std::endl;
		}

	// protected:
		std::string	_date; //HTTP메시지가 만들어진 시각
		std::string	_connection; //일반적으로 HTTP/1.1을 사용 ex) Connection: keep-alive
		std::string	_transfer_encoding; //사용자에게 entity를 안전하게 전송하기 위해 사용하는 인코딩 형식을 지정
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