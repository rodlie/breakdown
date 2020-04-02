/*
# Breakdown <https://github.com/rodlie/breakdown>
#
# Copyright (c) 2020, Ole-André Rodlie <ole.andre.rodlie@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "breakdown.h"

#include "common/scoped_ptr.h"
#include "google_breakpad/processor/basic_source_line_resolver.h"
#include "google_breakpad/processor/call_stack.h"
#include "google_breakpad/processor/minidump.h"
#include "google_breakpad/processor/minidump_processor.h"
#include "google_breakpad/processor/process_state.h"
#include "google_breakpad/processor/stack_frame_cpu.h"
#include "google_breakpad/processor/stack_frame_symbolizer.h"
#include "processor/pathname_stripper.h"
#include "processor/simple_symbol_supplier.h"

using google_breakpad::BasicSourceLineResolver;
using google_breakpad::CallStack;
using google_breakpad::Minidump;
using google_breakpad::MinidumpProcessor;
using google_breakpad::PathnameStripper;
using google_breakpad::ProcessResult;
using google_breakpad::ProcessState;
using google_breakpad::scoped_ptr;
using google_breakpad::SimpleSymbolSupplier;
using google_breakpad::StackFrame;
using google_breakpad::StackFrameSymbolizer;
using google_breakpad::SymbolSupplier;

const std::string BreakDown::convertDumpToString(const std::string &fileName,
                                                 const std::vector<std::string> &symbols)
{
    Minidump minidump(fileName);
    minidump.Read();

    scoped_ptr<SymbolSupplier> symbol_supplier;
    if (!symbols.empty()) {
        symbol_supplier.reset(new SimpleSymbolSupplier(symbols));
    }

    BasicSourceLineResolver resolver;
    StackFrameSymbolizer symbolizer(symbol_supplier.get(), &resolver);
    MinidumpProcessor minidump_processor(&symbolizer, true);
    ProcessState process_state;
    ProcessResult result = minidump_processor.Process(&minidump, &process_state);

    std::string txt;
    if (result != google_breakpad::PROCESS_OK) {
        return txt;
    }

    int requesting_thread = process_state.requesting_thread();
    if (process_state.crashed() && requesting_thread != -1) {
        //txt.append("\n### CRASH REPORT ### \n\n");
        txt.append(std::string("OS       : ").append(process_state.system_info()->os + " (" + process_state.system_info()->os_version +  ")\n"));
        txt.append(std::string("TYPE     : ").append(process_state.crash_reason() + "\n\n"));

        const CallStack *crashing_stack = process_state.threads()->at(requesting_thread);
        int frame_limit = 10;
        const unsigned kMaxThreadFrames = 100;
        const unsigned kTailFramesWhenTruncating = 10;
        int frame_count = crashing_stack->frames()->size();
        frame_count = std::min(frame_count, frame_limit);
        int last_head_frame = kMaxThreadFrames - kTailFramesWhenTruncating - 1;
        int first_tail_frame = frame_count - kTailFramesWhenTruncating;

        for (int frame_index = 0; frame_index < frame_count; ++frame_index) {
            if (frame_index > last_head_frame && frame_index < first_tail_frame) {
                continue;
            }
            const StackFrame *frame = crashing_stack->frames()->at(frame_index);
            if (!frame->module) { continue; }
            std::string frame_module;
            std::string frame_function;
            std::string frame_file;
            if (!frame->module->code_file().empty()) {
                frame_module = PathnameStripper::File(frame->module->code_file());
            }
            if (!frame->function_name.empty()) {
                frame_function = frame->function_name;
            }
            if (!frame->source_file_name.empty()) {
                frame_file = PathnameStripper::File(frame->source_file_name);
                frame_file.append(std::string(":").append(std::to_string(frame->source_line)));
            }
            std::string result;
            result.append(std::string("MODULE   : ").append(frame_module + "\n"));
            result.append(std::string("FUNCTION : ").append(frame_function + "\n"));
            result.append(std::string("SOURCE   : ").append(frame_file + "\n\n"));
            txt.append(result);
        }
    }
    return txt;
}
