#include "vm.h"

namespace mjs {

VM::VM(ValueSection* const_sect) :
	const_sect_{ const_sect } {
	cur_func_ = const_sect_->Get(0)->GetFunctionBody();
}

std::string VM::Disassembly() {
	return cur_func_->Disassembly();
}

Value* VM::GetVar(uint32_t idx) {
	if (cur_func_->var_sect.Get(idx)->GetType() == ValueType::kUp) {
		// upvalue�����γ�����(�������ɽ׶Σ����ݴ���������������ҵ��˱��������Ǹñ���ʵ����Ҳ��upvalue)�����Ҫ�ظ�������ֱ������upvalue
		// ��ʱ����ԴӴ��������Ǳ��Ż���Ҳ����ѭ�������ң�ֱ������ָ��upvalue
		auto func = cur_func_;
		auto upvalue = func->var_sect.Get(idx)->GetUp();
		while (upvalue->funcProto->var_sect.Get(upvalue->index)->GetType() == ValueType::kUp) {
			func = upvalue->funcProto;
			upvalue = func->var_sect.Get(upvalue->index)->GetUp();
		}
		return upvalue->funcProto->var_sect.Get(upvalue->index).get();
	}
	return cur_func_->var_sect.Get(idx).get();
}

std::unique_ptr<Value> VM::GetVarCopy(uint32_t idx) {
	return GetVar(idx)->Copy();
}

void VM::SetVar(uint32_t idx, std::unique_ptr<Value> var) {
	if (idx >= cur_func_->var_sect.Size()) {
		cur_func_->var_sect.ReSize(idx + 1);
	}

	else if (cur_func_->var_sect.Get(idx).get() && cur_func_->var_sect.Get(idx)->GetType() == ValueType::kUp) {
		auto func = cur_func_;
		auto upvalue = func->var_sect.Get(idx)->GetUp();
		while (upvalue->funcProto->var_sect.Get(upvalue->index)->GetType() == ValueType::kUp) {
			func = upvalue->funcProto;
			upvalue = func->var_sect.Get(upvalue->index)->GetUp();
		}
		upvalue->funcProto->var_sect.Set(upvalue->index, std::move(var));
		return;
	}

	cur_func_->var_sect.Set(idx, std::move(var));
}

void VM::SetVar(uint32_t idx, Value* var) {
	SetVar(idx, var->Copy());
}


void VM::Run() {
	do {
		auto opcode = cur_func_->instr_sect.GetOpcode(pc_++);
		switch (opcode) {
		case OpcodeType::kStop:
			return;
		case OpcodeType::kNop:
			break;
		case OpcodeType::kPushK: {
			auto constIdx = cur_func_->instr_sect.GetU32(pc_);
			pc_ += 4;
			stack_sect_.Push(const_sect_->Get(constIdx)->Copy());
			break;
		}
		case OpcodeType::kPushV: {
			auto varIdx = cur_func_->instr_sect.GetU32(pc_);
			pc_ += 4;
			stack_sect_.Push(GetVarCopy(varIdx));
			break;
		}
		case OpcodeType::kPop: {
			stack_sect_.Pop();
			break;
		}
		case OpcodeType::kPopV: {
			auto varIdx = cur_func_->instr_sect.GetU32(pc_);
			pc_ += 4;
			SetVar(varIdx, stack_sect_.Pop());
			break;
		}
		case OpcodeType::kAdd: {
			auto a = stack_sect_.Pop();
			auto& b = stack_sect_.Get(-1);
			b->GetNumber()->val += a->GetNumber()->val;
			break;
		}
		case OpcodeType::kSub: {
			auto a = stack_sect_.Pop();
			auto& b = stack_sect_.Get(-1);
			b->GetNumber()->val -= a->GetNumber()->val;
			break;
		}
		case OpcodeType::kMul: {
			auto a = stack_sect_.Pop();
			auto& b = stack_sect_.Get(-1);
			b->GetNumber()->val *= a->GetNumber()->val;
			break;
		}
		case OpcodeType::kDiv: {
			auto a = stack_sect_.Pop();
			auto& b = stack_sect_.Get(-1);
			b->GetNumber()->val /= a->GetNumber()->val;
			break;
		}
		case OpcodeType::kCall: {
			auto varIdx = cur_func_->instr_sect.GetU32(pc_);
			pc_ += 4;

			auto value = GetVar(varIdx)->GetFunctionProto()->val;

			auto par_count = stack_sect_.Pop()->GetBool()->val;

			if (value->GetType() == ValueType::kFunctionBody) {

				auto callFunc = value->GetFunctionBody();

				//printf("%s\n", callFunc->Disassembly().c_str());

				if (par_count < callFunc->par_count) {
					throw VMException("Wrong number of parameters passed when calling the function");
				}

				auto saveFunc = cur_func_;
				auto savePc = pc_;

				// �л�����
				cur_func_ = callFunc;
				pc_ = 0;

				// �ƶ�ջ�ϵĲ������º����ľֲ�������
				//m_varIdxBase = m_var_sect.size();
				for (int i = cur_func_->par_count - 1; i >= 0; i--) {
					SetVar(i, std::move(stack_sect_.Pop()));
				}

				// ���浱ǰ����
				stack_sect_.Push(std::make_unique<NumberValue>((uint64_t)saveFunc));
				stack_sect_.Push(std::make_unique<NumberValue>(savePc));

			}
			else if (value->GetType() == ValueType::kFunctionBridge) {
				auto call_func = value->GetFunctionBirdge();
				stack_sect_.Push(call_func->func_addr(par_count, &stack_sect_));
			}
			else {
				throw VMException("Wrong call type");
			}
			break;
		}
		case OpcodeType::kRet: {
			auto retValue = stack_sect_.Pop();
			auto pc = stack_sect_.Pop();
			auto& curFunc = stack_sect_.Get(-1);

			// �ָ�����
			cur_func_ = (FunctionBodyValue*)curFunc->GetNumber()->val;
			pc_ = pc->GetNumber()->val;

			curFunc = std::move(retValue);
			break;
		}
		case OpcodeType::kNe: {
			auto a = stack_sect_.Pop();
			auto& b = stack_sect_.Get(-1);
			bool res = !(*b == *a);
			b = std::move(std::make_unique<BoolValue>(res));
			break;
		}
		case OpcodeType::kEq: {
			auto a = stack_sect_.Pop();
			auto& b = stack_sect_.Get(-1);
			bool res = *b == *a;
			b = std::move(std::make_unique<BoolValue>(res));
			break;
		}
		case OpcodeType::kLt: {
			auto a = stack_sect_.Pop();
			auto& b = stack_sect_.Get(-1);
			bool res = *b < *a;
			b = std::move(std::make_unique<BoolValue>(res));
			break;
		}
		case OpcodeType::kLe: {
			auto a = stack_sect_.Pop();
			auto& b = stack_sect_.Get(-1);
			bool res = *b <= *a;
			b = std::move(std::make_unique<BoolValue>(res));
			break;
		}
		case OpcodeType::kGt: {
			auto a = stack_sect_.Pop();
			auto& b = stack_sect_.Get(-1);
			bool res = !(*b <= *a);
			b = std::move(std::make_unique<BoolValue>(res));
			break;
		}
		case OpcodeType::kGe: {
			auto a = stack_sect_.Pop();
			auto& b = stack_sect_.Get(-1);
			bool res = !(*b < *a);
			b = std::move(std::make_unique<BoolValue>(res));
			break;
		}
		case OpcodeType::kJcf: {
			auto newPc = cur_func_->instr_sect.GetU32(pc_);
			pc_ += 4;
			auto boolValue = stack_sect_.Pop()->GetBool()->val;
			if (boolValue == false) {
				pc_ = newPc;
			}
			break;
		}
		case OpcodeType::kJmp: {
			auto newPc = cur_func_->instr_sect.GetU32(pc_);
			pc_ = newPc;
			break;
		}
		default:
			throw VMException("Unknown instruction");
		}
	} while (true);
}

} // namespace mjs