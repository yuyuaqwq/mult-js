#pragma once

#include <memory>

#include "noncopyable.h"
#include "const_pool.h"
#include "context.h"

namespace mjs {

// �����ء��ֽ���Ȳ��ɱ乲����Դλ��Runtime
class Runtime : noncopyable {
public:
	const ConstPool& const_pool() const { return const_pool_; }
	ConstPool& const_pool() { return const_pool_; }

	Stack& stack() {
		static thread_local auto stack = Stack(1024);
		return stack;
	}

private:
	ConstPool const_pool_;
};

} // namespace mjs