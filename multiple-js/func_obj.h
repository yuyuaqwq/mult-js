#include "object.h"
#include "stack_frame.h"

namespace mjs {

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

} // namespace mjs