import { useEffect, useState } from 'react'
import criarModulo from '../wasm/chess_engine.js'
import type { JogoWasm, ModuloXadrez } from '../wasm/chess_engine.js'

// O módulo WebAssembly é baixado e inicializado uma única vez para a aplicação inteira.
let carregamento: Promise<ModuloXadrez> | null = null
function carregarModulo(): Promise<ModuloXadrez> {
    carregamento ??= criarModulo()
    return carregamento
}

// Devolve o objeto Jogo do motor C++ (ou null enquanto o módulo ainda está carregando).
export function useJogo(): JogoWasm | null {
    const [jogo, setJogo] = useState<JogoWasm | null>(null)

    useEffect(() => {
        let ativo = true
        let instancia: JogoWasm | null = null

        carregarModulo()
            .then(modulo => {
                if (!ativo) return
                instancia = new modulo.Jogo()
                setJogo(instancia)
            })
            .catch(erro => console.error('Falha ao carregar o motor de regras:', erro))

        return () => {
            ativo = false
            instancia?.delete()   // objetos do C++ precisam ser liberados manualmente
        }
    }, [])

    return jogo
}
