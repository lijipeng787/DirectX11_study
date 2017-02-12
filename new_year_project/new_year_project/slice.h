#pragma once

#ifndef APPLICATION_HEADER_SLICE_H

#define APPLICATION_HEADER_SLICE_H

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <string>

class Slice {
public:
	Slice() {}

	Slice(const char *data, size_t size) :data_(data), size_(size) {}

	Slice(const std::string& s) :data_(s.data()), size_(s.size()) {}

	Slice(const char *s) :data_(s), size_(strlen(s)) {}

	const char* get_data()const { return data_; }

	size_t get_size()const { return size_; }

	bool IsEmpty()const { return 0 == size_; }

	char operator[](size_t index)const { assert(index < get_size()); return data_[index]; }

	void Clear() { data_ = ""; size_ = 0; }

	void RemovePrefix(size_t n) { assert(n <= get_size()); data_ += n; size_ -= n; }

	std::string ToString()const { return std::string(data_, size_); }

	int Compare(const Slice& rhs)const;

	bool IsStartsWith(const Slice& slice)const {
		return ((size_ >= slice.size_) && (memcmp(data_, slice.data_, slice.size_) == 0));
	}

private:
	const char *data_ = nullptr;
	size_t size_ = 0;
};

inline bool operator==(const Slice& lhs, const Slice& rhs) {
	return ((lhs.get_size() == rhs.get_size()) && 
		(memcmp(lhs.get_data(), rhs.get_data(), rhs.get_size())));
}

inline bool operator!=(const Slice& lhs, const Slice& rhs) {
	return !(lhs == rhs);
}

inline int Slice::Compare(const Slice& rhs)const {
	const size_t min_len = (size_ < rhs.size_) ? size_ : rhs.size_;
	auto result = memcmp(data_, rhs.data_, min_len);

	if (0 == result) {
		if (size_ < rhs.size_)
			result = -1;
		else if (size_>rhs.size_)
			result = 1;
	}
	return result;
}

#endif // !APPLICATION_HEADER_SLICE_H
