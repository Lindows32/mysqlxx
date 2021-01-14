#ifndef LIBMYSQL_MYSQL_H
#define LIBMYSQL_MYSQL_H
#ifdef WINDOWS_MYSQL
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif
#include <vector>
#include <string>
namespace libsql
{
	class mysql :public MYSQL
	{
	private:
		class mysql_initor
		{
		public:
			mysql_initor();
			~mysql_initor();
		};
		static void(*ferror)(uint32_t err_no, const char* error);
		static mysql_initor __my_sql_environment;
		bool _connected;
		static char _code_page[32];


	public:
		//无法拷贝对象
		mysql(const MYSQL&) = delete;
		mysql(const mysql&) = delete;
		mysql(mysql&&) = delete;
		mysql& operator=(const mysql&) = delete;
		mysql& operator=(const MYSQL&) = delete;
		mysql& operator=(mysql&&) = delete;

		enum connect_flag
		{
			null = 0,
			client_compress = CLIENT_COMPRESS,//在客户端/服务器协议中使用压缩 
			client_found_rows = CLIENT_FOUND_ROWS,//返回找到(匹配)的行数,而不是更改的行数 
			client_ignore_sigpipe = CLIENT_IGNORE_SIGPIPE,//防止客户端库安装 SIGPIPE信号处理程序 这可以用来避免与应用程序已经安装的处理程序发生冲突 
			client_ignore_space = CLIENT_IGNORE_SPACE,//在函数名称后允许空格 使所有函数名称保留字 
			client_interactive = CLIENT_INTERACTIVE,//在关闭连接之前,允许 interactive_timeout 闲置几秒钟(而不是 wait_timeout几秒钟) 客户端的会话wait_timeout变量设置为会话 变量的值 interactive_timeout  
			client_local_files = CLIENT_LOCAL_FILES,//启用 LOAD DATA LOCAL处理 
			client_multi_results = CLIENT_MULTI_RESULTS,//告诉服务器客户端可以处理来自多个语句执行或存储过程的多个结果集 如果CLIENT_MULTI_STATEMENTS启用此标志,则会自动 启用 有关此标志的更多信息,请参见此表后面的注释 
			client_multi_statements = CLIENT_MULTI_STATEMENTS,//告诉服务器客户端可以在单个字符串(用; 字符分隔)中发送多个语句 如果未设置此标志,则禁用多语句执行 有关此标志的更多信息,请参见此表后面的注释 
			client_no_schema = CLIENT_NO_SCHEMA,//不允许 db_name.tbl_name.col_name 语法 这是用于ODBC的 如果使用该语法, 它将导致解析器生成错误, 这对于捕获某些ODBC程序中的错误很有用
			client_odbc = CLIENT_ODBC,//没用过
			//client_optional_resultset_metadata =CLIENT_OPTIONAL_RESULTSET_METADATA,//此标志使结果集元数据为可选,抑制元数据传输可以提高性能,特别是对于执行许多查询且每次返回很少行的会话
			client_ssl = CLIENT_SSL,//使用SSL(加密协议) 不要在应用程序中设置此选项 它在客户端库中内部设置 而是在调用之前 使用mysql_options()或mysql_ssl_set()mysql_real_connect()
			client_remember_options = CLIENT_REMEMBER_OPTIONS,//记住由调用指定的选项 mysql_options() 如果没有此选项,则如果 mysql_real_connect() 失败,则必须重新调用 mysql_options(),然后再尝试重新连接 使用此选项, mysql_options()无需重复会话 
		};
		void set_ferror(void(*fp)(uint32_t, const char*));
		void set_code_page(const char* code_page);
		uint32_t connect_database(const wchar_t* host_ip, const wchar_t* user_name, const wchar_t* passwd, const wchar_t* database_name, uint32_t port = 3306, connect_flag flag = null, void* sock_or_pipe = nullptr);
		uint32_t connect_database(const char* host_ip, const char* user_name, const char* passwd, const char* database_name, bool ansi = false, uint32_t port = 3306, connect_flag flag = null, void* sock_or_pipe = nullptr);
		uint32_t connect_local_database(const char* user_name, const char* passwd, const char* database_name, bool ansi = false, connect_flag flag = null, void* sock_or_pipe = nullptr);
		uint32_t connect_local_database(const wchar_t* user_name, const wchar_t* passwd, const wchar_t* database_name, connect_flag flag = null, void* sock_or_pipe = nullptr);
		uint32_t query(const char* statment, bool ansi = false);
		uint32_t query(const wchar_t* statment);


		class statement;
		class result_set
		{
			friend class statement;
			MYSQL_RES* _result;
			mysql* _mysql_object;
			statement* _stmt;
			size_t _rows;
			result_set(mysql* parent);
			result_set(statement* parent);

			class MYSQL_BIND_class :public MYSQL_BIND
			{
			public:
				struct buffer_class
				{
					union
					{
						bool b;
						int8_t c;
						uint16_t w;
						uint32_t i;
						uint64_t l;
						float f;
						double d;
						MYSQL_TIME t;
						int8_t s[256];
					};
					char* bin_buffer;
					bool is_null;
					unsigned long length;
				};
				MYSQL_BIND_class();
				void set_buffer(enum_field_types buffer_type, uint32_t buffer_size);
				~MYSQL_BIND_class();
			};
			std::vector<MYSQL_BIND_class> _bind_data;
		public:
			~result_set();
			result_set& set_buffer_size(uint32_t index, size_t size);
			size_t get_rows() { return _rows; }
			void execute();
			bool fetch();
			uint64_t get_int(uint32_t index, bool* is_null = nullptr, bool* is_unsigned = nullptr);
			float get_float(uint32_t index, bool* is_null = nullptr);
			double get_double(uint32_t index, bool* is_null = nullptr);
			std::string get_string(uint32_t index, bool ansi = false, bool* is_null = nullptr);
			std::wstring get_wstring(uint32_t index, bool* is_null = nullptr);
			void get_blob(uint32_t index, void** buffer_ptr, size_t* length, bool* is_null = nullptr);
			time_t get_timestamp(uint32_t index, bool* is_null = nullptr);
			MYSQL_TIME& get_datetime(uint32_t index, bool* is_null = nullptr);
			void finalize();
		};
		class statement
		{
			friend class mysql;
			mysql* _mysql_object;
			MYSQL_STMT* _stmt;
			statement(mysql* parent);
			class MYSQL_BIND_class :public MYSQL_BIND
			{
			public:
				struct buffer_class
				{
					union
					{
						bool b;
						char c;
						uint16_t w;
						uint32_t i;
						uint64_t l;
						float f;
						double d;
						MYSQL_TIME t;
					};
					std::string text_buffer;
					std::vector<char> blob_buffer;
				};
				MYSQL_BIND_class();
				void set_values(enum_field_types buffer_type, uint32_t size, void* value, bool is_unsigned, bool is_null);
				~MYSQL_BIND_class();
			};
			std::vector<MYSQL_BIND_class> _bind_data;
		public:
			operator MYSQL_STMT* const();
			uint32_t is_open();
			uint32_t prepare(const char* sql, bool ansi = false);
			uint32_t prepare(const wchar_t* sql);
			//statement& operator=(statement&& src);
			enum level
			{
				tiny = sizeof(char),
				mini = sizeof(uint16_t),
				normal = sizeof(uint32_t),
				large = sizeof(uint64_t)
			};
			statement& bind_int(uint32_t index, uint64_t value, size_t size, bool is_unsigned = false, bool is_null = false);
			statement& bind_double(uint32_t index, double value, bool is_null = false);
			statement& bind_float(uint32_t index, float value, bool is_null = false);

			statement& bind_string(uint32_t index, const char* value, bool ansi = false);
			statement& bind_string(uint32_t index, const wchar_t* value);

			statement& bind_blob(uint32_t index, void* value, uint32_t length, level lv = tiny, bool is_null = false);

			statement& bind_datetime(uint32_t index, MYSQL_TIME& time, bool is_null = false);
			statement& bind_timestamp(uint32_t index, time_t time, bool is_null = false);
			uint32_t execute();

			void reset();
			void finalize();
			void rebuild();
			result_set get_result();
		};

		statement create_statment();
		mysql();
		~mysql();
	};
}
#endif
