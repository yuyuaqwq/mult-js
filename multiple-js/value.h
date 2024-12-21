#pragma once

#include <string>
#include <memory>

#include "instr.h"

#include "section.h"

namespace mjs {

enum class ValueType {
	kNull = 0,
	kBool,
	kNumber,
	kString,
	kFunctionProto,
	kFunctionBody,
	kFunctionBridge,
	kUp,
};

class NullValue;
class BoolValue;
class NumberValue;
class StringValue;
class FunctionProtoValue;
class FunctionBodyValue;
class FunctionBridgeValue;
class UpValue;

class Value {
public:
	virtual ValueType GetType() const noexcept = 0;
	virtual std::unique_ptr<Value> Copy() const = 0;

	bool operator<(const Value& value) const;
	bool operator<=(const Value& value) const;
	bool operator==(const Value& value) const;

	BoolValue* GetBool() noexcept;
	NumberValue* GetNumber() noexcept;
	StringValue* GetString() noexcept;
	FunctionProtoValue* GetFunctionProto() noexcept;
	FunctionBodyValue* GetFunctionBody() noexcept;
	FunctionBridgeValue* GetFunctionBirdge() noexcept;
	UpValue* GetUp() noexcept;

};

class NullValue :public Value {
public:
	virtual ValueType GetType() const noexcept;
	virtual std::unique_ptr<Value> Copy() const;
};

class BoolValue :public Value {
public:
	virtual ValueType GetType() const noexcept;
	virtual std::unique_ptr<Value> Copy() const;
	explicit BoolValue(bool t_value) noexcept;

	bool val;
};

class NumberValue :public Value {
public:
	virtual ValueType GetType() const noexcept;
	virtual std::unique_ptr<Value> Copy() const;
	explicit NumberValue(uint64_t t_value) noexcept;

	uint64_t val;
};

class StringValue :public Value {
public:
	virtual ValueType GetType() const noexcept;
	virtual std::unique_ptr<Value> Copy() const;
	explicit StringValue(const std::string& t_value);

	std::string val;
};

// ���ں��������
// ���������ָ�����ķ�װ��ֻ�����ڳ�������庯�����ڳ�������������
// ���һ��ھֲ��������д���������ԭ�ͣ�ָ�����壬�����﷨�ǵ��뷨
class FunctionBodyValue : public Value {
public:
	virtual ValueType GetType() const noexcept;
	virtual std::unique_ptr<Value> Copy() const;
	explicit FunctionBodyValue(uint32_t t_parCount) noexcept;
	std::string Disassembly();

public:
	uint32_t par_count;
	InstrSection instr_sect;
	ValueSection var_sect;
};


typedef std::unique_ptr<Value>(*FunctionBridgeCall)(uint32_t parCount, ValueSection* stack);
class FunctionBridgeValue : public Value {
public:
	virtual ValueType GetType() const noexcept;
	std::unique_ptr<Value> Copy() const;
	explicit FunctionBridgeValue(FunctionBridgeCall t_funcAddr) noexcept;

	FunctionBridgeCall func_addr;
};

class FunctionProtoValue : public Value {
public:
	virtual ValueType GetType() const noexcept;
	virtual std::unique_ptr<Value> Copy() const;
	explicit FunctionProtoValue(FunctionBodyValue* value) noexcept;
	explicit FunctionProtoValue(FunctionBridgeValue* value) noexcept;

	union {
		Value* val;
		FunctionBodyValue* body_val;
		FunctionBridgeValue* bridge_val;
	};
};

class UpValue : public Value {
public:
	virtual ValueType GetType() const noexcept;
	virtual std::unique_ptr<Value> Copy() const;
	UpValue(uint32_t t_index, FunctionBodyValue* func_proto) noexcept;

public:
	uint32_t index;
	FunctionBodyValue* func_proto;
};

} // namespace mjs