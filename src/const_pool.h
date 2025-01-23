#pragma once

#include <vector>
#include <array>
#include <string>
#include <memory>
#include <mutex>

#include "value.h"

namespace mjs {

// poolʹ�÷ֶξ�̬���飬����resizeʹ����������Context���̷߳���const
// ��ÿ�龲̬������1024��Ԫ�أ�2����������֮���newһ���¾�̬���飬�Ż�1��������
// ������������idx / 1024�ҵ�1������������idx % 1024�ҵ�2����������
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