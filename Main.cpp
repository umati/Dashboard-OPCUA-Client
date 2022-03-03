/**
 * MIT License
 *
 * Copyright 2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "DashboardOpcUaClient.hpp"
#include <signal.h>
#include <easylogging++.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <ConfigureLogger.hpp>
#include <ConfigurationJsonFile.hpp>
#include <Exceptions/ConfigurationException.hpp>
#include <chrono>


std::atomic_bool running = {true};
std::atomic_bool reset = {false};

static void stopHandler(int sig)
{
	LOG(INFO) << "Execution termindated by ctrl-c";
	running = false;
}

static void issueReset()
{
	LOG(INFO) << "Requesting reset";
	reset = true;
}

int main(int argc, char *argv[])
{
	Umati::Util::ConfigureLogger("DashboardOpcUaClient");

	signal(SIGINT, stopHandler);
	signal(SIGTERM, stopHandler);

	LOG(INFO) << "Start Dashboard OPC UA Client";

	std::string configFilename("configuration.json");

	if (argc >= 2)
	{
		configFilename = argv[1];
	}

	std::shared_ptr<Umati::Util::Configuration> config;
	try
	{
		config = std::make_shared<Umati::Util::ConfigurationJsonFile>(configFilename);
	}
	catch (Umati::Util::Exception::ConfigurationException &ex)
	{
		LOG(ERROR) << "Configuration could not be loaded: " << ex.what();
		std::cout << "Usage <>.exe [ConfigurationFileName]";
		return 1;
	}

	std::size_t resetCounter = 0;
	while(running) {
		if(reset) {
			++resetCounter;
			reset = false;
		}
		DashboardOpcUaClient dashboardClient(config, issueReset);
		if (!dashboardClient.connect(running))
		{
			LOG(INFO) << "Connection not established, exiting.";
			return -1;
		}
		dashboardClient.ReadTypeDictionaries();
		dashboardClient.ReadTypes();
		dashboardClient.StartMachineObserver();
		while (running && !reset)
		{
			dashboardClient.Iterate();
		}
	}
	
	LOG(INFO) << "End Dashboard OPC UA Client";
	return 0;
}
