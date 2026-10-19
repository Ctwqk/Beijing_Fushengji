#include "EventSourceFactory.h"

#include <iostream>

#ifdef FSJ_HAVE_CURL
#include "CurlChatClient.h"
#endif

EventSourceSetup makeEventSource(const AiConfig &config, std::uint32_t seed) {
    auto offline = std::make_shared<OfflineEventSource>(seed);
    EventSourceSetup setup;
    setup.cancel = [] {};
    setup.source = offline;
    setup.description = offline->describe();

    if (!config.enabled())
        return setup;
#ifdef FSJ_HAVE_CURL
    auto client = std::make_shared<CurlChatClient>(config);
    auto llm = std::make_shared<LlmEventSource>(client);
    setup.source = std::make_shared<FallbackEventSource>(llm, offline);
    setup.description = llm->describe();
    setup.ai = true;
    setup.cancel = [client] { client->cancel(); };
#else
    std::cerr << "[events] AI events requested but this build has no libcurl support; using offline events\n";
#endif
    return setup;
}
