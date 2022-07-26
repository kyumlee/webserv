#include "./../../includes/header/EntityHeader.hpp"

EntityHeader::EntityHeader () {}
EntityHeader::EntityHeader (const EntityHeader& eh) { (void)eh; }
EntityHeader::~EntityHeader () {}

EntityHeader&			EntityHeader::operator= (const EntityHeader& eh) { (void)eh; return (*this); }

void					EntityHeader::setContentLength (const size_t& size)
{ this->_contentLength = intToStr(size); }

void					EntityHeader::setContentLength (const std::string& path, const std::string& contentLength)
{
	if (contentLength != "")
		this->_contentLength = contentLength;
	else if (path != "")
	{
		struct stat	fileStat;
		if (!stat(path.c_str(), &fileStat))
			this->_contentLength = intToStr(fileStat.st_size);
		else
		{
			printErr("file size");
			this->setCode(Internal_Server_Error);
		}
	}
}

void					EntityHeader::setContentTypeLocation (const std::string& path, std::string type, std::string contentLocation)
{

	if (type != "")
	{
		this->_contentType = type;
		this->_contentLocation = path;
		return ;
	}

	std::string	fileName = getFileName(path);
	std::string	fileType = getFileType(fileName);
	std::string	pureFileName = eraseFileType(fileName);
	std::string	contentType;

	if (fileType == "css")
		contentType = "text/css";
	else if (fileType == "html")
		contentType = "text/html";
	else if (fileType == "js")
		contentType = "text/javascript";
	else if (fileType == "apng")
		contentType = "image/apng";
	else if (fileType == "avif")
		contentType = "image/avif";
	else if (fileType == "gif")
		contentType = "image/gif";
	else if (fileType == "jpeg" || fileType == "jpg")
		contentType = "image/jpeg";
	else if (fileType == "png")
		contentType = "image/png";
	else if (fileType == "svg")
		contentType = "image/svg+xml";
	else if (fileType == "webp")
		contentType = "image/webp";
	//bmp는 사용을 안 하는 것이 좋다고 한다.
	// else if (fileType == "bmp")
	// 	fileType = "image/bmp";
	//비디오, 음악 파일은 일단 제외했다.
	else
	{//fileType이 이상할 떄
		fileType = "";
		contentType = "text/plain";
	}
	this->_contentType = contentType;

	std::string	checkContentLocation = path.substr(0, path.length() - fileName.length());
	
	if (contentLocation != "")
		this->_contentLocation = contentLocation;
	else if (fileType == "")
		this->_contentLocation = checkContentLocation + pureFileName;
	else
		this->_contentLocation = path;
	if (this->_contentLocation.at(0) != '/')
		this->_contentLocation.insert(0, "/");
}

void					EntityHeader::setContentLanguage (const std::string& contentLanguage)
{ this->_contentLanguage = contentLanguage;}

void					EntityHeader::setContentEncoding (const std::string& contentEncoding)
{ this->_contentEncoding = contentEncoding; }

//둘 다 쓰는지 확인해보자
void					EntityHeader::setAllow (const std::string& allow)
{ this->_allow = allow; }
void					EntityHeader::setAllow (const std::set<std::string>& methods)
{
	for (std::set<std::string>::iterator it = methods.begin(); it != methods.end(); it++)
	{
		this->_allow = *it;
		this->_allow += ", ";
	}
/*	while (it != methods.end())
	{
		this->_allow = *it;
		this->_allow += ", ";
		it++;
	}*/
}

void					EntityHeader::setCode (const int& code)
{ this->_code = code; }
/*{
	if (code != 0)
		this->_code = code;
	else
		this->_code = 0;
}*/

void					EntityHeader::initPossibleMethods ()
{
	this->_possibleMethods.insert("GET");
	this->_possibleMethods.insert("POST");
	this->_possibleMethods.insert("PUT");
	this->_possibleMethods.insert("DELETE");
	this->_possibleMethods.insert("HEAD");
}

void					EntityHeader::setAllowedMethods (const std::string& method)
{ this->_allowedMethods.insert(method); }

void					EntityHeader::initAllowedMethods (std::vector<std::string> allowedMethods)
{
	for (std::vector<std::string>::iterator it = allowedMethods.begin(); it != allowedMethods.end(); it++)
		this->setAllowedMethods(*it);
}

std::string				EntityHeader::getContentLength () const { return (this->_contentLength); }
std::string				EntityHeader::getContentType () const { return (this->_contentType); }
std::string				EntityHeader::getContentLanguage () const { return (this->_contentLanguage); }
std::string				EntityHeader::getContentLocation () const { return (this->_contentLocation); }
std::string				EntityHeader::getContentEncoding () const { return (this->_contentEncoding); }
std::string				EntityHeader::getAllow () const { return (this->_allow); }
int						EntityHeader::getCode () const { return (this->_code); }
std::set<std::string>	EntityHeader::getAllowedMethods () const { return (this->_allowedMethods); }
std::set<std::string>	EntityHeader::getPossibleMethods () const { return (this->_possibleMethods); }

void					EntityHeader::printEntityHeader ()
{
	std::cout << "content length: " << this->getContentLength() << std::endl;
	std::cout << "content type: " << this->getContentType() << std::endl;
	std::cout << "content language: " << this->getContentLanguage() << std::endl;
	std::cout << "content location: " << this->getContentLocation() << std::endl;
	std::cout << "content encoding: " << this->getContentEncoding() << std::endl;
	std::cout << "allow: " << this->getAllow() << std::endl;
	std::cout << "code: " << this->getCode() << std::endl;
	std::cout << "allow method: ";
	printSet(this->getAllowedMethods());
	std::cout << "possible method: ";
	printSet(this->getPossibleMethods());
}
