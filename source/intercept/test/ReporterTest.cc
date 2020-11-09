/*  Copyright (C) 2012-2020 by László Nagy
    This file is part of Bear.

    Bear is a tool to generate compilation database for clang tooling.

    Bear is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Bear is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gtest/gtest.h"

#include "collect/Reporter.h"

namespace {

    supervise::Event start_event()
    {
        auto inner = new supervise::Event_Started();
        inner->set_executable("/usr/bin/ls");
        inner->add_arguments("ls");
        inner->add_arguments("-l");
        inner->set_working_dir("/home/user");
        inner->mutable_environment()->insert({ "HOME", "/home/user" });
        inner->mutable_environment()->insert({ "PATH", "/usr/bin:/usr/local/bin" });

        auto result = supervise::Event();
        result.set_timestamp("2020-04-04T07:13:47.027Z");
        result.set_pid(42);
        result.set_ppid(12);
        result.set_allocated_started(inner);

        return result;
    }

    supervise::Event signal_event()
    {
        auto inner = new supervise::Event_Signalled();
        inner->set_number(15);

        auto result = supervise::Event();
        result.set_pid(42);
        result.set_timestamp("2020-04-04T07:13:47.045Z");
        result.set_allocated_signalled(inner);

        return result;
    }

    supervise::Event stop_event()
    {
        auto inner = new supervise::Event_Terminated();
        inner->set_status(0);

        auto result = supervise::Event();
        result.set_pid(42);
        result.set_timestamp("2020-04-04T07:13:47.074Z");
        result.set_allocated_terminated(inner);

        return result;
    }

    struct ReporterFixture : ic::Reporter {
    public:
        ReporterFixture(const std::string_view& view, report::Context&& context)
                : Reporter(view, std::move(context))
        {
        }

        using Reporter::flush;
        using Reporter::makeReport;
    };

    TEST(reporter, builder_makes_empty_execution_object)
    {
        report::Report expected = report::Report {
            report::Context { "session", { { "key", "value" } } },
            {}
        };
        ReporterFixture sut(
            "ignore",
            report::Context { "session", { { "key", "value" } } });

        report::Report result = sut.makeReport();
        EXPECT_EQ(result, expected);
    }

    TEST(reporter, builder_makes_empty_object_without_start_event)
    {
        report::Report expected = report::Report {
            report::Context { "session", { { "key", "value" } } },
            {}
        };
        ReporterFixture sut(
            "ignore",
            report::Context { "session", { { "key", "value" } } });
        sut.report(signal_event());
        sut.report(stop_event());

        report::Report result = sut.makeReport();
        EXPECT_EQ(result, expected);
    }

    TEST(reporter, builder_makes_execution_object_from_events)
    {
        report::Report expected = report::Report {
            report::Context { "session", { { "key", "value" } } },
            {
                report::Execution {
                    report::Command {
                        "/usr/bin/ls",
                        { "ls", "-l" },
                        "/home/user",
                        { { "HOME", "/home/user" }, { "PATH", "/usr/bin:/usr/local/bin" } } },
                    report::Run {
                        42 ,
                        12,
                        {
                            report::Event {"started", "2020-04-04T07:13:47.027Z", std::nullopt, std::nullopt },
                            report::Event {"signaled", "2020-04-04T07:13:47.045Z", std::nullopt, { 15 } },
                            report::Event {"terminated", "2020-04-04T07:13:47.074Z", { 0 }, std::nullopt }
                        }
                    }
                }
            }
        };
        ReporterFixture sut(
            "ignore",
            report::Context { "session", { { "key", "value" } } });
        sut.report(start_event());
        sut.report(signal_event());
        sut.report(stop_event());

        report::Report result = sut.makeReport();
        EXPECT_EQ(result, expected);
    }
}
