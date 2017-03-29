/*
  GridTools Libraries

  Copyright (c) 2016, GridTools Consortium
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  For information: http://eth-cscs.github.io/gridtools/
*/
#pragma once

#include <mpi.h>
#include <cstdio>
#include <ostream>
#include <stdexcept>

#include <communication/GCL.hpp>

#include "gtest/gtest.h"

/// A specialized listener desinged for printing test results with MPI.
///
/// When tests are run with MPI, one instance of each test is run on
/// each rank. The default behavior of Google Test is for each test
/// instance to print to stdout. With more than one MPI rank, this creates
/// the usual MPI mess of output.
///
/// This specialization has the first rank (rank 0) print to stdout, and all MPI
/// ranks print their output to separate text files.
/// For each test a message is printed showing
///     - detailed messages about errors on rank 0
///     - a head count of errors that occured on other MPI ranks

class mpi_listener : public testing::EmptyTestEventListener {
  private:
    using UnitTest = testing::UnitTest;
    using TestCase = testing::TestCase;
    using TestInfo = testing::TestInfo;
    using TestPartResult = testing::TestPartResult;

    int rank_ = 0;
    int size_ = 0;
    std::ofstream fid_;
    char buffer_[1024];
    int test_case_failures_ = 0;
    int test_case_tests_ = 0;
    int test_failures_ = 0;

    bool does_print() const { return rank_ == 0; }

    void print(const char *s) {
        fid_ << s;
        if (does_print()) {
            std::cout << s;
        }
    }

    void print(const std::string &s) { print(s.c_str()); }

    /// convenience function that handles the logic of using snprintf
    /// and forwarding the results to file and/or stdout.
    ///
    /// TODO : it might be an idea to use a resizeable buffer
    template < typename... Args >
    void printf_helper(const char *s, Args &&... args) {
        snprintf(buffer_, sizeof(buffer_), s, std::forward< Args >(args)...);
        print(buffer_);
    }

  public:
    mpi_listener(std::string f_base = "") {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
        MPI_Comm_size(MPI_COMM_WORLD, &size_);

        if (f_base.empty()) {
            return;
        }
        std::string fname = f_base + "_" + std::to_string(rank_) + ".txt";
        fid_.open(fname);
        if (!fid_) {
            throw std::runtime_error(
                "PID:" + std::to_string(rank_) + " could not open file " + fname + " for test output");
        }
    }

    /// Messages that are printed at the start and end of the test program.
    /// i.e. once only.
    virtual void OnTestProgramStart(const UnitTest &) override {
        printf_helper("*** test output for rank %d of %d\n\n", rank_, size_);
    }
    virtual void OnTestProgramEnd(const UnitTest &) override {
        printf_helper("*** end test output for rank %d of %d\n", rank_, size_);
    }

    /// Messages that are printed at the start and end of each test case.
    /// On startup a counter that counts the number of tests that fail in
    /// this test case is initialized to zero, and will be incremented for each
    /// test that fails.
    virtual void OnTestCaseStart(const TestCase &test_case) override {
        test_case_failures_ = 0;
        test_case_tests_ = 0;
    }
    virtual void OnTestCaseEnd(const TestCase &test_case) override {

        if (test_case_failures_) {
            printf_helper("[PASSED %3d; FAILED %3d] of %3d tests in %s\n\n",
                test_case_tests_ - test_case_failures_,
                test_case_failures_,
                test_case_tests_,
                test_case.name());
        } else {
            printf_helper(
                "[ALL PASSED %3d; ] of %3d tests in %s\n\n", test_case_tests_, test_case_tests_, test_case.name());
        }
    }

    // Called before a test starts.
    virtual void OnTestStart(const TestInfo &test_info) override {
        printf_helper("  TEST  %s::%s\n", test_info.test_case_name(), test_info.name());
        test_failures_ = 0;
    }

    // Called after a failed assertion or a SUCCEED() invocation.
    virtual void OnTestPartResult(const TestPartResult &test_part_result) override {
        const char *banner = "--------------------------------------------------------------------------------";

        // indent all lines in the summary by 4 spaces
        std::string summary = "    " + std::string(test_part_result.summary());
        auto pos = summary.find("\n");
        while (pos != summary.size() && pos != std::string::npos) {
            summary.replace(pos, 1, "\n    ");
            pos = summary.find("\n", pos + 1);
        }

        printf_helper("  LOCAL_%s\n    %s\n    %s:%d\n%s\n    %s\n",
            test_part_result.failed() ? "FAIL" : "SUCCESS",
            banner,
            test_part_result.file_name(),
            test_part_result.line_number(),
            summary.c_str(),
            banner);

        // note that there was a failure in this test case
        if (test_part_result.failed()) {
            test_failures_++;
        }
    }

    // Called after a test ends.
    virtual void OnTestEnd(const TestInfo &test_info) override {
        test_case_tests_++;

        // count the number of ranks that had errors
        int global_errors{};
        MPI_Reduce(&test_failures_, &global_errors, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (global_errors > 0) {
            test_case_failures_++;
            printf_helper("  GLOBAL_FAIL on %d ranks\n", global_errors);
        }
    }
};
