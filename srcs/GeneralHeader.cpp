# include "./../includes/GeneralHeader.hpp"

GeneralHeader::GeneralHeader()
	: _date(""),
	_connection(""),
	_transferEncoding("")
{}

GeneralHeader::GeneralHeader(const GeneralHeader& gh)
	: _date(gh._date),
	_connection(gh._connection),
	_transferEncoding(gh._transferEncoding)
{}
GeneralHeader::~GeneralHeader() {}

GeneralHeader&	GeneralHeader::operator=(const GeneralHeader& gh)
{
	_date = gh._date;
	_connection = gh._connection;
	_transferEncoding = gh._transferEncoding;
	return (*this);
}

std::string		GeneralHeader::getDate() const { return (_date); }
std::string		GeneralHeader::getConnection() const { return (_connection); }
std::string		GeneralHeader::getTransferEncoding() const { return (_transferEncoding); }

void			GeneralHeader::setDate(const std::string& date)
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

void	 		GeneralHeader::setConnection(const std::string& connection)
{
	if (connection.length() != 0)
	{
		_connection = connection;
		return ;
	}
	_connection = "keep-alive";
}

void			GeneralHeader::setTransferEncoding(const std::string& transferEncoding)
{
	if (transferEncoding == "")
		_transferEncoding = "identity";
}

void			GeneralHeader::printGeneralHeader() const
{
	std::cout << "date: " << getDate() << std::endl;
	std::cout << "connection: " << getConnection() << std::endl;
	std::cout << "transfer encoding: " << getTransferEncoding() << std::endl;
}
