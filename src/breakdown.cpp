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

const Breakdown::CrashResult Breakdown::generateCrashResult(const std::string &filename,
                                                            const std::vector<std::string> &storage,
                                                            int truncate)
{
    Minidump minidump(filename);
    minidump.Read();

    scoped_ptr<SymbolSupplier> symbol_supplier;
    if (!storage.empty()) {
        symbol_supplier.reset(new SimpleSymbolSupplier(storage));
    }

    BasicSourceLineResolver resolver;
    StackFrameSymbolizer symbolizer(symbol_supplier.get(), &resolver);
    MinidumpProcessor minidump_processor(&symbolizer, true);
    ProcessState process_state;
    ProcessResult result = minidump_processor.Process(&minidump, &process_state);
    Breakdown::CrashResult output;

    if (result != google_breakpad::PROCESS_OK) {
        return output;
    }

    int requesting_thread = process_state.requesting_thread();
    if (process_state.crashed() && requesting_thread != -1) {
        output.platform = process_state.system_info()->os + " " + process_state.system_info()->os_version;
        output.type = process_state.crash_reason();
        const CallStack *crashing_stack = process_state.threads()->at(requesting_thread);
        const unsigned kMaxThreadFrames = 100;
        const unsigned kTailFramesWhenTruncating = 10;
        int frame_count = crashing_stack->frames()->size();
        frame_count = std::min(frame_count, truncate);
        int last_head_frame = kMaxThreadFrames - kTailFramesWhenTruncating - 1;
        int first_tail_frame = frame_count - kTailFramesWhenTruncating;

        for (int frame_index = 0; frame_index < frame_count; ++frame_index) {
            if (frame_index > last_head_frame && frame_index < first_tail_frame) {
                continue;
            }
            const StackFrame *frame = crashing_stack->frames()->at(frame_index);
            if (!frame->module) { continue; }
            Breakdown::CrashFrame cframe;
            if (!frame->module->code_file().empty()) {
                cframe.module = PathnameStripper::File(frame->module->code_file());
            }
            if (!frame->function_name.empty()) {
                cframe.function = frame->function_name;
            }
            if (!frame->source_file_name.empty()) {
                cframe.source = frame->source_file_name;
                cframe.line = frame->source_line;
            }
            output.frames.push_back(cframe);
        }
    }
    return output;
}

const std::string Breakdown::generateCrashResultPlainText(const Breakdown::CrashResult report)
{
    std::string result;
    if (report.frames.size() == 0) { return result; }
    result.append("OS       : " + report.platform + "\n");
    result.append("TYPE     : " + report.type + "\n\n");
    for (unsigned int i = 0; i < report.frames.size(); ++i) {
        if (report.frames.at(i).module.empty()) { continue; }
        std::string frame;
        frame.append("MODULE   : " + report.frames.at(i).module + "\n");
        if (!report.frames.at(i).function.empty()) {
            frame.append("FUNCTION : " + report.frames.at(i).function + "\n");
        }
        if (!report.frames.at(i).source.empty()) {
            frame.append("SOURCE   : " + report.frames.at(i).source + ":" + std::to_string(report.frames.at(i).line) + "\n\n");
        }
        result.append(frame);
    }
    return result;
}

const std::string Breakdown::generateCrashResultPlainText(const std::string &filename,
                                                          const std::vector<std::string> &storage)
{
    Breakdown::CrashResult report = Breakdown::generateCrashResult(filename, storage);
    if (report.frames.size() == 0) { return std::string(); }
    return Breakdown::generateCrashResultPlainText(report);
}

const std::string Breakdown::generateCrashResultXML(const Breakdown::CrashResult report)
{
    std::string result;
    if (report.frames.size() == 0) { return result; }
    result.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    result.append("<root>\n");
    result.append("<platform>" + report.platform + "</platform>\n");
    result.append("<type>" + report.type + "</type>\n");
    for (unsigned int i = 0; i < report.frames.size(); ++i) {
        if (report.frames.at(i).module.empty()) { continue; }
        std::string frame = "<item>\n";
        frame.append("<module>" + report.frames.at(i).module + "</module>\n");
        if (!report.frames.at(i).function.empty()) {
            frame.append("<function><![CDATA[" + report.frames.at(i).function + "]]></function>\n");
        }
        if (!report.frames.at(i).source.empty()) {
            frame.append("<source>" + report.frames.at(i).source + "</source>\n");
            frame.append("<line>" + std::to_string(report.frames.at(i).line) + "</line>\n");
        }
        frame.append("</item>\n");
        result.append(frame);
    }
    result.append("</root>");
    return result;
}

const std::string Breakdown::generateCrashResultXML(const std::string &filename,
                                                    const std::vector<std::string> &storage)
{
    Breakdown::CrashResult report = Breakdown::generateCrashResult(filename, storage);
    if (report.frames.size() == 0) { return std::string(); }
    return Breakdown::generateCrashResultXML(report);
}

