#pragma once

#include <memory>

#include <mjs/noncopyable.h>
#include <mjs/vm.h>

namespace mjs {

class Runtime;
class Context : noncopyable {
public:
	Context(Runtime* runtime)
		: runtime_(runtime)
		, vm_(this) {}

	void Eval(std::string_view script);

	void Gc() {
		// 第一趟将孩子解引用为0的挂入tmp，因为该孩子节点只被当前节点引用
		// 如果当前节点可以被回收，那么该孩子就肯定也要被回收

		// 第二趟扫的时候，再将当前链表中的节点指向的孩子挂回来，因为当前节点不是垃圾，其孩子自然也不是垃圾
		// 如果没有被挂回链表的节点，那就是垃圾了，没有被根节点自下的路径引用
	}

	Runtime& runtime() const { return *runtime_; }
	// LocalConstPool& const_pool() { return local_const_pool_; }

private:
	Runtime* runtime_;

	// LocalConstPool local_const_pool_;

	Vm vm_;
};

} // namespace mjs