#pragma once

#include <vector>
#include <string>
#include <memory>

#include <mjs/noncopyable.h>
#include <mjs/value.h>

namespace mjs {

// StackFrame��
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

	// ������ʾ��ջ֡������������0��ʼ
	// ������ʾ��ջ֡������������-1��ʼ
	Value& Get(int32_t index);
	void Set(int32_t index, const Value& value);
	void Set(int32_t index, Value&& value);

	uint32_t bottom() const { return bottom_; }
	void set_bottom(uint32_t bottom) { bottom_ = bottom; }

private:
	Stack* stack_;
	uint32_t bottom_ = 0;	// ��ǰջ֡��ջ��
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

	void Upgrade(uint32_t size);
	void Reduce(uint32_t size);

	size_t size()  const noexcept;
	void resize(size_t size);

private:
	std::vector<Value> stack_;
};


} // namespace mjs