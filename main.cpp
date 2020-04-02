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
     * Write model loader that converts FBX into RPACK format.
     * Write runtime model loader.
     * Make vulkan work.
     */

    ModelImporter importer;
    ResourceManager manager = ResourceManager();

    manager.SetRPACK("test.rpack");

    GUID guid = importer.ImportResource("test.fbx");

    Relic relic;
    relic.Start();
}