#include "client/Client.h"
#include "utils/Version.h"
#include "modules/FeatureCatalog.h"

#include <iostream>

int main() {
    cloud9::Client client;
    if (!client.initialize()) return 1;

    std::size_t available = 0;
    for (const cloud9::Module* module : client.moduleManager().modules()) {
        if (module->available()) ++available;
    }

    std::cout << "Cloud9 " << CLOUD9_VERSION_MAJOR << '.' << CLOUD9_VERSION_MINOR << '.' << CLOUD9_VERSION_PATCH << '\n'
              << "feature catalog: " << cloud9::featureCatalog().size() << " entries, " << available << " available\n"
              << "config root: " << client.configManager().root().string() << '\n';

    for (const auto& message : client.executeCommand(".help Fullbright").messages) std::cout << message << '\n';
    client.shutdown();
    return 0;
}
