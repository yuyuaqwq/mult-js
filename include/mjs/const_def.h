#pragma once

#include <cstdint>

namespace mjs {

// ��������0��ʼ����ʾȫ�ֳ���������
// ��������-1��ʼ����ʾ�ֲ�����������
using ConstIndex = int32_t;

inline bool IsGlobalConstIndex(ConstIndex idx) {
    return idx >= 0;
}

inline bool IsLocalConstIndex(ConstIndex idx) {
    return idx < 0;
}

} // namespace mjs 