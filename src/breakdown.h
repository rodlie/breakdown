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

#ifndef BREAKDOWN_H
#define BREAKDOWN_H

#include <string>
#include <vector>

#if defined(_WIN32)
#  if !defined(BREAKDOWN_EXPORT) && !defined(BREAKDOWN_IMPORT)
#    define BREAKDOWN_EXPORT
#  elif defined(BREAKDOWN_IMPORT)
#    if defined(BREAKDOWN_EXPORT)
#      undef BREAKDOWN_EXPORT
#    endif
#    define BREAKDOWN_EXPORT __declspec(dllimport)
#  elif defined(BREAKDOWN_EXPORT)
#    undef BREAKDOWN_EXPORT
#    define BREAKDOWN_EXPORT __declspec(dllexport)
#  endif
#else
#  define BREAKDOWN_EXPORT
#endif

class BREAKDOWN_EXPORT Breakdown
{

public:

    struct CrashFrame
    {
        std::string module;
        std::string function;
        std::string source;
        int line = 0;
    };

    struct CrashResult
    {
        std::string dump;
        std::string platform;
        std::string type;
        std::vector<Breakdown::CrashFrame> frames;
    };

    /** @brief Generate crash result from Breakpad crash dump file */
    static const Breakdown::CrashResult generateCrashResult(const std::string &filename,
                                                            const std::vector<std::string> &storage,
                                                            int truncate = 10);

    /** @brief Generate crash result from Breakpad crash dump file in plain text */
    static const std::string generateCrashResultPlainText(const Breakdown::CrashResult report);
    static const std::string generateCrashResultPlainText(const std::string &filename,
                                                          const std::vector<std::string> &storage);

    /** @brief Generate crash result from Breakpad crash dump file in XML */
    static const std::string generateCrashResultXML(const Breakdown::CrashResult report);
    static const std::string generateCrashResultXML(const std::string &filename,
                                                    const std::vector<std::string> &storage);
};

#endif // BREAKDOWN_H
