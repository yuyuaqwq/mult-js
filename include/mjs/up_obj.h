#pragma once

#include <mjs/object.h>

namespace mjs {

// upvalue���
// ָ��һ��Value�����Value������ջ�ϣ�Ҳ�����ڶ���
// �����ñ����ĺ���δ�˳�ʱ����ֱ������ջ��Value
// �����ñ����ĺ������˳�ʱ��˵����Ҫ�ӳ�����������(b������a�����ڱ����壬������b������ȫ�֣�a�Ѿ�������)
// ���ʱ����Ҫ���临�Ƶ�b��func ref��
// ���øú���ʱ���������ʱհ��������ͻ����func ref�д洢��

// �ܽ᣺
// func ref���λ�ڸ��������������У������õ���ջvalue
// func ref������ڸ���������������(�����س�ȥ�ӳ���)�������õ��Ƕ�value


// �ӳ��������ڣ�



//class FunctionBodyObject;
//class UpValueObject : public Object {
//public:
//	UpValueObject() noexcept;
//
//public:
//	
//};

} // namespace mjs