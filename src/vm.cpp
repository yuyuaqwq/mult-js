#include <mjs/vm.h>

#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/func_obj.h>
#include <mjs/up_obj.h>

#include "instr.h"

namespace mjs {

Vm::Vm(Context* context)
	: context_(context)
	, stack_frame_(&stack()) {}

const ConstPool& Vm::const_pool() const {
	return context_->runtime().const_pool();
}

Stack& Vm::stack() {
	return context_->runtime().stack();
}

void Vm::SetEvalFunction(const Value& func) {
	cur_func_ = func.function_body();
	stack().resize(cur_func_->var_count);
}

Value& Vm::GetVar(uint32_t idx) {
	 if (stack_frame_.Get(idx).type() == ValueType::kUpValue) {
		 auto up_value = stack_frame_.Get(idx).up_value();

		 up_value->func_body();

	 }
	return stack_frame_.Get(idx);
}

void Vm::SetVar(uint32_t idx, Value&& var) {
	 if (stack_frame_.Get(idx).type() == ValueType::kUpValue) {

	 }
	stack_frame_.Set(idx, std::move(var));
}

void Vm::LoadValue(const Value& value) {
	stack_frame_.Push(value);
}


void Vm::Run() {
	if (!cur_func_) return;
	do {
		// auto pc = pc_; std::cout << cur_func_->byte_code.Disassembly(pc) << std::endl;
		auto opcode = cur_func_->byte_code.GetOpcode(pc_++);
		switch (opcode) {
		//case OpcodeType::kStop:
		//	return;
		case OpcodeType::kCLoad_0: {
		case OpcodeType::kCLoad_1:
		case OpcodeType::kCLoad_2:
		case OpcodeType::kCLoad_3:
		case OpcodeType::kCLoad_4:
		case OpcodeType::kCLoad_5:
			auto const_idx = opcode - OpcodeType::kCLoad_0;
			LoadValue(const_pool().Get(const_idx));
			break;
		}
		case OpcodeType::kCLoad: {
			auto const_idx = cur_func_->byte_code.GetU8(pc_);
			pc_ += 1;
			LoadValue(const_pool().Get(const_idx));
			break;
		}
		case OpcodeType::kCLoadW: {
			auto const_idx = cur_func_->byte_code.GetU16(pc_);
			pc_ += 2;
			LoadValue(const_pool().Get(const_idx));
			break;
		}
		case OpcodeType::kVLoad: {
			auto var_idx = cur_func_->byte_code.GetU8(pc_);
			pc_ += 1;
			LoadValue(GetVar(var_idx));
			break;
		}
		case OpcodeType::kVLoad_0:
		case OpcodeType::kVLoad_1:
		case OpcodeType::kVLoad_2: 
		case OpcodeType::kVLoad_3: {
			auto var_idx = opcode - OpcodeType::kVLoad_0;
			stack_frame_.Push(GetVar(var_idx));
			break;
		}
		case OpcodeType::kPop: {
			stack_frame_.Pop();
			break;
		}
		case OpcodeType::kVStore: {
			auto var_idx = cur_func_->byte_code.GetU8(pc_);
			pc_ += 1;
			SetVar(var_idx, stack_frame_.Pop());
			break;
		}
		case OpcodeType::kVStore_0:
		case OpcodeType::kVStore_1:
		case OpcodeType::kVStore_2:
		case OpcodeType::kVStore_3: {
			auto var_idx = opcode - OpcodeType::kVStore_0;
			SetVar(var_idx, stack_frame_.Pop());
			break;
		}
		case OpcodeType::kAdd: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = b + a;
			break;
		}
		case OpcodeType::kSub: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = b - a;
			break;
		}
		case OpcodeType::kMul: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = b * a;
			break;
		}
		case OpcodeType::kDiv: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = b / a;
			break;
		}
		case OpcodeType::kNeg: {
			auto& a = stack_frame_.Get(-1);
			a = -a;
			break;
		}		 
		case OpcodeType::kInvokeStatic: {
			auto var_idx = cur_func_->byte_code.GetU16(pc_);
			pc_ += 2;

			auto& func = GetVar(var_idx);
			auto par_count = stack_frame_.Pop().u64();

			FunctionBodyObject* func_body = nullptr;
			switch (func.type()) {
			case ValueType::kFunctionBody: {
				func_body = func.function_body();
				// ��������㺯����û���������Ҳ��Ҫ����������
				if (!func_body->closure_value_idxs.empty()) {
					// ����ڲ����ڲ���ú����������Ӻ���
					func = Value(new FunctionRefObject(func_body));
					auto func_ref = func.function_ref();
					

				}
			}
			case ValueType::kFunctionRef: {
				if (func.type() == ValueType::kFunctionRef) {
					func_body = func.function_ref()->func_body_;
				}

				// std::cout << func_body->Disassembly();

				if (par_count < func_body->par_count) {
					throw VmException("Wrong number of parameters passed when calling the function");
				}

				auto save_func = cur_func_;
				auto save_pc = pc_;
				auto save_bottom = stack_frame_.bottom();
				
				// ջ֡���֣�
				// ������Ϣ
				// �ֲ�����
				// ����1
				// ...
				// ����n    <- bottom
				// ��һջ֡

				// �Ѷ���Ĳ�������
				stack().Reduce(par_count - func_body->par_count);

				// �л�������ջ֡
				cur_func_ = func_body;
				pc_ = 0;
				assert(stack().size() >= cur_func_->par_count);
				stack_frame_.set_bottom(stack().size() - cur_func_->par_count);

				// �����Ѿ���ջ�ˣ��ٷ���ֲ���������
				assert(cur_func_->var_count >= cur_func_->par_count);
				stack().Upgrade(cur_func_->var_count - cur_func_->par_count);

				// ���浱ǰ����������Ret����
				stack_frame_.Push(Value(save_func));
				stack_frame_.Push(Value(save_pc));
				stack_frame_.Push(Value(save_bottom));

				break;
			}
			case ValueType::kFunctionBridge: {
				// �л�ջ֡
				auto old_bottom = stack_frame_.bottom();
				stack_frame_.set_bottom(stack().size() - par_count);

				auto ret = func.function_bridge()(par_count, &stack_frame_);

				// ��ԭջ֡
				stack_frame_.set_bottom(old_bottom);
				stack().Reduce(par_count);
				stack_frame_.Push(std::move(ret));
				break;
			}
			default:
				throw VmException("Non callable types.");
			}
			break;
		}
		case OpcodeType::kReturn: {
			auto ret_value = stack_frame_.Pop();

			auto save_bottom = stack_frame_.Pop();
			auto save_pc = stack_frame_.Pop();
			auto save_func = stack_frame_.Pop();

			// �ָ�������ջ֡
			cur_func_ = save_func.function_body();
			pc_ = save_pc.u64();

			// ����ջ����ջ��
			stack().resize(stack_frame_.bottom());
			stack_frame_.set_bottom(save_bottom.u64());

			// ����λ��ԭ��λ��ջ֡ջ���ķ���ֵ
			stack_frame_.Push(ret_value);
			break;
		}
		case OpcodeType::kNe: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = Value(!(b == a));
			break;
		}
		case OpcodeType::kEq: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = Value(b == a);
			break;
		}
		case OpcodeType::kLt: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = Value(b < a);
			break;
		}
		case OpcodeType::kLe: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = Value(!(b > a));
			break;
		}
		case OpcodeType::kGt: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = Value(b > a);
			break;
		}
		case OpcodeType::kGe: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = Value(!(b < a));
			break;
		}
		case OpcodeType::kIfEq: {
			auto boolean = stack_frame_.Pop().boolean();
			if (boolean == false) {
				pc_ = cur_func_->byte_code.CalcPc(--pc_);
			}
			else {
				pc_ += 2;
			}
			break;
		}
		case OpcodeType::kGoto: {
			pc_ = cur_func_->byte_code.CalcPc(--pc_);
			break;
		}
		default:
			throw VmException("Unknown instruction");
		}
	} while (pc_ >= 0 && pc_ < cur_func_->byte_code.Size());
}

} // namespace mjs