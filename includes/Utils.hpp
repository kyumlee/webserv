#ifndef __UTILS_HPP__
# define __UTILS_HPP__

# include <iostream>
# include <vector>
# include <map>
# include <fstream>
# include <sstream>
# include <string>
# include <sys/types.h> //st_mtime
# include <sys/event.h> //kevent
# include <sys/time.h> //timeval
# include <sys/socket.h>
# include <sys/stat.h> // struct stat
# include <arpa/inet.h> //sockaddr_in
# include <unistd.h>
# include <fcntl.h>
# include <dirent.h> //DIR
# include <set>

# include <limits> //numeric_limits

# include <cstring> //memset
# include <cctype> //isalpha
# include <cmath>

# define DEFAULT_CONF		"./../conf/mac2.conf"
# define NONE				0
# define EXACT				1
# define PREFERENTIAL		2
# define DEFAULT_AUTOINDEX	2
# define ON					1
# define OFF				0

# define CGI_BUFFER_SIZE 	50000
# define LISTEN_BUFFER_SIZE 1024
# define READ_BUFFER_SIZE	100000

# define DEFAULT_RETRY_AFTER	10

# define ERROR_HTML "<!DOCTYPE html>\n\
	<html lang=\"en\">\n\
	\t<meta charset=\"utf-8\">\n\
	\t<title>40404</title>\n\
	<body>\n\
	\t<h1>NO ERROR PAGE</h1>\n\
	\t<p>configuration file has no error page that fits error code</p>\n\
	</body>\n\
	</html>\n"

# define BAD_REQUEST_HTML			"./files/html/Bad_Request.html"
# define FORBIDDEN_HTML				"./files/html/Forbidden.html"
# define NOT_ALLOWED_HTML			"./files/html/Not_Allowed.html"
# define NOT_FOUND_HTML				"./files/html/Not_Found.html"
# define PAYLOAD_TOO_LARGE_HTML		"./files/html/Payload_Too_Large.html"
# define INTERNAL_SERVER_ERROR_HTML	"./files/html/Internal_Server_Error.html"
# define DEFAULT_HTML				"./files/html/index.html"

# define RED	"\e[31m"
# define GREEN	"\e[32m"
# define YELLOW	"\e[33m"
# define BLUE	"\e[34m"
# define PINK	"\e[35m"
# define CYAN	"\e[36m"
# define WHITE	"\e[37m"
# define RESET	"\e[0m"

# define DEFAULT_PORT		8000
# define DEFAULT_PORT_STR	"8000"

enum	ErrorCode
{
	Continue =		100, //지금까지 상태가 괜찮다는 임시적인 응답
	OK =			200, //요청이 성공적으로 됨
	Created =		201, //요청이 성공하여 새로운 리소스 생성
	No_Content =	204, //요청에 대해서 보내줄 콘텐츠가 없다

	Bad_Request =			400, //요청이 잘못된 문법으로 이루어져 있을 때
	Forbidden =				403, //서버가 클라이언트가 누군지 알고 있고, 클라이언트가 권리가 없을 때
	Not_Found =				404, //요청받은 리소스를 찾지 못했을 때
	Method_Not_Allowed =	405, //요청한 메소드를 서버에서 알고는 있지만 금지했을 때
	//GET과 HEAD는 필수 메소드이기 때문에 제거하면 안된다.
	Payload_Too_Large =		413, //요청 엔티티가 서버에서 정의한 한계보다 클 때
	//서버는 연결을 끊거나 Retry-After헤더필드로 돌려 보냄
	Internal_Server_Error =	500, //서버가 처리 방법을 모를 때

	//사용할지 말지 모르는 것들
	Moved_Permanently =					301, //요청한 리소스의 URI가 변경되었음을 의미
	Found =								302, //요청한 리소스의 URI가 일시적으로 변경되었음을 의미
	See_Other =							303, //요청한 리소스를 다른 URI에서 GET요청을 통해 얻어야 할 때
	Unauthorized =						401, //서버가 클라이언트가 누군지 모르고, 클라이언트가 권리가 없을 때
	Request_Timeout =					408, //요청을 한지 시간이 오래된 연결에 전송
	Conflict =							409, //요청이 현재 서버의 상태와 충돌할 때
	Length_Required =					411, //요청이 Content-Length헤더 필드가 정의되지 않을 때
	Precondition_Failed =				412, //클라이언트의 헤더에 있는 전제조건이 서버의 전제조건에 적절하지 않을 때
	URI_Too_Long =						414, //클라이언트가 요청한 URI가 서버에서 처리하지 않기로 한 길이보다 길 때
	Unsupported_Media_Type =			415, //요청한 미디어 포맷을 서버에서 지원하지 않을 때
	Requested_Range_Not_Satisfiable =	416, //Range 헤더 필드에 요청한 지정 범위를 만족시킬 수 없을 때
	Too_Many_Requests =					429, //user가 주어진 시간 내에서 너무 많은 request를 보냈을 때
	Request_Header_Fields_Too_Large =	431, //요청한 헤더 필드가 너무 클 때
	Bad_Gateway =						502, //서버가 요청을 처리하는 데 필요한 응답을 얻기 위해 게이트웨이로 작업하는 동안 잘못된 응답을 수신했을 때
	Service_Unavailable =				503, //서버가 요청을 처리할 준비가 되지 않았을 때
	HTTP_Version_Not_Supported =		505 //요청에 사용된 HTTP 버전을 서버에서 지원하지 않을 때
};

enum	BodyExist
{
	No_Body =		0,
	Body_Exist =	1,
	Body_Start =	2,
	Body_End =		3
};

enum	Connection
{
	Keep_Alive =	0,
	Close =			1
};

typedef struct s_listen
{
	unsigned int			host;
	int						port;
}							t_listen;

int							printErr (std::string errMsg);
std::vector<std::string>	split (std::string str, char delimiter = '\n');
std::string					trim (std::string str);
std::vector<std::string>	splitBlocks (std::string block, std::string type);
std::pair<bool, size_t>		skipKey (std::string line, std::string str, std::string delimiter);
bool						isNumber (std::string str);
std::string					parseValue (std::string line, size_t pos, std::string delimiter);
int							strToInt (std::string str);
int							compareURIs (std::string URI, std::string request, int mod);

int							pathIsFile (const std::string& path);

std::string					setUri (const std::string& dirList, const std::string& dirName, const std::string& host, const int port);
std::string					setHtml (const std::string& path, const std::string& lang, const std::string& charset, const std::string& h1, const std::string& host, const int port);

int							setErrorPages (const std::string& errPages, std::map<int, std::string>* errMap);

std::string					intToStr (int code);
std::string					sizetToStr (size_t code);

void						printVec (std::vector<std::string> strVec);
void						printSet (std::set<std::string> strSet);
void						printErrmap (std::map<int, std::string> errmap);

int							compareEnd (const std::string& s1, const std::string& s2);

std::string					findExtension (std::string& file);
std::string					findFileName (const std::string& path);
std::string					findFileType (const std::string& file);
std::string					eraseFileType (const std::string& file);

std::string					strTrimChar (const std::string& str, char deleteChar = ' ');
std::string					findHeaderValue (const std::string& header);
std::string					strDeleteRN (const std::string& str);

int							isStrAlpha (const std::string& str);
int							isStrUpper (const std::string& str);

int							makeHtml (const std::string& htmlName, int code, const std::string& codeStr, const std::string& serverName);

size_t						calExponent (const std::string& str);

int							checkHex (std::string& hex);
size_t						hexToDecimal (std::string& hex);

void						printStr (const std::string& str, const std::string& response);

#endif
