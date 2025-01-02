#pragma once

#include "stack_frame.h"

namespace mjs {

class Object {
public:
	Object() {
		tag_.full_ = 0;
		tag_.ref_ = 1;
	}


private:
	union {
		uint64_t full_;
		uint32_t ref_;
	} tag_;
};

// ���ں��������
// ���������ָ�����ķ�װ��ֻ�����ڳ�������庯�����ڳ����ش�������
// ���һ��ھֲ��������д���������ԭ�ͣ�ָ�����壬�����﷨�ǵ��뷨
class FunctionBodyObject : public Object {
public:
	explicit FunctionBodyObject(uint32_t t_parCount) noexcept;
	std::string Disassembly();

public:
	uint32_t par_count;
	ByteCode byte_code;
	StackFrame stack_frame;
};

class UpValueObject : public Object {
public:
	UpValueObject(uint32_t t_index, FunctionBodyObject* func_body) noexcept;

public:
	uint32_t index;
	FunctionBodyObject* func_body;
};

} // namespace mjs