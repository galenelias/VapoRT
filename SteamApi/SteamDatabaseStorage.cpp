#include <pch.h>

#include "SteamDataBaseStorage.h"
#include "StandardUtils.h"

namespace Util
{

CSQLite_Error::CSQLite_Error()
	: m_pszError(nullptr)
{

}

char ** CSQLite_Error::operator&()
{
	assert(m_pszError == nullptr);
	return &m_pszError;
}

CSQLite_Error::~CSQLite_Error()
{
	if (m_pszError)
		sqlite3_free(m_pszError);
}

CSQLite_Connection::CSQLite_Connection()
	: m_pDB(nullptr), m_lastError(SQLITE_OK)
{

}

bool CSQLite_Connection::FInitializeDatabase(const char *pszFileName)
{
	assert(!FDatabaseConnected());

	m_lastError = sqlite3_open(pszFileName, &m_pDB);

	return (m_lastError == SQLITE_OK);
}


bool CSQLite_Connection::FTableExists(const char * pszTableName)
{
	std::string stwFindTableQuery = FormatStr("SELECT name FROM sqlite_master WHERE type='table' and name='%s';", pszTableName);

	CSQLite_Results results = FRunQuery(stwFindTableQuery.data());

	assert(results.NRows() <= 1);
	return results.NRows() > 0;
}

bool CSQLite_Connection::FCreateTable(const char * pszTableName, const char * pszFieldDesc)
{
	if (FTableExists(pszTableName))
		return true;

	std::string stwCreateTableQuery = FormatStr("CREATE TABLE %s ( %s );", pszTableName, pszFieldDesc);
	CSQLite_Error errorMsg;

	m_lastError = sqlite3_exec(m_pDB, stwCreateTableQuery.data(), NULL, NULL, &errorMsg);

	return (m_lastError == SQLITE_OK);
}


CSQLite_Results CSQLite_Connection::FRunQuery(const char * pszQuery)
{
	CSQLite_Results results;

	results.FRunQuery(m_pDB, pszQuery);

	return results;
}

CSQLite_Results::CSQLite_Results()
	: m_ppszResults(nullptr), m_nRows(0), m_nColumns(0)
{
	
}

CSQLite_Results::CSQLite_Results(CSQLite_Results&& other)
{
	this->swap(other);
}

void CSQLite_Results::swap(CSQLite_Results & other)
{
	std::swap(m_ppszResults, other.m_ppszResults);
	std::swap(m_nRows, other.m_nRows);
	std::swap(m_nColumns, other.m_nColumns);
}

CSQLite_Results::~CSQLite_Results()
{
	if (m_ppszResults)
		sqlite3_free_table(m_ppszResults);
	m_ppszResults = nullptr;
}

char ** CSQLite_Results::operator [] (unsigned int i)
{
	return m_ppszResults + (i+1) * NColumns();
}

// "CREATE INDEX idx_ConversationHistory ON ConversationHistory ( SteamID ASC  );"



//CSQLite_Results & CSQLite_Results::operator=(CSQLite_Results && rhs)
//{
//	std::swap(m_ppszResults, rhs.m_ppszResults);
//}

bool CSQLite_Results::FRunQuery(sqlite3 *pDB, const char * pszQuery)
{
	CSQLite_Error error;

	int nResult = sqlite3_get_table(
		pDB,                /* An open database */
		pszQuery,           /* SQL to be evaluated */
		&m_ppszResults,     /* Results of the query */
		&m_nRows,           /* Number of result rows written here */
		&m_nColumns,        /* Number of result columns written here */
		&error       /* Error msg written here */
		);
	
	return (nResult == SQLITE_OK);
}

}