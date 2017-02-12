#include "status.h"

std::string Status::ToString() const{
	if (nullptr == state_)
		return "OK";
	else {
		char tmp[30] = { 0 };
		const char *type;
		switch (code())
		{
		case ErrorCode::kCorruption:
			type = "Corruption";
			break;
		case ErrorCode::kCreatedFailed:
			type = "CreatedFailed";
			break;
		case ErrorCode::kInvalidArgument:
			type = "InvalidArgument";
			break;
		case ErrorCode::kIOError:
			type = "IOError";
			break;
		case ErrorCode::kMapedFailed:
			type = "MapedFailed";
			break;
		case ErrorCode::kNotFound:
			type = "NotFound";
			break;
		case ErrorCode::kNotSupported:
			type = "NotSupported";
			break;
		case ErrorCode::kShaderCompiledFailed:
			type = "ShaderCompiledFailed";
			break;
		case ErrorCode::kOk:
			type = "OK";
			break;
		default:
			snprintf(tmp, sizeof(tmp), "Unknow code (%d):", static_cast<int>(code()));
			type = tmp;
			break;
		}
		std::string result(type);
		uint32_t length = 0;
		memcpy(&length, state_, sizeof(length));
		result.append(state_ + 5, length);
		return result;
	}
}

Status::Status(ErrorCode error_code, const Slice & msg, const Slice & msg2){
	assert(ErrorCode::kOk != error_code);

	const uint32_t length1 = msg.get_size();
	const uint32_t length2 = msg2.get_size();
	// state_[0..3] is code's length
	const uint32_t code_size = length1 + (length2 ? length2 + 2 : 0);

	char *result = new char[code_size + 5];
	memcpy(result, &code_size, sizeof(code_size));
	result[4] = static_cast<char>(error_code);
	memcpy(result + 5, msg.get_data(), length1);
	if (length2) {
		result[5 + length1] = ':';
		result[5 + length1 + 1] = ' ';
		memcpy(result + 5 + length1 + 2, msg2.get_data(), length2);
	}
	state_ = result;
}

const char * Status::CopyState(const char * s){
	uint32_t code_size = 0;
	memcpy(&code_size, s, sizeof(code_size));
	char* result = new char[code_size + 5];
	memcpy(result, s, code_size + 5);
	return result;
}
