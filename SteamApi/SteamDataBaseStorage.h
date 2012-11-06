#pragma once

#include <sqlite3.h>
#include <cassert>

namespace Util
{
	class CSQLite_Error
	{
	public:
		CSQLite_Error();
		~CSQLite_Error();

		char ** operator&();
	private:
		char *m_pszError;
	};

	class CSQLite_Results
	{
	public:
		CSQLite_Results();
		CSQLite_Results(CSQLite_Results&&);
		~CSQLite_Results();

		bool FValid();
		bool FRunQuery(sqlite3 *pDB, const char * pszQuery);

		int NColumns() const { return m_nColumns; }
		int NRows() const { return m_nRows; }

		void swap(CSQLite_Results & other);

		char **operator [] (unsigned int i);
	private:
		CSQLite_Results & operator=(const CSQLite_Results & rhs);
		CSQLite_Results(const CSQLite_Results & rhs);

		char ** m_ppszResults;
		int     m_nRows;
		int     m_nColumns;
	};

	class CSQLite_Connection
	{
	public:
		CSQLite_Connection();

		bool FInitializeDatabase(const char *pszFileName);
		bool FTableExists(const char * pszTableName);
		bool FCreateTable(const char * pszTableName, const char * pszFieldDesc);

		CSQLite_Results FRunQuery(const char * pszQuery);

	private:  //methods
		bool FDatabaseConnected() { return m_pDB != nullptr; }

	private:
		sqlite3 *m_pDB;
		int      m_lastError;
	};

}