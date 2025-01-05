﻿#include <Windows.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <chrono>
#include <random>

#include "lexer.h"
#include "parser.h"
#include "exp.h"
#include "codegener.h"
#include "vm.h"


int main() {
    using namespace mjs;

    auto a = alloca(0x4096);



    std::fstream srcFile;
    srcFile.open(R"(test.js)");

    srcFile.seekg(0, std::ios::end);
    std::streampos length = srcFile.tellg();
    srcFile.seekg(0, std::ios::beg);
    std::vector<char> res(length);
    srcFile.read((char*)res.data(), length);

    Lexer lexer{ res.data() };
    Parser parser(&lexer);

    auto src = parser.ParseSource();

    auto const_pool = std::make_unique<ConstPool>();
    CodeGener cg(const_pool.get());

    cg.RegistryFunctionBridge("println",
        [](uint32_t parCount, StackFrame* stack) -> Value {
            for (int i = 0; i < parCount; i++) {
                auto val = stack->Pop();
                if (val.type() == ValueType::kString) {
                    std::cout << val.string_u8();
                }
                else if (val.type() == ValueType::kNumber) {
                    std::cout << val.number();
                }
                //else if (val.type() == ValueType::kU64) {
                //    std::cout << val.u64();
                //}
            }
            printf("\n");
            return Value();
        }
    );

    cg.RegistryFunctionBridge("tick",
        [](uint32_t par_count, StackFrame* stack) -> Value {
            return Value(double(GetTickCount64()));
        }
    );

    cg.Generate(src.get());

    VM vm(const_pool.get());

    std::cout << vm.Disassembly() << std::endl;

    //double n;
    //std::cin >> n;
    auto start = std::chrono::high_resolution_clock::now();

    vm.Run();

    //double k = 0;
    //while (k < 10000000) {
    //    k = k + n;
    //}

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "Elapsed time: " << elapsed.count() << " ms\n";

    printf("%d", 100);
}
