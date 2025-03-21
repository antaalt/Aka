#pragma once

#include <Aka/Core/Container/String.h>

namespace aka {

struct Error {
	String message;
};

template <typename T, typename Err = Error>
struct Result {
	explicit Result(T&& data) :
		m_data(data),
		m_isOk(true)
	{
	}
	explicit Result(Err&& err) :
		m_error(err),
		m_isOk(false)
	{
	}
	~Result() {
		if (m_isOk) {
			if constexpr (std::is_destructible<T>::value)
				m_data.~T();
		}
		else
		{
			if constexpr (std::is_destructible<Err>::value)
				m_error.~Err();
		}
	}
	Result(const Result<T, Err>&) = delete;
	Result& operator=(const Result<T, Err>&) = delete;
	Result(Result<T, Err>&&) = default;
	Result& operator=(Result<T, Err>&&) = default;
	static Result<T, Err> error(Err err) {
		return Result<T, Err>(std::move(err));
	}
	static Result<T, Err> ok(T data) {
		return Result<T, Err>(std::move(data));
	}
	T& getData() {
		if (isErr())
			throw std::runtime_error("Failed to get data");
		return m_data;
	}
	Err& getError() {
		if (isOk())
			throw std::runtime_error("Failed to get error");
		return m_error;
	}
	bool isOk() const {
		return m_isOk;
	}
	bool isErr() const {
		return !m_isOk;
	}
private:
	union {
		T m_data;
		Err m_error;
	};
	const bool m_isOk;
};

}