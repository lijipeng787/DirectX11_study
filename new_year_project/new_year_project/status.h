#pragma once

#ifndef APPLICATION_HEADER_STATUS_H

#define APPLICATION_HEADER_STATUS_H

#include <string>
#include "slice.h"

class Status {
private:
	enum class ErrorCode {
		kOk = 0,
		kCreatedFailed,
		kInvalidArgument,
		kIOError,
		kNotSupported,
		kMapedFailed,
		kShaderCompiledFailed,
		kNotFound,
		kCorruption
	};

public:
	Status() {}

	Status(const Status& copy);
	Status& operator=(const Status& copy);

	~Status() { delete[] state_; }

	static Status Ok() { return Status(); }

	static Status CreatedFailed(const Slice& msg, const Slice& msg2 = Slice()) {
		return Status(ErrorCode::kCreatedFailed, msg, msg2);
	}

	static Status InvalidArgument(const Slice& msg, const Slice& msg2 = Slice()) {
		return Status(ErrorCode::kInvalidArgument, msg, msg2);
	}

	static Status IOError(const Slice& msg, const Slice& msg2 = Slice()) {
		return Status(ErrorCode::kIOError, msg, msg2);
	}

	static Status NotSupported(const Slice& msg, const Slice& msg2 = Slice()) {
		return Status(ErrorCode::kNotSupported, msg, msg2);
	}

	static Status MapedFailed(const Slice& msg, const Slice& msg2 = Slice()) {
		return Status(ErrorCode::kMapedFailed, msg, msg2);
	}

	static Status ShaderCompiledFailed(const Slice& msg, const Slice& msg2 = Slice()) {
		return Status(ErrorCode::kShaderCompiledFailed, msg, msg2);
	}

	static Status Corruption(const Slice& msg, const Slice& msg2 = Slice()) {
		return Status(ErrorCode::kCorruption, msg, msg2);
	}

	static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) {
		return Status(ErrorCode::kNotFound, msg, msg2);
	}

	bool IsOk()const { return (nullptr == state_); }

	bool IsNotFound()const { return (ErrorCode::kNotFound == code()); }

	bool IsCorruption()const { return (ErrorCode::kCorruption == code()); }

	bool IsIOError()const { return (ErrorCode::kIOError == code()); }

	bool IsNotSupported()const { return (ErrorCode::kNotSupported == code()); }

	bool IsInvalidArgument()const { return (ErrorCode::kInvalidArgument == code()); }

	bool IsMapedFailed()const { return (ErrorCode::kMapedFailed == code()); }

	bool IsShaderCompiledFailed()const { return (ErrorCode::kShaderCompiledFailed == code()); }

	bool IsCreatedFailed()const { return (ErrorCode::kCreatedFailed == ErrorCode()); }

	std::string ToString()const;

private:
	ErrorCode code()const {
		return (nullptr == state_) 
			? ErrorCode::kOk 
			: static_cast<ErrorCode>(state_[4]);
	}

	Status(ErrorCode error_code, const Slice& msg, const Slice& msg2);

	static const char* CopyState(const char* s);

private:
	// OK status has a NULL state_.  
	// Otherwise, state_ is a new[] array
	// of the following form:
	// state_[0..3] == length of message
	// state_[4]    == code
	// state_[5..]  == message
	const char *state_ = nullptr;
};

inline Status::Status(const Status& copy) {
	state_ = (nullptr == copy.state_) ? nullptr : CopyState(copy.state_);
}

inline Status& Status::operator=(const Status& copy) {
	if (state_ != copy.state_) {
		delete[] state_;
		state_ = ((copy.state_ == nullptr) ? nullptr : CopyState(copy.state_));
	}
	return *this;
}

#endif // !APPLICATION_HEADER_STATUS_H
