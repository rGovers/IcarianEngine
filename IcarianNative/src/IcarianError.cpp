#include "IcarianError.h"

#ifdef WIN32
// WIN32 keeps conflicting with stuff so needed to seperate into its own file
#include "Core/WindowsHeaders.h"
#endif

void IcarianError(const std::string_view& a_msg)
{
#ifdef WIN32
    MessageBoxA(NULL, a_msg.data(), NULL, MB_OK);
#endif

    ICARIAN_ASSERT_MSG_R(0, a_msg);

    exit(1);
}