#ifndef __GENERAL_HEADER_HPP__
# define __GENERAL_HEADER_HPP__

# include "./ServerBlock.hpp"

class GeneralHeader
{
	public:
		GeneralHeader();
		GeneralHeader(const GeneralHeader& gh);
		virtual ~GeneralHeader();

		GeneralHeader&	operator=(const GeneralHeader& gh);

		std::string		getDate() const;
		std::string		getConnection() const;
		std::string		getTransferEncoding() const;
	
		void			setDate(const std::string& date = "");
		void	 		setConnection(const std::string& connection = "");
		void			setTransferEncoding(const std::string& transferEncoding = "");

		void			printGeneralHeader() const;

	protected:
		std::string	_date;
		std::string	_connection;
		std::string	_transferEncoding;
	
};

#endif
