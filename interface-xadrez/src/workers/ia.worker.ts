import { carregarModulo } from '../hooks/useJogo'
import type { JogoWasm } from '../wasm/chess_engine.js'

interface PedidoIA { id: number; fen: string; profundidade: number }

let jogo: JogoWasm | null = null
const pronto = carregarModulo().then(modulo => {
    jogo = new modulo.Jogo()
})

self.onmessage = async (e: MessageEvent<PedidoIA>) => {
    await pronto
    const { id, fen, profundidade } = e.data

    let uci: string | null = null
    if (jogo && jogo.carregarFen(fen)) {
        uci = jogo.melhorLanceIA(profundidade) || null
    }
    self.postMessage({ id, uci })
}