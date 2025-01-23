#pragma once

#include <vector>
#include <string>
#include <memory>

#include "noncopyable.h"
#include "value.h"

#include <cstdlib>

namespace mjs {

// StackFrame �Ż���
// ʹ��tls��Ϊÿ���߳�ָ��һ���ڴ�(��1M)
// ��ǰStackFrame��������ֱ������������ڴ���
// ����ʱ����Ϊ�ɵ�StackFrame��������
// �µ�StackFrame��ָ�룬ֱ��ָ����ڴ��δʹ�ò���(�����Ǿɵ�StackFrame)��Ȼ��ʹ���µ�StackFrame����
class Stack;
class StackFrame {
public:
	StackFrame(Stack* stack)
		: stack_(stack){}

	void Push(const Value& value);
	void Push(Value&& value);
	Value Pop();

	// ������ʾ��ջ������������0��ʼ
	// ������ʾ��ջ������������-1��ʼ
	Value& Get(int32_t index);
	void Set(int32_t index, const Value& value);
	void Set(int32_t index, Value&& value);

	uint32_t offset() const { return offset_; }
	void set_offset(uint32_t offset) { offset_ = offset; }

private:
	Stack* stack_;
	uint32_t offset_ = 0;	// ��ǰջ֡��ջ������
};

// ÿ���̶̹߳���ջ
class Stack : noncopyable {
public:
	Stack(size_t count) {
		stack_.reserve(count);
	}

	void Push(const Value& value);
	void Push(Value&& value);
	Value Pop();

	Value& Get(int32_t index);
	void Set(int32_t index, const Value& value);
	void Set(int32_t index, Value&& value);

	size_t Size()  const noexcept;
	void ReSize(size_t size);
	void Upgrade(uint32_t size);
	void Reduce(uint32_t size);

private:
	std::vector<Value> stack_;
};


} // namespace mjs