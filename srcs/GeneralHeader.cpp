# include "./../includes/GeneralHeader.hpp"

GeneralHeader::GeneralHeader () {}
GeneralHeader::GeneralHeader (const GeneralHeader& gh) { (void)gh; }
GeneralHeader::~GeneralHeader () {}

GeneralHeader&	GeneralHeader::operator= (const GeneralHeader& gh) { (void)gh; return (*this); }
// virtual void abstract() = 0;

void			GeneralHeader::setDate (const std::string& date)
{
	if (date.length() != 0)
	{
		_date = date;
		return ;
	}
	char			buf[100];
	struct timeval	tv;
	struct tm*		tm;

	gettimeofday(&tv, NULL);
	tm = gmtime(&tv.tv_sec);
	strftime(buf, 100, "%a, %d %b %Y %H:%M:%S GMT", tm);

	std::string	strDate(buf);
	_date = strDate;
}

//connection이 없다면 default로 keep-alive
void	 		GeneralHeader::setConnection (const std::string& connection)
{
	if (connection.length() != 0)
	{
		_connection = connection;
		return ;
	}
	_connection = "keep-alive";
}

//transferEncoding가 없다면 압축이나 수정이 없는 버전으로 초기화
void			GeneralHeader::setTransferEncoding (const std::string& transferEncoding)
{
	if (transferEncoding == "")
		_transferEncoding = "identity";
}

std::string		GeneralHeader::getDate () const { return (_date); }
std::string		GeneralHeader::getConnection () const { return (_connection); }
std::string		GeneralHeader::getTransferEncoding () const { return (_transferEncoding); }

void			GeneralHeader::printGeneralHeader () const
{
	std::cout << "date: " << getDate() << std::endl;
	std::cout << "connection: " << getConnection() << std::endl;
	std::cout << "transfer encoding: " << getTransferEncoding() << std::endl;
}
