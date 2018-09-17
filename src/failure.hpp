#ifndef FAILURE_HPP
#define FAILURE_HPP

#include <ostream>
#include <functional>

struct Error
{
	int code;
	std::string message;

public:
	Error(int _code, std::string & _msg): 
		code(_code), message(_msg) {}

	Error(const std::string & _msg):
		code(-1), message(_msg) {}

	Error(const char * _msg):
		code(-1), message(_msg) {}

	Error(int _code):
		code(_code), message("") {}
};

template <typename ST>
class ResultOrError
{
private:
	const bool m_success;

	union {
		ST m_successValue;
		Error m_failValue;
	};

public:
	ResultOrError();

	ResultOrError(const ResultOrError & res):
		m_success(res.m_success)
	{
		if (res.m_success) {
			m_successValue = res.m_successValue;
		}
		else {
			m_failValue = res.m_failValue;
		}
	}

	ResultOrError(const ST & val):
		m_success(true), m_successValue(val) {}

	ResultOrError(const Error & err):
		m_success(false), m_failValue(err) {}

	~ResultOrError() {}
	
	template <typename T>
	static ResultOrError<ST> succeed(const T & arg) {
		return ResultOrError<T>(arg);
	}

	template <typename T>
	static ResultOrError<ST> fail(const Error & err) {
		return ResultOrError<T>(err);
	}

	bool succeeded() {
		return m_success;
	}

	bool failed() {
		return !m_success;
	}

	ResultOrError<ST> & onSuccess(std::function<void(const ST &)> func) {
		if (succeeded())
			func(m_successValue);
		return *this;
	}

	ResultOrError<ST> & onFailure(std::function<void(const Error &)> func) {
		if (failed())
			func(m_failValue);
		return *this;
	}

	template <typename MT>
	ResultOrError<MT> mapSuccess(std::function<ResultOrError<MT>(const ST &)> mapper) {
		if (succeeded())
			return mapper(m_successValue);
		return m_failValue;
	}
	
	const ST & getResult() {
		return m_successValue;
	}

	const Error & getError() {
		return m_failValue;
	}
};

template <typename ST>
ResultOrError<ST> succeed(const ST & arg) 
{
	return ResultOrError<ST>(arg);
}

inline Error fail(const Error & err) 
{
	return err;
}

#endif