#pragma once

#include <vector>
#include <array>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <optional>

#include <mjs/const_def.h>
#include <mjs/value.h>

namespace mjs {

// ʹ�÷ֶξ�̬���飬����resizeʹ����������Context���̷߳���const
// ÿ�龲̬������1024��Ԫ�أ����˾�new��1������
class GlobalConstPool {
private:
	static constexpr size_t kStaticArraySize = 1024;
	using StaticArray = std::array<Value, kStaticArraySize>;

public:
	GlobalConstPool();

	ConstIndex New(const Value& value);
	ConstIndex New(Value&& value);

	const Value& Get(ConstIndex index) const;
	Value& Get(ConstIndex index);

	std::optional<ConstIndex> Find(const Value& value);

private:
	std::mutex mutex_;
	std::map<Value, ConstIndex> const_map_;
	std::array<std::unique_ptr<StaticArray>, kStaticArraySize> pool_;
	uint32_t const_index_ = 1;
};

class LocalConstPool {
public:
	LocalConstPool();

	ConstIndex New(const Value& value);
	ConstIndex New(Value&& value);

	const Value& Get(ConstIndex index) const;
	Value& Get(ConstIndex index);

private:
	std::map<Value, ConstIndex> const_map_;
	std::vector<Value*> pool_;
};

} // namespace mjs