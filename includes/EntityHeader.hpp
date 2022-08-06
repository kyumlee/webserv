#ifndef __ENTITY_HEADER_HPP__
# define __ENTITY_HEADER_HPP__

# include "./GeneralHeader.hpp"

class EntityHeader : public GeneralHeader
{
	public:
		EntityHeader();
		EntityHeader(const EntityHeader& eh);
		virtual ~EntityHeader();

		EntityHeader&			operator=(const EntityHeader& eh);

		std::string				getContentLength() const;
		std::string				getContentType() const;
		std::string				getContentLanguage() const;
		std::string				getContentLocation() const;
		std::string				getContentEncoding() const;
		std::string				getAllow() const;
		int						getCode() const;
		std::set<std::string>	getAllowedMethods() const;
		std::set<std::string>	getPossibleMethods() const;

		void					setContentLength(const size_t& size);
		void					setContentLength(const std::string& path, const std::string& contentLength);
		void					setContentTypeLocation(const std::string& path, std::string type, std::string contentLocation);
		void					setContentLanguage(const std::string& contentLanguage = "en");
		void					setContentLocation(const std::string& loc);
		void					setContentEncoding(const std::string& contentEncoding = "");
		void					setAllow(const std::string& allow);
		void					setAllow(const std::set<std::string>& methods);
		void					setCode(const int& code = 0);
		void					setAllowedMethod(const std::string& method);

		void					initPossibleMethods();
		void					initAllowedMethods(std::vector<std::string> allowedMethods);

		void					printEntityHeader() const;
	
	protected:
		std::string				_contentLength;
		std::string				_contentType;
		std::string				_contentLanguage;
		std::string				_contentLocation;
		std::string				_contentEncoding;
		std::string				_allow;
		
		int						_code;
		std::set<std::string>	_allowedMethods;
		std::set<std::string>	_possibleMethods;
};

#endif
