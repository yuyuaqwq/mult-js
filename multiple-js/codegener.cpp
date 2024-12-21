#include "codegener.h"


namespace mjs {

CodeGener::CodeGener(ValueSection* const_sect_)
	: const_sect_(const_sect_)
{
	const_sect_->Clear();
	const_sect_->Push(std::make_unique<FunctionBodyValue>(0));
	cur_func_ = const_sect_->Get(0)->GetFunctionBody();

	scope_.push_back(Scope{ cur_func_ });
}

void CodeGener::EntryScope(FunctionBodyValue* subFunc) {
	if (!subFunc) {
		// ��������������º���
		scope_.push_back(Scope{ cur_func_, scope_[scope_.size() - 1].var_count });
		return;
	}
	// ��������������º���
	scope_.push_back(Scope{ subFunc, 0 });
}

void CodeGener::ExitScope() {
	scope_.pop_back();
}

uint32_t CodeGener::AllocConst(std::unique_ptr<Value> value) {
	uint32_t constIdx;
	auto it = const_map_.find(value.get());
	if (it == const_map_.end()) {
		const_sect_->Push(std::move(value));
		constIdx = const_sect_->Size() - 1;
	}
	else {
		constIdx = it->second;
	}
	return constIdx;
}

uint32_t CodeGener::AllocVar(std::string var_name) {
	auto& varTable = scope_[scope_.size() - 1].var_table;
	if (varTable.find(var_name) != varTable.end()) {
		throw CodeGenerException("local var redefinition");
	}
	auto varIdx = scope_[scope_.size() - 1].var_count++;
	varTable.insert(std::make_pair(var_name, varIdx));
	return varIdx;
}

uint32_t CodeGener::GetVar(std::string var_name) {
	uint32_t varIdx = -1;
	// �ͽ��ұ���

	for (int i = scope_.size() - 1; i >= 0; i--) {
		auto& varTable = scope_[i].var_table;
		auto it = varTable.find(var_name);
		if (it != varTable.end()) {
			if (scope_[i].func == cur_func_) {
				varIdx = it->second;
			}
			else {
				// �����ⲿ�����ı�������Ҫ����Ϊ��ǰ��������upvalue����
				auto constIdx = AllocConst(std::make_unique<UpValue>(it->second, scope_[i].func));
				cur_func_->instr_sect.EmitPushK(constIdx);
				varIdx = AllocVar(var_name);
				cur_func_->instr_sect.EmitPopV(varIdx);
			}
			break;
		}
	}
	return varIdx;
}


void CodeGener::RegistryFunctionBridge(std::string func_name, FunctionBridgeCall funcAddr) {
	auto varIdx = AllocVar(func_name);
	auto constIdx = AllocConst(std::make_unique<FunctionBridgeValue>(funcAddr));


	// ���ɽ������ŵ��������еĴ���
	// ���������ִ��ʱȥ���أ���������ּ��صĳ����Ǻ����壬�ͻὫ����ԭ�͸����ֲ�����
	cur_func_->instr_sect.EmitPushK(constIdx);
	cur_func_->instr_sect.EmitPopV(varIdx);
}


void CodeGener::Generate(BlockStat* block, ValueSection* constSect) {

	for (auto& stat : block->stat_list) {
		GenerateStat(stat.get());
	}
	cur_func_->instr_sect.EmitStop();

}

void CodeGener::GenerateBlock(BlockStat* block) {
	EntryScope();
	for (auto& stat : block->stat_list) {
		GenerateStat(stat.get());
	}
	ExitScope();
}

void CodeGener::GenerateStat(Stat* stat) {
	switch (stat->GetType())
	{
	case StatType::kBlock: {
		GenerateBlock(static_cast<BlockStat*>(stat));
		break;
	}
	case StatType::kExp: {
		auto expStat = static_cast<ExpStat*>(stat)->exp.get();

		// ���������ʽ�������ս��
		if (expStat) {
			GenerateExp(expStat);
			cur_func_->instr_sect.EmitPop();
		}

		break;
	}
	case StatType::kFuncDef: {
		GenerateFuncDefStat(static_cast<FuncDefStat*>(stat));
		break;
	}
	case StatType::kReturn: {
		GenerateReturnStat(static_cast<ReturnStat*>(stat));
		break;
	}
	case StatType::kAssign: {
		GenerateAssignStat(static_cast<AssignStat*>(stat));
		break;
	}
	case StatType::kNewVar: {
		GenerateNewVarStat(static_cast<NewVarStat*>(stat));
		break;
	}
	case StatType::kIf: {
		GenerateIfStat(static_cast<IfStat*>(stat));
		break;
	}
	case StatType::kWhile: {
		GenerateWhileStat(static_cast<WhileStat*>(stat));
		break;
	}
	case StatType::kContinue: {
		GenerateContinueStat(static_cast<ContinueStat*>(stat));
		break;
	}
	case StatType::kBreak: {
		GenerateBreakStat(static_cast<BreakStat*>(stat));
		break;
	}

	default:
		throw CodeGenerException("Unknown statement type");
	}
}

void CodeGener::GenerateFuncDefStat(FuncDefStat* stat) {
	auto varIdx = AllocVar(stat->func_name);
	auto constIdx = AllocConst(std::make_unique<FunctionBodyValue>(stat->par_list.size()));

	// ���ɽ������ŵ��������еĴ���
	// ���������ִ��ʱȥ���أ���������ּ��صĳ����Ǻ����壬�ͻὫ����ԭ�͸����ֲ�����
	cur_func_->instr_sect.EmitPushK(constIdx);
	cur_func_->instr_sect.EmitPopV(varIdx);


	// ���滷������������ָ����
	auto savefunc = cur_func_;

	// �л�����
	EntryScope(const_sect_->Get(constIdx)->GetFunctionBody());
	cur_func_ = const_sect_->Get(constIdx)->GetFunctionBody();


	for (int i = 0; i < cur_func_->par_count; i++) {
		AllocVar(stat->par_list[i]);
	}

	auto block = stat->block.get();
	for (int i = 0; i < block->stat_list.size(); i++) {
		auto& stat = block->stat_list[i];
		GenerateStat(stat.get());
		if (i == block->stat_list.size() - 1) {
			if (stat->GetType() != StatType::kReturn) {
				// ��ȫĩβ��return
				cur_func_->instr_sect.EmitPushK(AllocConst(std::make_unique<NullValue>()));
				cur_func_->instr_sect.EmitRet();
			}
		}
	}

	// �ָ�����
	ExitScope();
	cur_func_ = savefunc;
}

void CodeGener::GenerateReturnStat(ReturnStat* stat) {
	if (stat->exp.get()) {
		GenerateExp(stat->exp.get());
	}
	else {
		cur_func_->instr_sect.EmitPushK(AllocConst(std::make_unique<NullValue>()));
	}
	cur_func_->instr_sect.EmitRet();
}

// Ϊ�˼����������ǰ����/����޸��ֲ���������������˲��ܷ��䵽ջ��
// ���������
	// �����ṩ����������

// ������ָ�����ı�������������ʱ���޷�ȷ���������������
// ԭ��
	// ����ʱ�޷���ǰ��֪call��λ�ã���call�����жദ��ÿ��callʱ���������״̬���ܶ���һ��
// ���������
	// Ϊÿ�������ṩһ���Լ��ı��������ŵ�������У�callʱ�л�������
	// �ڴ������ɹ����У���Ҫ��ȡ����ʱ���������ʹ�õı����ǵ�ǰ����֮����ⲿ������ģ��ͻ��ڳ������д���һ������Ϊupvalue�ı����������ص���ǰ�����ı�����
	// upvalue�洢���ⲿ������Body��ַ���Լ���Ӧ�ı�������


void CodeGener::GenerateNewVarStat(NewVarStat* stat) {
	auto varIdx = AllocVar(stat->var_name);
	GenerateExp(stat->exp.get());		// ���ɱ��ʽ����ָ����ս���ᵽջ��
	cur_func_->instr_sect.EmitPopV(varIdx);	// �������ֲ�������
}

void CodeGener::GenerateAssignStat(AssignStat* stat) {
	auto varIdx = GetVar(stat->var_name);
	if (varIdx == -1) {
		throw CodeGenerException("var not defined");
	}
	GenerateExp(stat->exp.get());		// ���ʽѹջ
	cur_func_->instr_sect.EmitPopV(varIdx);	// ������������
}

void CodeGener::GenerateIfStat(IfStat* stat) {
	GenerateExp(stat->exp.get());		// ���ʽ���ѹջ


	uint32_t ifPc = cur_func_->instr_sect.GetPc() + 1;		// ������һ��elif/else�޸�
	cur_func_->instr_sect.EmitJcf(0);		// ��ǰд������Ϊfalseʱ��ת��ָ��

	GenerateBlock(stat->block.get());



	// if

	// jcf end
	// ...
// end:
	// ...


	// if
	// else

	// jcf else
	// ...
	// jmp end
// else:
	// ...
// end:
	// ....


	// if 
	// elif
	// else

	// jcf elif
	// ...
	// jmp end
// elif:
	// jcf else
	// ...
	// jmp end
// else:
	// ...
// end:
	// ....


	// if 
	// elif
	// elif
	// else

	// jcf elif1
	// ...
	// jmp end
// elif1:
	// jcf elif2
	// ...
	// jmp end
// elif2:
	// jcf else
	// ...
	// jmp end
// else:
	// ...
// end:
	// ....

	std::vector<uint32_t> repairEndPcList;
	for (auto& elifStat : stat->else_if_stat_list) {
		repairEndPcList.push_back(cur_func_->instr_sect.GetPc() + 1);
		cur_func_->instr_sect.EmitJmp(0);		// ��ǰд����һ��֧�˳�if��֧�ṹ��jmp��ת

		// �޸�����Ϊfalseʱ����ת��if/elif��֮��ĵ�ַ
		*(uint32_t*)cur_func_->instr_sect.GetPtr(ifPc) = cur_func_->instr_sect.GetPc();

		GenerateExp(elifStat->exp.get());		// ���ʽ���ѹջ
		ifPc = cur_func_->instr_sect.GetPc() + 1;		// ������һ��elif/else�޸�
		cur_func_->instr_sect.EmitJcf(0);		// ��ǰд������Ϊfalseʱ��ת��ָ��

		GenerateBlock(elifStat->block.get());
	}

	if (stat->else_stat.get()) {
		repairEndPcList.push_back(cur_func_->instr_sect.GetPc() + 1);
		cur_func_->instr_sect.EmitJmp(0);		// ��ǰд����һ��֧�˳�if��֧�ṹ��jmp��ת

		// �޸�����Ϊfalseʱ����ת��if/elif��֮��ĵ�ַ
		*(uint32_t*)cur_func_->instr_sect.GetPtr(ifPc) = cur_func_->instr_sect.GetPc();

		GenerateBlock(stat->else_stat->block.get());
	}
	else {
		// �޸�����Ϊfalseʱ����ת��if/elif��֮��ĵ�ַ
		*(uint32_t*)cur_func_->instr_sect.GetPtr(ifPc) = cur_func_->instr_sect.GetPc();
	}

	// ����if��֧�ṹ�������޸������˳���֧�ṹ�ĵ�ַ
	for (auto repairEndPc : repairEndPcList) {
		*(uint32_t*)cur_func_->instr_sect.GetPtr(repairEndPc) = cur_func_->instr_sect.GetPc();
	}
}

void CodeGener::GenerateWhileStat(WhileStat* stat) {
	auto save_cur_loop_repair_end_pc_list = cur_loop_repair_end_pc_list_;
	auto save_cur_loop_start_pc = cur_loop_start_pc_;

	std::vector<uint32_t> loopRepairEndPcList;
	cur_loop_repair_end_pc_list_ = &loopRepairEndPcList;

	// ��¼����ѭ����pc
	uint32_t loop_start_pc = cur_func_->instr_sect.GetPc();
	cur_loop_start_pc_ = loop_start_pc;

	// ���ʽ���ѹջ
	GenerateExp(stat->exp.get());

	// �ȴ��޸�
	loopRepairEndPcList.push_back(cur_func_->instr_sect.GetPc() + 1);
	// ��ǰд������Ϊfalseʱ����ѭ����ָ��
	cur_func_->instr_sect.EmitJcf(0);

	GenerateBlock(stat->block.get());

	// ���»�ȥ���Ƿ���Ҫѭ��
	cur_func_->instr_sect.EmitJmp(loop_start_pc);

	for (auto repairEndPc : loopRepairEndPcList) {
		// �޸�����ѭ����ָ���pc
		*(uint32_t*)cur_func_->instr_sect.GetPtr(repairEndPc) = cur_func_->instr_sect.GetPc();
	}

	cur_loop_start_pc_ = save_cur_loop_start_pc;
	cur_loop_repair_end_pc_list_ = save_cur_loop_repair_end_pc_list;

}

void CodeGener::GenerateContinueStat(ContinueStat* stat) {
	if (cur_loop_repair_end_pc_list_ == nullptr) {
		throw CodeGenerException("Cannot use break in acyclic scope");
	}
	cur_func_->instr_sect.EmitJmp(cur_loop_start_pc_);		// ���ص�ǰѭ������ʼpc
}

void CodeGener::GenerateBreakStat(BreakStat* stat) {
	if (cur_loop_repair_end_pc_list_ == nullptr) {
		throw CodeGenerException("Cannot use break in acyclic scope");
	}
	cur_loop_repair_end_pc_list_->push_back(cur_func_->instr_sect.GetPc() + 1);
	cur_func_->instr_sect.EmitJmp(0);		// �޷���ǰ��֪����pc��������޸�pc���ȴ��޸�
}


void CodeGener::GenerateExp(Exp* exp) {
	switch (exp->GetType())
	{
	case ExpType::kNull: {
		auto constIdx = AllocConst(std::make_unique<NullValue>());
		cur_func_->instr_sect.EmitPushK(constIdx);
		break;
	}
	case ExpType::kBool: {
		auto boolexp = static_cast<BoolExp*>(exp);
		auto constIdx = AllocConst(std::make_unique<BoolValue>(boolexp->value));
		cur_func_->instr_sect.EmitPushK(constIdx);
		break;
	}
	case ExpType::kNumber: {
		auto numexp = static_cast<NumberExp*>(exp);
		auto constIdx = AllocConst(std::make_unique<NumberValue>(numexp->value));
		cur_func_->instr_sect.EmitPushK(constIdx);
		break;
	}
	case ExpType::kString: {
		auto strexp = static_cast<StringExp*>(exp);
		auto constIdx = AllocConst(std::make_unique<StringValue>(strexp->value));
		cur_func_->instr_sect.EmitPushK(constIdx);
		break;
	}
	case ExpType::kName: {
		// ��ȡ����ֵ�Ļ������ҵ���Ӧ�ı�����ţ�������ջ
		auto nameExp = static_cast<NameExp*>(exp);

		auto varIdx = GetVar(nameExp->name);
		if (varIdx == -1) {
			throw CodeGenerException("var not defined");
		}

		cur_func_->instr_sect.EmitPushV(varIdx);	// �ӱ����л�ȡ


		break;
	}
	case ExpType::kBinaOp: {
		auto binaOpExp = static_cast<BinaOpExp*>(exp);

		// ���ұ��ʽ��ֵ��ջ
		GenerateExp(binaOpExp->left_exp.get());
		GenerateExp(binaOpExp->right_exp.get());

		// ��������ָ��
		switch (binaOpExp->oper) {
		case TokenType::kOpAdd:
			cur_func_->instr_sect.EmitAdd();
			break;
		case TokenType::kOpSub:
			cur_func_->instr_sect.EmitSub();
			break;
		case TokenType::kOpMul:
			cur_func_->instr_sect.EmitMul();
			break;
		case TokenType::kOpDiv:
			cur_func_->instr_sect.EmitDiv();
			break;
		case TokenType::kOpNe:
			cur_func_->instr_sect.EmitNe();
			break;
		case TokenType::kOpEq:
			cur_func_->instr_sect.EmitEq();
			break;
		case TokenType::kOpLt:
			cur_func_->instr_sect.EmitLt();
			break;
		case TokenType::kOpLe:
			cur_func_->instr_sect.EmitLe();
			break;
		case TokenType::kOpGt:
			cur_func_->instr_sect.EmitGt();
			break;
		case TokenType::kOpGe:
			cur_func_->instr_sect.EmitGe();
			break;
		default:
			throw CodeGenerException("Unrecognized binary operator");
		}
		break;
	}
	case ExpType::kFunctionCall: {
		auto funcCallExp = static_cast<FunctionCallExp*>(exp);

		auto varIdx = GetVar(funcCallExp->name);
		if (varIdx == -1) {
			throw CodeGenerException("Function not defined");
		}

		//if (funcCallExp->par_list.size() < const_table_[]->GetFunctionBody()->par_count) {
		//	throw CodeGenerException("Wrong number of parameters passed during function call");
		//}

		for (int i = funcCallExp->par_list.size() - 1; i >= 0; i--) {
			// ����������ջ����call����ջ�в����ŵ���������
			GenerateExp(funcCallExp->par_list[i].get());
		}

		cur_func_->instr_sect.EmitPushK(AllocConst(std::make_unique<NumberValue>(funcCallExp->par_list.size())));

		// ����ԭ�ʹ���ڱ�������
		cur_func_->instr_sect.EmitCall(varIdx);
		break;
	}
	default:
		break;
	}
}


} // namespace codegener