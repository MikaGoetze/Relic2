//
// Created by mikag on 3/29/2020.
//

#include "FBXUtil.h"

ofbx::IScene *LoadScene(const std::string &fileName)
{
    auto data = ReadFile(fileName);
    return ofbx::load(reinterpret_cast<ofbx::u8 *>(data.data()), data.size(), static_cast<ofbx::u64>(ofbx::LoadFlags::TRIANGULATE));
}
