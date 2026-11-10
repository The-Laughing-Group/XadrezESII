#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include "search.hpp"

namespace {

class DebugListener : public ::testing::EmptyTestEventListener {
    // Chamado ANTES de cada teste
    void OnTestStart(const ::testing::TestInfo& info) override {
        g_log_busca.str("");     // esvazia o buffer
        g_log_busca.clear();

        // Liga o debug só nas suítes que começam com "IA", exceto a de performance
        const std::string suite = info.test_suite_name();
        g_debug_busca = suite.rfind("IA", 0) == 0
                        && suite.find("Performance") == std::string::npos;
    }

    // Chamado DEPOIS de cada teste
    void OnTestEnd(const ::testing::TestInfo& info) override {
        g_debug_busca = false;
        if (!info.result()->Failed()) return;   // passou: descarta o buffer

        // O primeiro teste que falha apaga o arquivo antigo; os próximos acrescentam
        static bool primeiro = true;
        std::ofstream arq("debug_falhas.txt",
                          primeiro ? std::ios::trunc : std::ios::app);
        primeiro = false;

        arq << "===== " << info.test_suite_name() << "." << info.name()
            << " =====\n" << g_log_busca.str() << "\n";
    }
};

// Roda antes do main: registra o listener no gtest (o gtest passa a ser dono dele)
[[maybe_unused]] const bool registrado = [] {
    ::testing::UnitTest::GetInstance()->listeners().Append(new DebugListener);
    return true;
}();

}  // namespace