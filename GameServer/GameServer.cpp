#include "pch.h"
#include "AsioNetwork.h"

int main()
{
    try
    {
        AsioNetwork* cAsioNetwork;
        cAsioNetwork->Run();

    }
    catch(std::exception& exception)
    {
        std::cerr << "Exception : " << exception.what() << endl;
    }
}
