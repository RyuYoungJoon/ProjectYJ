#pragma once

#include "Types.h"
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <chrono>
#include <format>
#include <filesystem>
#include <random>

#include <..\include\plog\Log.h>
#include <..\include\plog\Appenders\ColorConsoleAppender.h>
#include <..\include\plog\Formatters\TxtFormatter.h>
#include <..\include\plog\Initializers\ConsoleInitializer.h>
#include <..\include\plog\Appenders\RollingFileAppender.h>
#include <..\include\plog\Formatters\CsvFormatter.h>

#include "Shlwapi.h"

#include <iostream>
#include <boost/asio.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <boost/pool/pool_alloc.hpp>

using namespace std;

using boost::asio::ip::tcp;