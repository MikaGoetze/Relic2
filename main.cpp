#include <iostream>
#include <Core/Relic.h>
#include <Importers/ModelImporter.h>
#include <Graphics/Model.h>

struct Test
{
    int a;
    float b;
};

int main()
{
    /* TODO:
     *
     * Make vulkan work. :(
     */

    //TESTING CODE
    ModelImporter importer;
    ResourceManager manager = ResourceManager();

    manager.SetRPACK("test.rpack");

    GUID guid = importer.ImportResource("test.fbx");
    Logger::Log(std::to_string(guid).c_str());
    ResourceManager::GetInstance()->WriteRPACK();

    auto* model = (Model*) ResourceManager::GetInstance()->GetResourceData(guid, true);
    Logger::Log("Loaded model");
    //TESTING CODE

    Relic relic;
    relic.Start();
}