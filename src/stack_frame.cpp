#include "stack_frame.h"

#include "value.h"

namespace mjs {

void Stack::Push(const Value& value) {
	stack_.emplace_back(value);
}

void Stack::Push(Value&& value) {
	stack_.emplace_back(std::move(value));
}

Value Stack::Pop() {
	auto value = std::move(stack_.back());
	stack_.pop_back();
	return value;
}

Value& Stack::Get(int32_t index) {
	return stack_[index];
}

void Stack::Set(int32_t index, const Value& value) {
	stack_[index] = value;
}

void Stack::Set(int32_t index, Value&& value) {
	stack_[index] = std::move(value);
}

size_t Stack::Size() const noexcept {
	return stack_.size();
}

void Stack::ReSize(size_t size) {
	stack_.resize(size);
}

void Stack::Upgrade(uint32_t size) {
	stack_.resize(stack_.size() + size);
}

void Stack::Reduce(uint32_t size) {
	stack_.resize(stack_.size() - size);
}




void StackFrame::Push(const Value& value) {
	stack_->Push(value);
}

void StackFrame::Push(Value&& value) {
	stack_->Push(std::move(value));
}

Value StackFrame::Pop() {
	return stack_->Pop();
}

// ������ʾ��ջ����������
Value& StackFrame::Get(int32_t index) {
	if (index >= 0) {
		return stack_->Get(offset_ + index);
	}
	else {
		return stack_->Get(stack_->Size() + index);
	}
}

void StackFrame::Set(int32_t index, const Value& value) {
	auto value_ = value;
	Set(index, std::move(value_));
}

void StackFrame::Set(int32_t index, Value&& value) {
	if (index >= 0) {
		stack_->Set(offset_ + index, value);
	}
	else {
		stack_->Set(stack_->Size() + index, value);
	}
}

} // namespace mjs