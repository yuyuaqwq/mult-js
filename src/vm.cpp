#include <mjs/vm.h>

#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/arr_obj.h>
#include <mjs/func_obj.h>

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

void Vm::EvalFunction(const Value& func_val) {
	pc_ = 0;
	cur_func_val_ = func_val;
	stack().resize(function_def(cur_func_val_)->var_count);

	FunctionLoadInit(&cur_func_val_);
	FunctionCallInit(cur_func_val_);

	Run();
}

FunctionDefObject* Vm::function_def(const Value& func_val) const {
	if (func_val.type() == ValueType::kFunction) {
		return func_val.function()->func_def_;
	}
	else if  (func_val.type() == ValueType::kFunctionDef) {
		return func_val.function_def();
	}
	return nullptr;
}

bool Vm::FunctionLoadInit(Value* func_val) {
	if (func_val->type() != ValueType::kFunctionDef) {
		return false;
	}
	auto func_def = func_val->function_def();
	if (func_def->closure_var_defs_.empty()) {
		return false;
	}

	*func_val = Value(new FunctionObject(func_def));

	auto func = func_val->function();
	// func->upvalues_.resize(func_def->closure_var_defs_.size());

	func->closure_value_arr_ = Value(new ArrayObject());
	auto& arr_obj = func->closure_value_arr_.object<ArrayObject>();
	arr_obj.mutale_values().resize(func_def->closure_var_defs_.size());

	return true;
}

void Vm::FunctionCallInit(const Value& func_val) {
	if (func_val.type() == ValueType::kFunction) {
		auto func = func_val.function();
		auto func_def = func->func_def_;

		// ���õ��Ǻ������󣬿�����Ҫ����հ��ڵ�upvalue
		auto& arr_obj = func->closure_value_arr_.object<ArrayObject>();
		for (auto& def : func_def->closure_var_defs_) {
			// ջ�ϵĶ���ͨ��upvalue�������հ�����
			stack_frame_.Set(def.first, Value(
				UpValue(&arr_obj.mutale_values()[def.second.arr_idx])
			));
		}
	}
}

Value& Vm::GetVar(uint32_t idx) {
	auto* var = &stack_frame_.Get(idx);
	while (var->type() == ValueType::kUpValue) {
		var = var->up_value().value;
	}
	return *var;
}

void Vm::SetVar(uint32_t idx, Value&& var) {
	auto* var_ = &stack_frame_.Get(idx);
	while (var_->type() == ValueType::kUpValue) {
		var_ = var_->up_value().value;
	}
	*var_ = std::move(var);
}

void Vm::LoadValue(const Value& value) {
	if (value.type() == ValueType::kFunctionDef) {
		// �������Ե�����һ��ָ��
		// �������������͵ĸ�ֵ���������closure_var_defs_����Ҫ�Զ�ת�ɺ�������
		auto func_val = value;
		if (FunctionLoadInit(&func_val)) {
			// array��Ҫ��ʼ��Ϊupvalue��ָ�򸸺�����ArrayValue
			// ���û�и������Ͳ���Ҫ�����Ƕ��㺯����Ĭ�ϳ�ʼ��Ϊδ����

			auto func = func_val.function();
			auto parent_func = cur_func_val_.function();

			// ���������������ü����������ӳ��������е�ArrayValue����������
			func->parent_function_ = cur_func_val_;

			// ���õ���������ArrayValue
			auto& arr_obj = func->closure_value_arr_.object<ArrayObject>().mutale_values();
			auto& parent_arr_obj = parent_func->closure_value_arr_.object<ArrayObject>().mutale_values();

			for (auto& def : func->func_def_->closure_var_defs_) {
				if (def.second.parent_var_idx == -1) {
					continue;
				}
				auto parent_arr_idx = parent_func->func_def_->closure_var_defs_[def.second.parent_var_idx].arr_idx;
				arr_obj[def.second.arr_idx] = Value(UpValue(&parent_arr_obj[parent_arr_idx]));
			}
		}
		stack_frame_.Push(func_val);
		return;
	}
	stack_frame_.Push(value);
}


void Vm::Run() {
	auto cur_func_def = function_def(cur_func_val_);
	if (!cur_func_def) return;

	do {
		// auto pc = pc_; std::cout << cur_func_def->byte_code.Disassembly(pc) << std::endl;
		auto opcode = cur_func_def->byte_code.GetOpcode(pc_++);
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
			auto const_idx = cur_func_def->byte_code.GetU8(pc_);
			pc_ += 1;
			LoadValue(const_pool().Get(const_idx));
			break;
		}
		case OpcodeType::kCLoadW: {
			auto const_idx = cur_func_def->byte_code.GetU16(pc_);
			pc_ += 2;
			LoadValue(const_pool().Get(const_idx));
			break;
		}
		case OpcodeType::kVLoad: {
			auto var_idx = cur_func_def->byte_code.GetU8(pc_);
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
			auto var_idx = cur_func_def->byte_code.GetU8(pc_);
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
			auto var_idx = cur_func_def->byte_code.GetU16(pc_);
			pc_ += 2;

			auto& func_val = GetVar(var_idx);
			auto par_count = stack_frame_.Pop().u64();

			FunctionDefObject* func_def = nullptr;
			switch (func_val.type()) {
			case ValueType::kFunction:
			case ValueType::kFunctionDef: {
				func_def = function_def(func_val);

				printf("%s\n", func_def->Disassembly().c_str());

				if (par_count < func_def->par_count) {
					throw VmException("Wrong number of parameters passed when calling the function");
				}

				auto save_func = cur_func_def;
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
				stack().Reduce(par_count - func_def->par_count);

				// �л�������ջ֡
				cur_func_def = func_def;
				cur_func_val_ = func_val;
				pc_ = 0;
				assert(stack().size() >= cur_func_def->par_count);
				stack_frame_.set_bottom(stack().size() - cur_func_def->par_count);

				// �����Ѿ���ջ�ˣ��ٷ���ֲ���������
				assert(cur_func_def->var_count >= cur_func_def->par_count);
				stack().Upgrade(cur_func_def->var_count - cur_func_def->par_count);

				// ���浱ǰ����������Ret����
				stack_frame_.Push(Value(save_func));
				stack_frame_.Push(Value(save_pc));
				stack_frame_.Push(Value(save_bottom));

				FunctionCallInit(func_val);

				break;
			}
			case ValueType::kFunctionBridge: {
				// �л�ջ֡
				auto old_bottom = stack_frame_.bottom();
				stack_frame_.set_bottom(stack().size() - par_count);

				auto ret = func_val.function_bridge()(par_count, &stack_frame_);

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
			// ��Ҫ���أ���Ҫ������ǰջ֡�ϵıհ�����������
			// �õ�����֮��ĺ������������丳ֵΪ

			auto ret_value = stack_frame_.Pop();

			auto save_bottom = stack_frame_.Pop();
			auto save_pc = stack_frame_.Pop();
			auto save_func = stack_frame_.Pop();

			// �ָ�������ջ֡
			cur_func_def = save_func.function_def();
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
				pc_ = cur_func_def->byte_code.CalcPc(--pc_);
			}
			else {
				pc_ += 2;
			}
			break;
		}
		case OpcodeType::kGoto: {
			pc_ = cur_func_def->byte_code.CalcPc(--pc_);
			break;
		}
		default:
			throw VmException("Unknown instruction");
		}
	} while (pc_ >= 0 && pc_ < cur_func_def->byte_code.Size());
}

} // namespace mjs