#pragma once

#include <string>

#include <mjs/object.h>
#include <mjs/value.h>

#include "instr.h"

namespace mjs {

// ���ں��������
// ���������ָ�����ķ�װ��ֻ�����ڳ�������庯�����ڳ����ش�������
// ���һ��ھֲ��������д������������ã�ָ�����壬�����﷨�ǵ��뷨
class FunctionBodyObject : public Object {
public:
	explicit FunctionBodyObject(uint32_t par_count) noexcept;
	std::string Disassembly();

public:
	uint32_t par_count;
	uint32_t var_count = 0;
	ByteCode byte_code;

	// ���õıհ�����

};

class FunctionRefObject : public Object {
public:
	explicit FunctionRefObject(FunctionBodyObject* func_body) noexcept;

public:
	FunctionBodyObject* func_body_;

	Value parent_func_;
	// ������ı����б�

};

} // namespace mjs