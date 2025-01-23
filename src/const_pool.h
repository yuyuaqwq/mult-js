#pragma once

#include <vector>
#include <string>
#include <memory>

#include "value.h"

namespace mjs {

// ��Ҫע����ǣ�poolӦ���޸�Ϊʹ�÷ֶξ�̬���飬����resizeʹ����������Context���̷߳���const
// ��ÿ�龲̬������1024��Ԫ�أ�����֮���newһ���¾�̬���飬�Żض�̬������
// ������������idx / 1024�ҵ���̬����������idx % 1024�ҵ���̬��������
class ConstPool {
public:
	uint32_t New(const Value& value);
	uint32_t New(Value&& value);

	// ������ʾ��β��������
	const Value& Get(int32_t index) const;
	size_t Size() const noexcept;

private:
	// ����
	std::map<Value, uint32_t> const_map_;

	std::vector<Value> pool_;
};

} // namespace mjs