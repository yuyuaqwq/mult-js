#include "const_pool.h"

namespace mjs {

void ConstPool::Push(const Value& value) {
	pool_.emplace_back(value);
}

void ConstPool::Push(Value&& value) {
	pool_.emplace_back(std::move(value));
}

// ������ʾ��β��������
Value& ConstPool::Get(int32_t index) {
	if (index >= 0) {
		return pool_[index];
	}
	else {
		return pool_[pool_.size() + index];
	}
}

size_t ConstPool::Size() const noexcept {
	return pool_.size();
}

} // namespace mjs