import { useCallback, useEffect, useState } from 'react'
import type { JogoWasm } from '../wasm/chess_engine.js'
import { carregarModulo } from './useJogo'

const FEN_INICIAL = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'

// O que o React precisa para DESENHAR a partida. É uma cópia do estado do motor,
// atualizada depois de cada mudança. Atenção: aqui são VALORES (partida.fen),
// e não funções como no objeto do C++ (jogo.fen()).
interface EstadoDaPartida {
    fen: string              // posição completa em texto
    tabuleiro: string[]      // 8 textos de 8 caracteres, '.' = casa vazia
    vezDasBrancas: boolean
}

export interface Partida extends EstadoDaPartida {
    pronta: boolean                              // false enquanto o WebAssembly carrega
    lancesDe: (casa: string) => string[]         // ex.: lancesDe("e2") -> ["e2e3", "e2e4"]
    tentarLance: (uci: string) => boolean        // true se o motor aceitou e atualizou a posição
    carregarFen: (fen: string) => boolean        // true se o FEN era válido e foi carregado
    reiniciar: () => void                        // volta para a posição inicial
}

const ESTADO_VAZIO: EstadoDaPartida = { fen: '', tabuleiro: [], vezDasBrancas: true }

function lerEstado(jogo: JogoWasm): EstadoDaPartida {
    return { fen: jogo.fen(), tabuleiro: jogo.tabuleiro(), vezDasBrancas: jogo.vezDasBrancas() }
}

// A posição "de verdade" vive no objeto C++. Como esse objeto muda POR DENTRO (a referência
// continua a mesma), o React não percebe. Por isso guardamos uma cópia em um estado e a
// atualizamos depois de cada mudança aceita: é a mudança do estado que faz o React redesenhar.
//
// Use este hook UMA vez só (por exemplo no Game.tsx) e passe o resultado para os componentes.
// Cada chamada cria um Jogo separado, com a sua própria posição.
export function usePartida(): Partida {
    const [jogo, setJogo] = useState<JogoWasm | null>(null)
    const [estado, setEstado] = useState<EstadoDaPartida>(ESTADO_VAZIO)

    useEffect(() => {
        let ativo = true
        let instancia: JogoWasm | null = null

        carregarModulo()
            .then(modulo => {
                if (!ativo) return
                instancia = new modulo.Jogo()
                setJogo(instancia)
                setEstado(lerEstado(instancia))   // primeira cópia: posição inicial
            })
            .catch(erro => console.error('Falha ao carregar o motor de regras:', erro))

        return () => {
            ativo = false
            instancia?.delete()   // objetos do C++ precisam ser liberados manualmente
        }
    }, [])

    const lancesDe = useCallback(
        (casa: string) => (jogo ? jogo.lancesDe(casa) : []),
        [jogo],
    )

    const tentarLance = useCallback(
        (uci: string) => {
            if (!jogo) return false
            const aceito = jogo.tentarLance(uci)       // o MOTOR decide se o lance vale
            if (aceito) setEstado(lerEstado(jogo))     // nova cópia -> React redesenha
            return aceito
        },
        [jogo],
    )

    const carregarFen = useCallback(
        (fen: string) => {
            if (!jogo) return false
            const carregado = jogo.carregarFen(fen)
            if (carregado) setEstado(lerEstado(jogo))
            return carregado
        },
        [jogo],
    )

    const reiniciar = useCallback(() => {
        carregarFen(FEN_INICIAL)
    }, [carregarFen])

    return { pronta: jogo !== null, ...estado, lancesDe, tentarLance, carregarFen, reiniciar }
}