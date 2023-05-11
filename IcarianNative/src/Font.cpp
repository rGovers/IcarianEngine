#include "Rendering/UI/Font.h"

#include <fstream>
#include <limits>

#include "Flare/IcarianAssert.h"
#include "Trace.h"

Font::Font(const std::string_view& a_path)
{
    TRACE("Loading font");

    std::ifstream file = std::ifstream(a_path.data(), std::ios_base::binary);
    if (file.good() && file.is_open())
    {
        file.ignore(std::numeric_limits<std::streamsize>::max());
        const std::streamsize size = file.gcount();
        file.clear();
        file.seekg(0, std::ios::beg);

        ICARIAN_ASSERT_R(size != 0);

        unsigned char* dat = new unsigned char[size];
        file.read((char*)dat, size);

        file.close();

        // ICARIAN_ASSERT_R(stbtt_InitFont(&m_fontInfo, dat, 0) != 0);

        delete[] dat;
    }
    else
    {
        ICARIAN_ASSERT_MSG_R(0, "Unable to open font file");
    }
}
Font::~Font()
{
    
}