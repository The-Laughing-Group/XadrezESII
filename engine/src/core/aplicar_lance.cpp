#include "xadrez/posicao.hpp"
#include "lances.hpp"

namespace xadrez {

Posicao aplicarLance(const Posicao& pos, Lance m) {
    Posicao nova = pos;   // Trabalha sobre uma cópia para preservar o histórico/estado anterior

    // Aplica o lance utilizando a máquina de regras otimizada por Bitboards
    fazer_lance(nova.b, m);

    return nova;
}

} // namespace xadrez