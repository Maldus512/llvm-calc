#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "codegen.h"
#include "parser.h"
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>


typedef struct {
    LLVMModuleRef  module;
    LLVMTypeRef    void_type;
    LLVMTypeRef    int32_type;
    LLVMTypeRef    ptr_type;
    LLVMValueRef   int32_zero;
    LLVMBuilderRef builder;

    const char   bindings[CONFIG_MAX_WITH_VARIABLES][64];
    LLVMValueRef calls[CONFIG_MAX_WITH_VARIABLES];
} codegen_t;


static LLVMValueRef ir_generation(codegen_t *codegen, parser_ast_t *ast);
static void         run(codegen_t *codegen, parser_ast_t *ast);


void codegen_compile(parser_ast_t *ast) {
    LLVMContextRef context = LLVMContextCreate();
    LLVMModuleRef  module  = LLVMModuleCreateWithNameInContext("calc.expr", context);

    LLVMTypeRef int32_type = LLVMInt32TypeInContext(context);
    codegen_t   codegen    = {
             .module     = module,
             .void_type  = LLVMVoidTypeInContext(context),
             .int32_type = int32_type,
             .ptr_type   = LLVMPointerTypeInContext(context, 0),
             .int32_zero = LLVMConstInt(int32_type, 0, 1),
             .builder    = LLVMCreateBuilderInContext(context),
    };

    run(&codegen, ast);

    char *ir = LLVMPrintModuleToString(module);
    LLVMDisposeBuilder(codegen.builder);
    LLVMDisposeModule(module);

    printf("%s\n", ir);

    // Cleanup
    LLVMDisposeMessage(ir);
}


static void run(codegen_t *codegen, parser_ast_t *ast) {
    {
        LLVMTypeRef  param_types[] = {codegen->int32_type, codegen->ptr_type};
        LLVMValueRef function =
            LLVMAddFunction(codegen->module, "main", LLVMFunctionType(codegen->int32_type, param_types, 2, 0));

        LLVMBasicBlockRef basic_block = LLVMAppendBasicBlock(function, "entry");
        LLVMPositionBuilderAtEnd(codegen->builder, basic_block);
    }

    LLVMValueRef value = ir_generation(codegen, ast);

    {
        LLVMTypeRef  param_types[] = {codegen->int32_type};
        LLVMTypeRef  function_type = LLVMFunctionType(codegen->void_type, param_types, 1, 0);
        LLVMValueRef function      = LLVMAddFunction(codegen->module, "calc_write", function_type);
        LLVMValueRef args[]        = {value};

        LLVMBuildCall2(codegen->builder, function_type, function, args, 1, "");
        LLVMBuildRet(codegen->builder, codegen->int32_zero);
    }
}


static LLVMValueRef ir_generation(codegen_t *codegen, parser_ast_t *ast) {
    switch (ast->tag) {
        case PARSER_AST_TAG_WITH_DECLARATION: {
            LLVMTypeRef  param_types[] = {codegen->ptr_type};
            LLVMTypeRef  function_type = LLVMFunctionType(codegen->int32_type, param_types, 1, 0);
            LLVMValueRef function      = LLVMAddFunction(codegen->module, "calc_read", function_type);

            for (size_t i = 0; i < CONFIG_MAX_WITH_VARIABLES; i++) {
                if (ast->as.with_declaration.identifiers[i] == NULL) {
                    break;
                } else {
                    parser_copy_string_from_source_reference((char *)codegen->bindings[i],
                                                             ast->as.with_declaration.identifiers[i]);

                    LLVMValueRef string_text = LLVMConstStringInContext2(
                        LLVMGetModuleContext(codegen->module), codegen->bindings[i], strlen(codegen->bindings[i]), 0);

                    char name[CONFIG_MAX_VARIABLE_NAME_LENGTH] = {0};
                    snprintf(name, sizeof(name), "%s.str", name);
                    LLVMValueRef string = LLVMAddGlobal(codegen->module, LLVMTypeOf(string_text), name);
                    LLVMSetInitializer(string, string_text);
                    LLVMSetGlobalConstant(string, 1);
                    LLVMSetLinkage(string, LLVMPrivateLinkage);

                    codegen->calls[i] = LLVMBuildCall2(codegen->builder, function_type, function, &string, 1, "");
                }
            }

            return ir_generation(codegen, ast->as.with_declaration.expr);
        }

        case PARSER_AST_TAG_IDENTIFIER: {
            for (size_t i = 0; i < CONFIG_MAX_WITH_VARIABLES; i++) {
                if (parser_compare_string_in_source_reference((char *)codegen->bindings[i], ast->source_reference)) {
                    return codegen->calls[i];
                }
            }
            assert(0);
        }

        case PARSER_AST_TAG_NUMBER: {
            int value = parser_get_int_value_from_source_reference(ast->source_reference);
            return LLVMConstInt(codegen->int32_type, value, 0);
        }

        case PARSER_AST_TAG_BINARY_OPERATION: {
            LLVMValueRef left_value  = ir_generation(codegen, ast->as.binary_operation.left);
            LLVMValueRef right_value = ir_generation(codegen, ast->as.binary_operation.right);

            switch (ast->as.binary_operation.op) {
                case PARSER_OPERATOR_PLUS:
                    return LLVMBuildAdd(codegen->builder, left_value, right_value, "");
                case PARSER_OPERATOR_MINUS:
                    return LLVMBuildSub(codegen->builder, left_value, right_value, "");
                case PARSER_OPERATOR_STAR:
                    return LLVMBuildMul(codegen->builder, left_value, right_value, "");
                case PARSER_OPERATOR_SLASH:
                    return LLVMBuildSDiv(codegen->builder, left_value, right_value, "");
            }
        }

        default:
            printf("Code %i\n", ast->tag);
            assert(0);
            break;
    }
}
