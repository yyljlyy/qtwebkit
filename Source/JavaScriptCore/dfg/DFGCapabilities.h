/*
 * Copyright (C) 2011, 2012 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef DFGCapabilities_h
#define DFGCapabilities_h

#include "Intrinsic.h"
#include "DFGCommon.h"
#include "DFGNode.h"
#include "Executable.h"
#include "Options.h"
#include "Interpreter.h"
#include <wtf/Platform.h>

namespace JSC { namespace DFG {

#if ENABLE(DFG_JIT)
// Fast check functions; if they return true it is still necessary to
// check opcodes.
inline bool mightCompileEval(CodeBlock* codeBlock)
{
    return codeBlock->instructionCount() <= Options::maximumOptimizationCandidateInstructionCount;
}
inline bool mightCompileProgram(CodeBlock* codeBlock)
{
    return codeBlock->instructionCount() <= Options::maximumOptimizationCandidateInstructionCount;
}
inline bool mightCompileFunctionForCall(CodeBlock* codeBlock)
{
    return codeBlock->instructionCount() <= Options::maximumOptimizationCandidateInstructionCount;
}
inline bool mightCompileFunctionForConstruct(CodeBlock* codeBlock)
{
    return codeBlock->instructionCount() <= Options::maximumOptimizationCandidateInstructionCount;
}

inline bool mightInlineFunctionForCall(CodeBlock* codeBlock)
{
    return codeBlock->instructionCount() <= Options::maximumFunctionForCallInlineCandidateInstructionCount
        && !codeBlock->ownerExecutable()->needsActivation();
}
inline bool mightInlineFunctionForConstruct(CodeBlock* codeBlock)
{
    return codeBlock->instructionCount() <= Options::maximumFunctionForConstructInlineCandidateInstructionCount
        && !codeBlock->ownerExecutable()->needsActivation();
}

// Opcode checking.
inline CapabilityLevel canCompileOpcode(OpcodeID opcodeID, CodeBlock*, Instruction*)
{
    switch (opcodeID) {
    case op_enter:
    case op_convert_this:
    case op_create_this:
    case op_bitand:
    case op_bitor:
    case op_bitxor:
    case op_rshift:
    case op_lshift:
    case op_urshift:
    case op_pre_inc:
    case op_post_inc:
    case op_pre_dec:
    case op_post_dec:
    case op_add:
    case op_sub:
    case op_negate:
    case op_mul:
    case op_mod:
    case op_div:
#if ENABLE(DEBUG_WITH_BREAKPOINT)
    case op_debug:
#endif
    case op_mov:
    case op_check_has_instance:
    case op_instanceof:
    case op_is_undefined:
    case op_is_boolean:
    case op_is_number:
    case op_is_string:
    case op_is_object:
    case op_is_function:
    case op_not:
    case op_less:
    case op_lesseq:
    case op_greater:
    case op_greatereq:
    case op_eq:
    case op_eq_null:
    case op_stricteq:
    case op_neq:
    case op_neq_null:
    case op_nstricteq:
    case op_get_by_val:
    case op_put_by_val:
    case op_method_check:
    case op_get_scoped_var:
    case op_put_scoped_var:
    case op_get_by_id:
    case op_put_by_id:
    case op_put_by_id_transition_direct:
    case op_put_by_id_transition_normal:
    case op_get_global_var:
    case op_get_global_var_watchable:
    case op_put_global_var:
    case op_put_global_var_check:
    case op_jmp:
    case op_loop:
    case op_jtrue:
    case op_jfalse:
    case op_loop_if_true:
    case op_loop_if_false:
    case op_jeq_null:
    case op_jneq_null:
    case op_jless:
    case op_jlesseq:
    case op_jgreater:
    case op_jgreatereq:
    case op_jnless:
    case op_jnlesseq:
    case op_jngreater:
    case op_jngreatereq:
    case op_loop_hint:
    case op_loop_if_less:
    case op_loop_if_lesseq:
    case op_loop_if_greater:
    case op_loop_if_greatereq:
    case op_ret:
    case op_end:
    case op_call_put_result:
    case op_resolve:
    case op_resolve_base:
    case op_resolve_global:
    case op_new_object:
    case op_new_array:
    case op_new_array_buffer:
    case op_strcat:
    case op_to_primitive:
    case op_throw:
    case op_throw_reference_error:
    case op_call:
    case op_construct:
    case op_new_regexp: 
    case op_init_lazy_reg:
    case op_create_activation:
    case op_tear_off_activation:
    case op_create_arguments:
    case op_tear_off_arguments:
    case op_new_func:
    case op_new_func_exp:
    case op_get_argument_by_val:
    case op_get_arguments_length:
    case op_jneq_ptr:
        return CanCompile;
        
    case op_call_varargs:
        return ShouldProfile;

    default:
        return CannotCompile;
    }
}

inline bool canInlineOpcode(OpcodeID opcodeID, CodeBlock* codeBlock, Instruction* pc)
{
    switch (opcodeID) {
        
    // These opcodes would be easy to support with inlining, but we currently don't do it.
    // The issue is that the scope chain will not be set correctly.
    case op_get_scoped_var:
    case op_put_scoped_var:
    case op_resolve:
    case op_resolve_base:
        
    // Constant buffers aren't copied correctly. This is easy to fix, but for
    // now we just disable inlining for functions that use them.
    case op_new_array_buffer:
        
    // Inlining doesn't correctly remap regular expression operands.
    case op_new_regexp:
        
    // We don't support inlining code that creates activations or has nested functions.
    case op_create_activation:
    case op_tear_off_activation:
    case op_new_func:
    case op_new_func_exp:
        return false;
        
    // Inlining supports op_call_varargs if it's a call that just forwards the caller's
    // arguments.
    case op_call_varargs:
        return codeBlock->usesArguments() && pc[3].u.operand == codeBlock->argumentsRegister();
        
    default:
        return canCompileOpcode(opcodeID, codeBlock, pc) == CanCompile;
    }
}

CapabilityLevel canCompileOpcodes(CodeBlock*);
bool canInlineOpcodes(CodeBlock*);
#else // ENABLE(DFG_JIT)
inline bool mightCompileEval(CodeBlock*) { return false; }
inline bool mightCompileProgram(CodeBlock*) { return false; }
inline bool mightCompileFunctionForCall(CodeBlock*) { return false; }
inline bool mightCompileFunctionForConstruct(CodeBlock*) { return false; }
inline bool mightInlineFunctionForCall(CodeBlock*) { return false; }
inline bool mightInlineFunctionForConstruct(CodeBlock*) { return false; }

inline CapabilityLevel canCompileOpcode(OpcodeID, CodeBlock*, Instruction*) { return CannotCompile; }
inline bool canInlineOpcode(OpcodeID, CodeBlock*, Instruction*) { return false; }
inline CapabilityLevel canCompileOpcodes(CodeBlock*) { return CannotCompile; }
inline bool canInlineOpcodes(CodeBlock*) { return false; }
#endif // ENABLE(DFG_JIT)

inline CapabilityLevel canCompileEval(CodeBlock* codeBlock)
{
    if (!mightCompileEval(codeBlock))
        return CannotCompile;
    
    return canCompileOpcodes(codeBlock);
}

inline CapabilityLevel canCompileProgram(CodeBlock* codeBlock)
{
    if (!mightCompileProgram(codeBlock))
        return CannotCompile;
    
    return canCompileOpcodes(codeBlock);
}

inline CapabilityLevel canCompileFunctionForCall(CodeBlock* codeBlock)
{
    if (!mightCompileFunctionForCall(codeBlock))
        return CannotCompile;
    
    return canCompileOpcodes(codeBlock);
}

inline CapabilityLevel canCompileFunctionForConstruct(CodeBlock* codeBlock)
{
    if (!mightCompileFunctionForConstruct(codeBlock))
        return CannotCompile;
    
    return canCompileOpcodes(codeBlock);
}

inline bool canInlineFunctionForCall(CodeBlock* codeBlock)
{
    return mightInlineFunctionForCall(codeBlock) && canInlineOpcodes(codeBlock);
}

inline bool canInlineFunctionForConstruct(CodeBlock* codeBlock)
{
    return mightInlineFunctionForConstruct(codeBlock) && canInlineOpcodes(codeBlock);
}

inline bool mightInlineFunctionFor(CodeBlock* codeBlock, CodeSpecializationKind kind)
{
    if (kind == CodeForCall)
        return mightInlineFunctionForCall(codeBlock);
    ASSERT(kind == CodeForConstruct);
    return mightInlineFunctionForConstruct(codeBlock);
}

inline bool canInlineFunctionFor(CodeBlock* codeBlock, CodeSpecializationKind kind)
{
    if (kind == CodeForCall)
        return canInlineFunctionForCall(codeBlock);
    ASSERT(kind == CodeForConstruct);
    return canInlineFunctionForConstruct(codeBlock);
}

} } // namespace JSC::DFG

#endif // DFGCapabilities_h

