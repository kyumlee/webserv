# include "./../../includes/header/GeneralHeader.hpp"

GeneralHeader::GeneralHeader() {}
GeneralHeader::GeneralHeader (const GeneralHeader& gh) { (void)gh; }
GeneralHeader::~GeneralHeader() {}

GeneralHeader&	GeneralHeader::operator= (const GeneralHeader& gh) { (void)gh; return (*this); }
// virtual void abstract() = 0;

void			GeneralHeader::setDate (const std::string& date)
{
	if (date.length() != 0)
	{
		this->_date = date;
		return ;
	}
	char			buf[100];
	struct timeval	tv;
	struct tm*		tm;

	gettimeofday(&tv, NULL);
	tm = gmtime(&tv.tv_sec);
	strftime(buf, 100, "%a, %d %b %Y %H:%M:%S GMT", tm);
	std::string	strDate(buf);
	this->_date = strDate;
}

void			GeneralHeader::setConnection (const std::string& connection)
{//connection이 없다면 default로 keep-alive
	if (connection.length() != 0)
	{
		this->_connection = connection;
		return ;
	}
	this->_connection = "keep-alive";
}

void			GeneralHeader::setTransferEncoding (const std::string& transferEncoding)
{//transfer_encoding가 없다면 압축이나 수정이 없는 버전으로 초기화
	if (transferEncoding == "")
		this->_transferEncoding = "identity";
}

std::string		GeneralHeader::getDate () const { return (this->_date); }
std::string		GeneralHeader::getConnection () const { return (this->_connection); }
std::string		GeneralHeader::getTransferEncoding () const { return (this->_transferEncoding); }

void			GeneralHeader::printGeneralHeader() const
{
	std::cout << "date: " << this->getDate() << std::endl;
	std::cout << "connection: " << this->getConnection() << std::endl;
	std::cout << "transfer encoding: " << this->getTransferEncoding() << std::endl;
}
