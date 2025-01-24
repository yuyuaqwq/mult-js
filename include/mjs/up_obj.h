#pragma once

#include <mjs/object.h>
#include <mjs/func_obj.h>

namespace mjs {

class UpValueObject : public Object {
public:
	UpValueObject(FunctionBodyObject* func_body, uint32_t closure_value_idx) noexcept
		: func_body(func_body)
		, closure_value_idx(closure_value_idx) {}

public:
	FunctionBodyObject* func_body;
	uint32_t closure_value_idx;
};

// �հ�ֵ����
// ����UpValueʱ��ʵ������ClosureValueRef��ջ
// ����·����
// func ref -> closure_values_[closure_value_idxs_[closure_value_idx]]

// ��ô�õ����func ref��
// func refҲ��һ��parent����
// ͨ����ǰfunc ref��ѭ��������func ref��ÿ��һ���ͱȽ�һ��func body��ƥ�����ʾ�������func ref

// func ref��ôά����
// ÿ�κ������þͰѵ�ǰfunc ref��ֵΪparent��

class ClosureValueRefObject : public Object {
private:

public:
	Value* value_ref;
};


} // namespace mjs