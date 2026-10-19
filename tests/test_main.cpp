#include <gtest/gtest.h>

#include "Language.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    LANG.initialize();
    LANG.setLanguage(Language::Lang::ENGLISH);
    return RUN_ALL_TESTS();
}
