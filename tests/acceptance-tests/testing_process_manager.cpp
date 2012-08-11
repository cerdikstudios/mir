/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "testing_process_manager.h"

#include "mir/display_server.h"

#include "mir/chrono/chrono.h"
#include "mir/thread/all.h"

#include <boost/asio.hpp>

#include <gmock/gmock.h>

#include <stdexcept>

namespace mc = mir::compositor;
namespace mp = mir::process;

namespace mir
{
::testing::AssertionResult WasStarted(
    std::shared_ptr<mir::process::Process> const& server_process)
{
    if (server_process)
        return ::testing::AssertionSuccess() << "server started";
    else
        return ::testing::AssertionFailure() << "server NOT started";
}

void startup_pause()
{
    if (!mir::detect_server(mir::test_socket_file(), std::chrono::milliseconds(200)))
        throw std::runtime_error("Failed to find server");
}


TestingProcessManager::TestingProcessManager() :
    is_test_process(true)
{
}

TestingProcessManager::~TestingProcessManager()
{
}

namespace
{
mir::DisplayServer* volatile signal_display_server;
}

extern "C"
{
void (*signal_prev_fn)(int);
void signal_terminate (int )
{
    while (true)
    {
	if(signal_display_server)
	    break;
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    signal_display_server->stop();
 
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // TODO: Investigate into C99 atomic-support to get rid
    // of the ugly mutex-based synchronization approach.
    //auto sds = mir::signal_display_server.load();
        
    /* could spin briefly during startup */;
    /*for (; !sds; sds = mir::signal_display_server.load())
	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	sds->stop();*/
}
}

void TestingProcessManager::launch_server_process(TestingServerConfiguration& config)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        throw std::runtime_error("Failed to fork process");
    }

    if (pid == 0)
    {
        using namespace mir;
        is_test_process = false;

        // We're in the server process, so create a display server
        SCOPED_TRACE("Server");

        signal_prev_fn = signal(SIGTERM, signal_terminate);

        auto buffer_allocation_strategy =
            config.make_buffer_allocation_strategy();

        mir::DisplayServer server(
                config.make_communicator(config.make_ipc_factory(buffer_allocation_strategy)),
                buffer_allocation_strategy,
                config.make_renderer());

        //std::atomic_store(&signal_display_server, &server);
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	// TODO: Re-enable once we have proper support for atomics.
	{
	    //std::lock_guard<std::mutex> lg(guard);
	    signal_display_server = &server;
	}

        {
            struct ScopedFuture
            {
                std::future<void> future;
                ~ScopedFuture() { future.wait(); }
            } scoped;

            scoped.future = std::async(std::launch::async, std::bind(&mir::DisplayServer::start, &server));

            config.exec(&server);
        }

        config.on_exit(&server);
    }
    else
    {
        server_process = std::shared_ptr<mp::Process>(new mp::Process(pid));
        startup_pause();
    }
}

void TestingProcessManager::launch_client_process(TestingClientConfiguration& config)
{
    if (!is_test_process)
    {
        return; // We're not in the test process, so just return gracefully
    }

    // We're in the test process, so make sure we started a service
    ASSERT_TRUE(WasStarted(server_process));

    pid_t pid = fork();

    if (pid < 0)
    {
        throw std::runtime_error("Failed to fork process");
    }

    if (pid == 0)
    {
        is_test_process = false;

        // Need to avoid terminating server or other clients
        server_process->detach();
        for(auto client = clients.begin(); client != clients.end(); ++client)
        {
            (*client)->detach();
        }

        clients.clear();
        server_process.reset();

        SCOPED_TRACE("Client");
        config.exec();
        exit(::testing::Test::HasFailure() ? EXIT_FAILURE : EXIT_SUCCESS);
    }
    else
    {
        clients.push_back(std::shared_ptr<mp::Process>(new mp::Process(pid)));
    }
}

void TestingProcessManager::tear_down_clients()
{
    if (is_test_process)
    {
        using namespace testing;

        if (clients.empty())
        {
            // Allow some time for server-side only tests to run
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        for(auto client = clients.begin(); client != clients.end(); ++client)
        {
            auto result((*client)->wait_for_termination());
            EXPECT_TRUE(result.succeeded()) << "result=" << result;
        }

        clients.clear();
    }
    else
    {
        exit(::testing::Test::HasFailure() ? EXIT_FAILURE : EXIT_SUCCESS);
    }
}

void TestingProcessManager::tear_down_server()
{
    if (is_test_process)
    {
        ASSERT_TRUE(clients.empty())  << "Clients should be stopped before server";
        // We're in the test process, so make sure we started a service
        ASSERT_TRUE(WasStarted(server_process));
        server_process->terminate();
        mp::Result const result = server_process->wait_for_termination();
        EXPECT_TRUE(result.succeeded()) << result;
        server_process.reset();
    }
}

void TestingProcessManager::tear_down_all()
{
    tear_down_clients();
    tear_down_server();
}

bool detect_server(
        const std::string& socket_file,
        std::chrono::milliseconds const& timeout)
{
    auto limit = std::chrono::system_clock::now() + timeout;

    bool error = false;
    struct stat file_status;

    do
    {
      if (error) {
	  std::this_thread::sleep_for(std::chrono::milliseconds(0));
      }
      error = stat(socket_file.c_str(), &file_status);
    }
    while (error && std::chrono::system_clock::now() < limit);

    return !error;
}
}
