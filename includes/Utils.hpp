#ifndef __UTILS_HPP__
# define __UTILS_HPP__

# include <iostream>
# include <vector>
# include <map>
# include <fstream>
# include <sstream>
# include <string>
# include <sys/types.h>
# include <sys/event.h>
# include <sys/time.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <fcntl.h>
# include <dirent.h>
# include <set>

# include <limits>

# include <cstring>
# include <cctype>
# include <cmath>

# define DEFAULT_CONF		"./conf/mac2.conf"
# define NONE				0
# define ON					1
# define OFF				0

# define CGI_BUFFER_SIZE 	50000
# define LISTEN_BUFFER_SIZE 1024
# define READ_BUFFER_SIZE	100000

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
	Continue =		100,
	OK =			200,
	Created =		201,
	No_Content =	204,
	Bad_Request =			400,
	Forbidden =				403,
	Not_Found =				404,
	Method_Not_Allowed =	405,
	Payload_Too_Large =		413,
	Internal_Server_Error =	500,
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

int							printErr(std::string errMsg);
std::vector<std::string>	split(std::string str, char delimiter = '\n');
std::string					trim(std::string str);
std::vector<std::string>	splitBlocks(std::string block, std::string type);
std::pair<bool, size_t>		skipKey(std::string line, std::string str, std::string delimiter);
bool						isNumber(std::string str);
std::string					parseValue(std::string line, size_t pos, std::string delimiter);
int							strToInt(std::string str);
int							compareURIs(std::string URI, std::string request, int mod);

int							pathIsFile(const std::string& path);

std::string					setUri(const std::string& dirList, const std::string& dirName, const std::string& host, const int port);
std::string					setHtml(const std::string& path, const std::string& lang, const std::string& charset, const std::string& h1, const std::string& host, const int port);

int							setErrorPages(const std::string& errPages, std::map<int, std::string>* errMap);

std::string					intToStr(int code);
std::string					sizetToStr(size_t code);

void						printVec(std::vector<std::string> strVec);
void						printSet(std::set<std::string> strSet);
void						printErrmap(std::map<int, std::string> errmap);
void						printStr(const std::string& str, const std::string& response);

int							compareEnd(const std::string& s1, const std::string& s2);

std::string					findExtension(std::string& file);
std::string					findFileName(const std::string& path);
std::string					findFileType(const std::string& file);
std::string					eraseFileType(const std::string& file);

std::string					strTrimChar(const std::string& str, char deleteChar = ' ');
std::string					findHeaderValue(const std::string& header);
std::string					strDeleteRN(const std::string& str);
std::string					allDeleteRN(const std::string& str);

int							isStrUpper(const std::string& str);

int							makeHtml(const std::string& htmlName, int code, const std::string& codeStr, const std::string& serverName);

bool						hostToInt(std::string host, t_listen* listen);
int							setListen(std::string host, t_listen* listen);
size_t						calExponent(const std::string& str);

int							checkHex(std::string& hex);
size_t						hexToDecimal(std::string& hex);

#endif
