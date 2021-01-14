#include "mysqlxx.h"
#ifdef _WINDOWS
#pragma comment(lib,"libmysql.lib")
#endif
#include <boost/locale/encoding.hpp>
#include <time.h>
#include <string>
#include <string.h>
using std::string;
using std::wstring;
using namespace libsql;
using boost::locale::conv::to_utf;
using boost::locale::conv::from_utf;
using boost::locale::conv::between;
using boost::locale::conv::utf_to_utf;

#define W2U(str) from_utf(wstring(str), "UTF-8").c_str()
#define U2W(str) utf_to_utf(string(str)).c_str()
#define A2U(str) between(string(str),"UTF-8",_code_page).c_str()
#define U2A(str) between(string(str),_code_page,"UTF-8").c_str()
#define A2W(str) to_utf<wchar_t>(string(str), _code_page).c_str()
#define W2A(str) from_utf(wstring(str), _code_page).c_str()

mysql::mysql_initor mysql::__my_sql_environment;
void(*mysql::ferror)(uint32_t, const char* error);
char mysql::_code_page[32] = "GBK";
mysql::mysql_initor::mysql_initor()
{
	ferror = nullptr;
	if (mysql_library_init(0, nullptr, nullptr))
		exit(0);
}

mysql::mysql_initor::~mysql_initor()
{
	mysql_library_end();
}

uint32_t mysql::connect_database(const wchar_t* host_ip, const wchar_t* user_name, const wchar_t* passwd, const wchar_t* database_name, uint32_t port, connect_flag flag, void* sock_or_pipe)
{
	mysql_real_connect(this, W2U(host_ip), W2U(user_name), W2U(passwd), W2U(database_name), port, (const char*)sock_or_pipe, flag);
	uint32_t res = mysql_errno(this);
	_connected = !res;
	if (res && ferror)
		ferror(res, mysql_error(this));
	return res;
}

uint32_t mysql::connect_database(const char* host_ip, const char* user_name, const char* passwd, const char* database_name, bool ansi, uint32_t port, connect_flag flag, void* sock_or_pipe)
{
	if (ansi)
		mysql_real_connect(this, A2U(host_ip), A2U(user_name), A2U(passwd), A2U(database_name), port, (const char*)sock_or_pipe, flag);
	else
		mysql_real_connect(this, host_ip, user_name, passwd, database_name, port, (const char*)sock_or_pipe, flag);
	uint32_t res = mysql_errno(this);
	_connected = !res;
	if (res && ferror)
		ferror(res, mysql_error(this));
	return res;
}

uint32_t mysql::connect_local_database(const char* user_name, const char* passwd, const char* database_name, bool ansi, connect_flag flag, void* sock_or_pipe)
{
	if (ansi)
		mysql_real_connect(this, nullptr, A2U(user_name), A2U(passwd), A2U(database_name), 0, (const char*)sock_or_pipe, flag);
	else
		mysql_real_connect(this, nullptr, user_name, passwd, database_name, 0, (const char*)sock_or_pipe, flag);
	uint32_t res = mysql_errno(this);
	_connected = !res;
	if (res && ferror)
		ferror(res, mysql_error(this));
	return res;
}

uint32_t mysql::connect_local_database(const wchar_t* user_name, const wchar_t* passwd, const wchar_t* database_name, connect_flag flag, void* sock_or_pipe)
{
	mysql_real_connect(this, nullptr, W2U(user_name), W2U(passwd), W2U(database_name), 0, (const char*)sock_or_pipe, flag);
	uint32_t res = mysql_errno(this);
	_connected = !res;
	if (res && ferror)
		ferror(res, mysql_error(this));
	return res;
}

uint32_t mysql::query(const char* statment, bool ansi)
{
	if (ansi)
		mysql_query(this, A2U(statment));
	else
		mysql_query(this, statment);
	return mysql_errno(this);
}

uint32_t mysql::query(const wchar_t* statment)
{
	mysql_query(this, W2U(statment));
	uint32_t res = mysql_errno(this);
	if (res && ferror)
		ferror(res, mysql_error(this));
	return res;
}

mysql::statement mysql::create_statment()
{
	return mysql::statement(this);
}

mysql::mysql()
{
	_connected = false;
	if (mysql_init(this))
	{
		mysql_options(this, MYSQL_SET_CHARSET_NAME, "utf8mb4");
	}
}

mysql::~mysql()
{
	mysql_close(this);
}

mysql::statement::statement(mysql* parent)
{
	_mysql_object = parent;
	_stmt = mysql_stmt_init(parent);
}

void mysql::set_code_page(const char* code_page)
{
#ifdef _WINDOWS
	strcpy_s(_code_page, code_page);
#else
	strcpy(_code_page, code_page);
#endif
}

void mysql::set_ferror(void(*fp)(uint32_t, const char*))
{
	ferror = fp;
}

mysql::statement::operator MYSQL_STMT* const()
{
	return _stmt;
}

uint32_t mysql::statement::is_open()
{
	return _stmt != nullptr;
}

uint32_t mysql::statement::prepare(const char* sql, bool ansi)
{
	string string_sql;
	if (ansi)
		string_sql = between(string(sql), "UTF-8", _code_page);
	else
		string_sql = sql;
	mysql_stmt_prepare(_stmt, string_sql.c_str(), string_sql.length() + 1);
	uint32_t param_count = mysql_stmt_param_count(_stmt);
	_bind_data.resize(param_count);
	uint32_t res = mysql_errno(_mysql_object);
	if (res && ferror)
		ferror(res, mysql_error(_mysql_object));
	return res;
}

uint32_t mysql::statement::prepare(const wchar_t* sql)
{
	string string_sql = from_utf(wstring(sql), "UTF-8");
	uint32_t res = mysql_stmt_prepare(_stmt, string_sql.c_str(), string_sql.length());
	uint32_t param_count = mysql_stmt_param_count(_stmt);
	_bind_data.resize(param_count);
	res = mysql_errno(_mysql_object);
	if (res && ferror)
		ferror(res, mysql_error(_mysql_object));
	return res;
}

//mysql::statement& mysql::statement::operator=(statement&& src)
//{
//	_mysql_object = src._mysql_object;
//	_stmt = src._stmt;
//	_result = src._result;
//	_bind_data = std::move(src._bind_data);
//	return *this;
//}

mysql::statement& mysql::statement::bind_int(uint32_t index, uint64_t value, size_t size, bool is_unsigned, bool is_null)
{

	switch (size)
	{
	case tiny:
	{
		char temp = (char)value;
		_bind_data[index - 1].set_values(MYSQL_TYPE_TINY, sizeof(char), &temp, is_unsigned, is_null);
		break;
	}
	case mini:
	{
		uint16_t temp = (uint16_t)value;
		_bind_data[index - 1].set_values(MYSQL_TYPE_SHORT, sizeof(uint16_t), &temp, is_unsigned, is_null);
		break;
	}
	case normal:
	{
		uint32_t temp = (uint32_t)value;
		_bind_data[index - 1].set_values(MYSQL_TYPE_LONG, sizeof(uint32_t), &temp, is_unsigned, is_null);
		break;
	}
	case large:
	{
		uint64_t temp = (uint64_t)value;
		_bind_data[index - 1].set_values(MYSQL_TYPE_LONGLONG, sizeof(uint64_t), &temp, is_unsigned, is_null);
		break;
	}
	default:
		bool temp = (bool)value;
		_bind_data[index - 1].set_values(MYSQL_TYPE_BIT, sizeof(bool), &temp, is_unsigned, is_null);
		break;
	}
	return *this;
}

mysql::statement& mysql::statement::bind_double(uint32_t index, double value, bool is_null)
{
	_bind_data[index - 1].set_values(MYSQL_TYPE_DOUBLE, sizeof(double), &value, false, is_null);
	return *this;
}

mysql::statement& mysql::statement::bind_float(uint32_t index, float value, bool is_null)
{
	_bind_data[index - 1].set_values(MYSQL_TYPE_FLOAT, sizeof(float), &value, false, is_null);
	return *this;
}

mysql::statement& mysql::statement::bind_string(uint32_t index, const char* value, bool ansi)
{
	if (value)
	{
		if (ansi)
			_bind_data[index - 1].set_values(MYSQL_TYPE_STRING, 0, (char*)A2U(value), false, false);
		else
			_bind_data[index - 1].set_values(MYSQL_TYPE_STRING, 0, (char*)value, false, false);
	}
	return *this;
}

mysql::statement& mysql::statement::bind_string(uint32_t index, const wchar_t* value)
{
	if (value)
		_bind_data[index - 1].set_values(MYSQL_TYPE_STRING, 0, (char*)W2U(value), false, false);
	return *this;
}

mysql::statement& mysql::statement::bind_blob(uint32_t index, void* value, uint32_t length, level lv, bool is_null)
{
	if (value)
	{
		enum_field_types current_type;
		switch (lv)
		{
		case tiny:
			current_type = MYSQL_TYPE_TINY_BLOB;
			break;
		case mini:
			current_type = MYSQL_TYPE_BLOB;
			break;
		case normal:
			current_type = MYSQL_TYPE_MEDIUM_BLOB;
			break;
		case large:
			current_type = MYSQL_TYPE_LONG_BLOB;
			break;
		default:
			break;
		}
		_bind_data[index - 1].set_values(current_type, length, value, false, false);
	}
	return *this;
}


mysql::statement& mysql::statement::bind_datetime(uint32_t index, MYSQL_TIME& time, bool is_null)
{
	_bind_data[index - 1].set_values(MYSQL_TYPE_DATETIME, sizeof(MYSQL_TIME), &time, false, is_null);
	return *this;
}

mysql::statement& mysql::statement::bind_timestamp(uint32_t index, time_t time, bool is_null)
{
	_bind_data[index - 1].set_values(MYSQL_TYPE_TIMESTAMP, sizeof(time_t), &time, true, is_null);
	return *this;
}

uint32_t mysql::statement::execute()
{
	if (_bind_data.size())
		mysql_stmt_bind_param(_stmt, _bind_data.data());
	mysql_stmt_execute(_stmt);
	uint32_t res = mysql_errno(_mysql_object);
	if (res && ferror)
		ferror(res, mysql_error(_mysql_object));
	return res;
}

void mysql::statement::reset()
{
	for (auto& r : _bind_data)
	{
		char* buffer = (char*)r.buffer;
		unsigned length = r.buffer_length;
		MYSQL_BIND_class::buffer_class* extension = (MYSQL_BIND_class::buffer_class*)r.extension;
		memset(&((MYSQL_BIND_class::buffer_class*)extension)->t, 0, sizeof(MYSQL_TIME));
		memset(&r, 0, sizeof(MYSQL_BIND_class));
		r.extension = extension;
		r.buffer = buffer;
		r.buffer_type = MYSQL_TYPE_NULL;
		r.is_null = &r.is_null_value;
		r.is_null_value = true;
	}
	mysql_stmt_reset(_stmt);
}

void mysql::statement::finalize()
{
	if (_stmt)
	{
		mysql_stmt_close(_stmt);
		_stmt = nullptr;
	}
}

void mysql::statement::rebuild()
{
	finalize();
	_bind_data.clear();
	_stmt = mysql_stmt_init(_mysql_object);
}

mysql::result_set mysql::statement::get_result()
{
	return result_set(this);
}


mysql::statement::MYSQL_BIND_class::MYSQL_BIND_class()
{
	memset(this, 0, sizeof(MYSQL_BIND_class));
	this->buffer_type = MYSQL_TYPE_NULL;
	extension = new buffer_class;
	buffer = extension;
	memset(&((buffer_class*)extension)->t, 0, sizeof(MYSQL_TIME));
	is_null = &is_null_value;
	is_null_value = true;
}

void mysql::statement::MYSQL_BIND_class::set_values(enum_field_types buffer_type, uint32_t size, void* value, bool is_unsigned, bool is_null)
{

#define NEWS \

#ifdef _WINDOWS
#define NEWB \
	{\
	((buffer_class*)extension)->blob_buffer.resize(size);\
	memcpy_s(((buffer_class*)extension)->blob_buffer.data(),size,value,size);\
	this->buffer = (void*)((buffer_class*)extension)->blob_buffer.data();\
	buffer_length=size;\
	}
#else
#define NEWB \
	{\
	((buffer_class*)extension)->blob_buffer.resize(size);\
	memcpy(((buffer_class*)extension)->blob_buffer.data(),value,size);\
	this->buffer = (void*)((buffer_class*)extension)->blob_buffer.data();\
	buffer_length=size;\
	}
#endif

	if (!is_null)
	{
		this->buffer_type = buffer_type;
		this->is_unsigned = is_unsigned;
		this->buffer_length = size;
		this->is_null = nullptr;
		is_null_value = false;
		switch (buffer_type)
		{
		case MYSQL_TYPE_DECIMAL:
			//?
			break;
		case MYSQL_TYPE_TINY:
			((buffer_class*)extension)->c = *(char*)value;
			buffer = extension;
			break;
		case MYSQL_TYPE_SHORT:
			((buffer_class*)extension)->w = *(uint16_t*)value;
			buffer = extension;
			break;
		case MYSQL_TYPE_LONG:
			((buffer_class*)extension)->i = *(uint32_t*)value;
			buffer = extension;
			break;
		case MYSQL_TYPE_FLOAT:
			((buffer_class*)extension)->f = *(float*)value;
			buffer = extension;
			break;
		case MYSQL_TYPE_DOUBLE:
			((buffer_class*)extension)->d = *(double*)value;
			buffer = extension;
			break;
		case MYSQL_TYPE_NULL:
			break;
		case MYSQL_TYPE_TIMESTAMP:
		{
			MYSQL_TIME time = { 0 };
			tm stTime;
#ifdef _WINDOWS
			localtime_s(&stTime, (const time_t*)value);
#else
			tm* ptm = localtime((const time_t*)value);
			memcpy(&stTime, ptm, sizeof(tm));
#endif
			MYSQL_TIME my_time = { 0 };
			my_time.year = 1900 + stTime.tm_year;
			my_time.month = 1 + stTime.tm_mon;
			my_time.day = stTime.tm_mday;
			my_time.hour = stTime.tm_hour;
			my_time.minute = stTime.tm_min;
			my_time.second = stTime.tm_sec;
			memcpy(&((buffer_class*)extension)->t, &my_time, sizeof(MYSQL_TIME));
			buffer = extension;
		}
		break;
		case MYSQL_TYPE_LONGLONG:
			((buffer_class*)extension)->l = *(uint64_t*)value;
			buffer = extension;
			break;
		case MYSQL_TYPE_DATE:
			memcpy(&((buffer_class*)extension)->t, value, sizeof(MYSQL_TIME));
			buffer = extension;
			break;
		case MYSQL_TYPE_TIME:
			memcpy(&((buffer_class*)extension)->t, value, sizeof(MYSQL_TIME));
			buffer = extension;
			break;
		case MYSQL_TYPE_DATETIME:
			memcpy(&((buffer_class*)extension)->t, value, sizeof(MYSQL_TIME));
			buffer = extension;
			break;
		case MYSQL_TYPE_YEAR:
			memcpy(&((buffer_class*)extension)->t, value, sizeof(MYSQL_TIME));
			buffer = extension;
			break;
		case MYSQL_TYPE_BIT:
			((buffer_class*)extension)->b = *(bool*)value;
			buffer = extension;
			break;
		case MYSQL_TYPE_NEWDECIMAL:
			break;
		case MYSQL_TYPE_ENUM:
			memcpy(&((buffer_class*)extension)->i, value, sizeof(uint32_t));
			buffer = extension;
			break;
		case MYSQL_TYPE_TINY_BLOB:
			NEWB
				break;
		case MYSQL_TYPE_BLOB:
			NEWB
				break;
		case MYSQL_TYPE_MEDIUM_BLOB:
			NEWB
				break;
		case MYSQL_TYPE_LONG_BLOB:
			NEWB
				break;
		case MYSQL_TYPE_STRING:
		{
			((buffer_class*)extension)->text_buffer = (const char*)value;
			this->buffer = (void*)((buffer_class*)extension)->text_buffer.c_str();
			buffer_length = ((buffer_class*)extension)->text_buffer.length();
		}
		break;
		default:
			break;
		}
	}
	else
	{
		this->is_null = &is_null_value;
		is_null_value = true;
		this->buffer_type = MYSQL_TYPE_NULL;
	}
#undef NEWS
}

mysql::statement::MYSQL_BIND_class::~MYSQL_BIND_class()
{
	char* buffer = (char*)this->buffer;
	MYSQL_BIND_class::buffer_class* extension = (MYSQL_BIND_class::buffer_class*)this->extension;
	delete extension;
}

mysql::result_set::MYSQL_BIND_class::MYSQL_BIND_class()
{
	memset(this, 0, sizeof(MYSQL_BIND_class));
	this->buffer_type = MYSQL_TYPE_NULL;
	extension = new buffer_class;
	buffer = extension;
	memset(&((buffer_class*)extension)->t, 0, sizeof(MYSQL_TIME));
	((buffer_class*)extension)->bin_buffer = nullptr;
	((buffer_class*)extension)->is_null = false;
	((buffer_class*)extension)->length = 0;
	is_null = &is_null_value;
	is_null_value = true;
}

void mysql::result_set::MYSQL_BIND_class::set_buffer(enum_field_types buffer_type, uint32_t buffer_size)
{
	this->buffer_type = buffer_type;
	if ((buffer_type == MYSQL_TYPE_VARCHAR || buffer_type >= MYSQL_TYPE_TINY_BLOB) && buffer_size > sizeof(buffer_class::s))
		this->buffer_length = buffer_size;

	this->is_null = (my_bool*)&((buffer_class*)extension)->is_null;
	this->length = &((buffer_class*)extension)->length;
	switch (buffer_type)
	{
	case MYSQL_TYPE_DECIMAL:
		//?
		break;
	case MYSQL_TYPE_TINY:
		buffer = &(((buffer_class*)extension)->c);
		this->buffer_length = sizeof(buffer_class::c);
		break;
	case MYSQL_TYPE_SHORT:
		buffer = &(((buffer_class*)extension)->w);
		this->buffer_length = sizeof(buffer_class::w);
		break;
	case MYSQL_TYPE_LONG:
		buffer = &(((buffer_class*)extension)->i);
		this->buffer_length = sizeof(buffer_class::i);
		break;
	case MYSQL_TYPE_FLOAT:
		buffer = &(((buffer_class*)extension)->f);
		this->buffer_length = sizeof(buffer_class::f);
		break;
	case MYSQL_TYPE_DOUBLE:
		buffer = &(((buffer_class*)extension)->d);
		this->buffer_length = sizeof(buffer_class::d);
		break;
	case MYSQL_TYPE_NULL:
		buffer = extension;
		break;
	case MYSQL_TYPE_LONGLONG:
		buffer = &(((buffer_class*)extension)->l);
		this->buffer_length = sizeof(buffer_class::l);
		break;
	case MYSQL_TYPE_BIT:
		buffer = &(((buffer_class*)extension)->t);
		this->buffer_length = sizeof(buffer_class::t);
		break;
	case MYSQL_TYPE_NEWDECIMAL:
		break;
	case MYSQL_TYPE_ENUM:
		buffer = &(((buffer_class*)extension)->i);
		this->buffer_length = sizeof(buffer_class::i);
		break;
	default:
	{
		if (buffer_type == MYSQL_TYPE_VARCHAR || buffer_type >= MYSQL_TYPE_TINY_BLOB)
		{
			if (buffer_size <= sizeof(buffer_class::s))
			{
				buffer = &(((buffer_class*)extension)->t);
				this->buffer_length = sizeof(buffer_class::s);
			}
			else
			{
				delete[]((buffer_class*)extension)->bin_buffer;
				((buffer_class*)extension)->bin_buffer = new char[buffer_size];
				this->buffer = ((buffer_class*)extension)->bin_buffer;
			}
		}
		else if (buffer_type == MYSQL_TYPE_TIMESTAMP || buffer_type > MYSQL_TYPE_INT24 && buffer_type < MYSQL_TYPE_NEWDATE)
		{
			buffer = &(((buffer_class*)extension)->t);
			this->buffer_length = sizeof(buffer_class::t);
		}
	}
	break;
	}
}

mysql::result_set::MYSQL_BIND_class::~MYSQL_BIND_class()
{
	char* buffer = (char*)this->buffer;
	MYSQL_BIND_class::buffer_class* extension = (MYSQL_BIND_class::buffer_class*)this->extension;
	delete extension->bin_buffer;
	delete extension;
}


mysql::result_set::result_set(mysql* parent)
{
	_mysql_object = parent;
	_result = mysql_store_result(parent);
	_stmt = nullptr;
	_result = nullptr;
}

mysql::result_set::result_set(statement* parent)
{
	_stmt = parent;
	_mysql_object = _stmt->_mysql_object;
	_result = nullptr;
	_result = mysql_stmt_result_metadata(_stmt->_stmt);
	if (_result)
	{
		uint32_t width = mysql_num_fields(_result);
		_bind_data.resize(width);
		for (int i = 0; i < width; i++)
		{
			MYSQL_FIELD* field = mysql_fetch_field(_result);
			_bind_data[i].set_buffer(field->type, 0);
		}
	}
	else
	{

	}
}

mysql::result_set& mysql::result_set::set_buffer_size(uint32_t index, size_t size)
{
	_bind_data[index].set_buffer(_bind_data[index].buffer_type, size);
	return *this;
}

void mysql::result_set::execute()
{
	mysql_stmt_bind_result(_stmt->_stmt, _bind_data.data());
	_stmt->execute();
	mysql_stmt_store_result(_stmt->_stmt);
	_rows = mysql_stmt_num_rows(_stmt->_stmt);
}

bool mysql::result_set::fetch()
{
	return mysql_stmt_fetch(_stmt->_stmt);
}

uint64_t mysql::result_set::get_int(uint32_t index, bool* is_null, bool* is_unsigned)
{
	bool t_is_null = ((MYSQL_BIND_class::buffer_class*)_bind_data[index].extension)->is_null;
	if (is_null)
		*is_null = t_is_null;
	if (is_unsigned)
		*is_unsigned = _bind_data[index].is_unsigned;
	uint32_t* p = (uint32_t*)(MYSQL_BIND_class::buffer_class*)_bind_data[index].buffer;
	switch (_bind_data[index].buffer_type)
	{
	case MYSQL_TYPE_TINY:
		return *(uint8_t*)(MYSQL_BIND_class::buffer_class*)_bind_data[index].buffer;
	case MYSQL_TYPE_SHORT:
		return *(uint16_t*)(MYSQL_BIND_class::buffer_class*)_bind_data[index].buffer;
	case MYSQL_TYPE_LONG:
		return *(uint32_t*)(MYSQL_BIND_class::buffer_class*)_bind_data[index].buffer;
	case MYSQL_TYPE_LONGLONG:
		return *(uint64_t*)(MYSQL_BIND_class::buffer_class*)_bind_data[index].buffer;
	default:
		return 0;
	}
}

float mysql::result_set::get_float(uint32_t index, bool* is_null)
{
	bool t_is_null = ((MYSQL_BIND_class::buffer_class*)_bind_data[index].extension)->is_null;
	if (is_null)
		*is_null = t_is_null;
	if (!t_is_null)
		return *(float*)(MYSQL_BIND_class::buffer_class*)_bind_data[index].buffer;
	else
		return 0;
}

double mysql::result_set::get_double(uint32_t index, bool* is_null)
{
	bool t_is_null = ((MYSQL_BIND_class::buffer_class*)_bind_data[index].extension)->is_null;
	if (is_null)
		*is_null = t_is_null;
	if (!t_is_null)
		return *(double*)(MYSQL_BIND_class::buffer_class*)_bind_data[index].buffer;
	else
		return 0;
}

std::string mysql::result_set::get_string(uint32_t index, bool ansi, bool* is_null)
{
	bool t_is_null = ((MYSQL_BIND_class::buffer_class*)_bind_data[index].extension)->is_null;
	if (is_null)
		*is_null = t_is_null;
	if (!t_is_null)
	{
		if (ansi)
			return between((const char*)(MYSQL_BIND_class::buffer_class*)_bind_data[index].buffer, _code_page, "UTF-8");
		else
			return std::string((const char*)(MYSQL_BIND_class::buffer_class*)_bind_data[index].buffer);
	}
	else
		return string();
}

std::wstring mysql::result_set::get_wstring(uint32_t index, bool* is_null)
{
	bool t_is_null = ((MYSQL_BIND_class::buffer_class*)_bind_data[index].extension)->is_null;
	if (is_null)
		*is_null = t_is_null;
	if (!t_is_null)
		return to_utf<wchar_t>(string((const char*)(MYSQL_BIND_class::buffer_class*)_bind_data[index].buffer), "UTF-8");
	else
		return wstring();
}

void mysql::result_set::get_blob(uint32_t index, void** buffer_ptr, size_t* length, bool* is_null)
{
	bool t_is_null = ((MYSQL_BIND_class::buffer_class*)_bind_data[index].extension)->is_null;
	if (is_null)
		*is_null = t_is_null;
	if (!t_is_null)
	{
		*buffer_ptr = (MYSQL_BIND_class::buffer_class*)_bind_data[index].buffer;
		*length = _bind_data[index].length_value;
	}
	else
	{
		*buffer_ptr = nullptr;
		*length = 0;
	}
}

time_t mysql::result_set::get_timestamp(uint32_t index, bool* is_null)
{
	bool t_is_null = ((MYSQL_BIND_class::buffer_class*)_bind_data[index].extension)->is_null;
	if (is_null)
		*is_null = t_is_null;
	if (!t_is_null)
	{
		MYSQL_TIME& r = *(MYSQL_TIME*)(MYSQL_BIND_class::buffer_class*)_bind_data[index].buffer;
		tm stTime = { 0 };
		stTime.tm_year = r.year - 1900;
		stTime.tm_mon = r.month - 1;
		stTime.tm_mday = r.day;
		stTime.tm_hour = r.hour;
		stTime.tm_min = r.minute;
		stTime.tm_sec = r.second;
		return mktime(&stTime);
	}
	else
		return 0;
}

MYSQL_TIME& mysql::result_set::get_datetime(uint32_t index, bool* is_null)
{
	bool t_is_null = ((MYSQL_BIND_class::buffer_class*)_bind_data[index].extension)->is_null;
	if (is_null)
		*is_null = t_is_null;
	return *(MYSQL_TIME*)(MYSQL_BIND_class::buffer_class*)_bind_data[index].buffer;
}

void mysql::result_set::finalize()
{
	if (_result)
	{
		mysql_free_result(_result);
		_result = nullptr;
	}
	if (_stmt && _stmt->_stmt)
	{
		mysql_stmt_free_result(_stmt->_stmt);
	}
}

mysql::result_set::~result_set()
{
	finalize();
}
