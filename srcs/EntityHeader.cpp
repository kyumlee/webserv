#include "./../includes/EntityHeader.hpp"

EntityHeader::EntityHeader () {}
EntityHeader::EntityHeader (const EntityHeader& eh) { (void)eh; }
EntityHeader::~EntityHeader() {}

EntityHeader&			EntityHeader::operator= (const EntityHeader& eh) { (void)eh; return (*this); }

void					EntityHeader::setContentLength (const size_t& size)
{ _contentLength = intToStr(size); }

void					EntityHeader::setContentLength (const std::string& path, const std::string& contentLength)
{
	if (contentLength != "")
		_contentLength = contentLength;
	else if (path != "")
	{
		struct stat	fileStat;

		if (!stat(path.c_str(), &fileStat))
			_contentLength = intToStr(fileStat.st_size);
		else
		{
			printErr("failed to get size");
			setCode(Internal_Server_Error);
		}
	}
}

//html, css, js, jpeg, png, bmp, plain만 정해놨는데 다른 것도 해야되는지 알아봐야 한다.
void					EntityHeader::setContentTypeLocation (const std::string& path, std::string type, std::string contentLocation)
{
	if (type != "")
	{
		_contentType = type;
		_contentLocation = path;
		return ;
	}
	std::string	fileName = findFileName(path);
	std::string	fileType = findFileType(fileName);
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
	//fileType이 이상할 떄
	else
	{
		fileType = "";
		contentType = "text/plain";
	}
	_contentType = contentType;

	std::string	checkContentLocation = path.substr(0, path.length() - fileName.length());
	
	if (contentLocation != "")
		_contentLocation = contentLocation;
	else if (fileType == "")
		_contentLocation = checkContentLocation + pureFileName;
	else
		_contentLocation = path;

	if (_contentLocation.at(0) != '/')
		_contentLocation.insert(0, "/");
}
void					EntityHeader::setContentLanguage (const std::string& contentLanguage)
{ _contentLanguage = contentLanguage;}

void					EntityHeader::setContentEncoding (const std::string& contentEncoding)
{ _contentEncoding = contentEncoding; }

//둘 다 쓰는지 확인해보자
void					EntityHeader::setAllow (const std::string& allow)
{ _allow = allow; }
void					EntityHeader::setAllow (const std::set<std::string>& methods)
{
	std::set<std::string>::iterator	it = methods.begin();
	while (it != methods.end())
	{
		_allow = *it;
		_allow += ", ";
		it++;
	}
}

void					EntityHeader::setCode (const int& code)
{
	if (code != 0)
		_code = code;
	else
		_code = 0;
}

void					EntityHeader::initPossibleMethods ()
{
	_possibleMethods.insert("GET");
	_possibleMethods.insert("POST");
	_possibleMethods.insert("PUT");
	_possibleMethods.insert("DELETE");
	_possibleMethods.insert("HEAD");
}

void					EntityHeader::setAllowedMethod (const std::string& method)
{ _allowedMethods.insert(method); }

void					EntityHeader::initAllowedMethods (std::vector<std::string> allowedMethods)
{
	for (std::vector<std::string>::iterator it = allowedMethods.begin(); it != allowedMethods.end(); it++)
		setAllowedMethod(*it);
}

std::string				EntityHeader::getContentLength () const { return (_contentLength); }
std::string				EntityHeader::getContentType () const { return (_contentType); }
std::string				EntityHeader::getContentLanguage () const { return (_contentLanguage); }
std::string				EntityHeader::getContentLocation () const { return (_contentLocation); }
std::string				EntityHeader::getContentEncoding () const { return (_contentEncoding); }
std::string				EntityHeader::getAllow () const { return (_allow); }
int						EntityHeader::getCode () const { return (_code); }
std::set<std::string>	EntityHeader::getAllowedMethods () const { return (_allowedMethods); }
std::set<std::string>	EntityHeader::getPossibleMethods () const { return (_possibleMethods); }

void					EntityHeader::printEntityHeader () const
{
	std::cout << "content length: " << getContentLength() << std::endl;
	std::cout << "content type: " << getContentType() << std::endl;
	std::cout << "content language: " << getContentLanguage() << std::endl;
	std::cout << "content location: " << getContentLocation() << std::endl;
	std::cout << "content encoding: " << getContentEncoding() << std::endl;
	std::cout << "allow: " << getAllow() << std::endl;
	std::cout << "code: " << getCode() << std::endl;
	std::cout << "allow method: ";
	printSet(getAllowedMethods());
	std::cout << "possible method: ";
	printSet(getPossibleMethods());
}
