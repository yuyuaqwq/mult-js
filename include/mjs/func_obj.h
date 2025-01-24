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
	explicit FunctionBodyObject(FunctionBodyObject* parent, uint32_t par_count) noexcept;
	std::string Disassembly();

public:
	FunctionBodyObject* parent;

	uint32_t par_count;
	uint32_t var_count = 0;
	ByteCode byte_code;

	// ��¼���ⲿ�����������в���ıհ�����
	struct ClosureInfo {
		uint32_t var_idx;
	};
	std::vector<ClosureInfo> closure_infos_;
};

class FunctionRefObject : public Object {
public:
	explicit FunctionRefObject(FunctionBodyObject* func_body) noexcept;

public:
	FunctionBodyObject* func_body_;
};

} // namespace mjs