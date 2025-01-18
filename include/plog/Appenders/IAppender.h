#pragma once
#include <..\include\plog/Record.h>
#include <..\include\plog/Util.h>

namespace plog
{
    class PLOG_LINKAGE IAppender
    {
    public:
        virtual ~IAppender()
        {
        }

        virtual void write(const Record& record) = 0;
    };
}
