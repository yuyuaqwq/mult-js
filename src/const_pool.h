#pragma once

#include <vector>
#include <array>
#include <string>
#include <memory>
#include <mutex>

#include "value.h"

namespace mjs {

// ʹ�÷ֶξ�̬���飬����resizeʹ����������Context���̷߳���const
// ÿ�龲̬������1024��Ԫ�أ����˾�new��1������
class ConstPool {
private:
	static constexpr size_t kStaticArraySize = 1024;
	using StaticArray = std::array<Value, kStaticArraySize>;

public:
	uint32_t New(const Value& value);
	uint32_t New(Value&& value);

	const Value& Get(uint32_t index) const;
	Value& Get(uint32_t index);

private:
	std::mutex mutex_;
	std::map<Value, uint32_t> const_map_;
	std::array<std::unique_ptr<StaticArray>, kStaticArraySize> pool_;
	uint32_t const_index_ = 0;
};

} // namespace mjs