#include "xadrez/posicao.hpp"
#include "lances.hpp"

namespace xadrez {

// Devolve a posição depois do lance. A original não muda.
// Se o lance for ilegal (deixaria o rei em xeque), fazer_lance devolve false e a
// cópia sai idêntica à original; quem precisa saber disso deve validar antes
// (Jogo::tentarLance faz isso).
Posicao aplicarLance(const Posicao& pos, Lance m) {
    Posicao nova = pos;

    const bool eraVezDasPretas = (nova.b.estado.lado_a_jogar == Cor::PRETO);
    if (fazer_lance(nova.b, m) && eraVezDasPretas) {
        ++nova.numeroLance;   // o número do lance só avança depois do lance das pretas
    }
    return nova;
}

} // namespace xadrez