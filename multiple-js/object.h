#pragma once

#include "stack_frame.h"

namespace mjs {

class ObjectHeader {

};

// ���ں��������
// ���������ָ�����ķ�װ��ֻ�����ڳ�������庯�����ڳ����ش�������
// ���һ��ھֲ��������д���������ԭ�ͣ�ָ�����壬�����﷨�ǵ��뷨
class FunctionBodyObject : public ObjectHeader {
public:
	explicit FunctionBodyObject(uint32_t t_parCount) noexcept;
	std::string Disassembly();

public:
	uint32_t par_count;
	ByteCode byte_code;
	StackFrame stack_frame;
};


typedef Value(*FunctionBridgeCall)(uint32_t par_count, StackFrame* stack);
class FunctionBridgeObject : public ObjectHeader {
public:
	explicit FunctionBridgeObject(FunctionBridgeCall func_addr) noexcept;

	FunctionBridgeCall func_addr;
};

class FunctionProtoObject : public ObjectHeader {
public:
	explicit FunctionProtoObject(FunctionBodyObject* value) noexcept;
	explicit FunctionProtoObject(FunctionBridgeObject* value) noexcept;

	union {
		FunctionBodyObject* body_val;
		FunctionBridgeObject* bridge_val;
	};
};

class UpObject : public ObjectHeader {
public:
	UpObject(uint32_t t_index, FunctionBodyObject* func_proto) noexcept;

public:
	uint32_t index;
	FunctionBodyObject* func_proto;
};

} // namespace mjs