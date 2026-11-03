import { useCallback, useEffect, useRef, useState } from 'react'
import type { InfoLance, JogoWasm, SituacaoPartida } from '../wasm/chess_engine.js'
import { carregarModulo } from './useJogo'

const FEN_INICIAL = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'

// O que o React precisa para DESENHAR a partida. É uma cópia do estado do motor,
// atualizada depois de cada mudança. Atenção: aqui são VALORES (partida.fen),
// e não funções como no objeto do C++ (jogo.fen()).
interface EstadoDaPartida {
    fen: string              // posição completa em texto
    tabuleiro: string[]      // 8 textos de 8 caracteres, '.' = casa vazia
    vezDasBrancas: boolean
    situacao: SituacaoPartida // como está a partida: em andamento, mate, afogamento, empates...
    ultimoLance: InfoLance    // o que o último lance fez (roque, en passant, captura...); uci === '' se não houve
}

export interface Partida extends EstadoDaPartida {
    pronta: boolean                              // false enquanto o WebAssembly carrega
    lancesDe: (casa: string) => string[]         // ex.: lancesDe("e2") -> ["e2e3", "e2e4"]; [] se a partida terminou
    tentarLance: (uci: string) => boolean        // true se o motor aceitou e atualizou a posição; false se a partida terminou
    jogarIA: (profundidade: number) => string | null // calcula e aplica a resposta das pretas; null se não jogou
    carregarFen: (fen: string) => boolean        // true se o FEN era válido e foi carregado
    reiniciar: () => void                        // volta para a posição inicial
}

// Valores de partida: usados enquanto o WebAssembly ainda não carregou.
const SITUACAO_INICIAL: SituacaoPartida = { estado: 'em_andamento', vencedor: '', emXeque: false, terminou: false }
const ULTIMO_LANCE_VAZIO: InfoLance = {
    uci: '', tipo: '', captura: false, pecaCapturada: '',
    casaCapturada: '', promocao: '', torreOrigem: '', torreDestino: '',
}

const ESTADO_VAZIO: EstadoDaPartida = {
    fen: '',
    tabuleiro: [],
    vezDasBrancas: true,
    situacao: SITUACAO_INICIAL,
    ultimoLance: ULTIMO_LANCE_VAZIO,
}

// situacao() e ultimoLance() devolvem objetos JavaScript comuns (o embind COPIA os campos),
// então não há nada para dar .delete() aqui, ao contrário do próprio Jogo.
function lerEstado(jogo: JogoWasm): EstadoDaPartida {
    return {
        fen: jogo.fen(),
        tabuleiro: jogo.tabuleiro(),
        vezDasBrancas: jogo.vezDasBrancas(),
        situacao: jogo.situacao(),
        ultimoLance: jogo.ultimoLance(),
    }
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
    const ultimaPosicaoCalculadaPelaIA = useRef<string | null>(null)

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

    // O motor só recusa lances ILEGAIS. Empates como repetição, regra dos 50 e material
    // insuficiente continuam permitindo lances, então quem "tranca" a partida é o frontend.
    // Fazemos isso aqui, no hook, para valer para qualquer componente que use a partida.
    const terminou = estado.situacao.terminou

    const lancesDe = useCallback(
        (casa: string) => (jogo && !terminou ? jogo.lancesDe(casa) : []),
        [jogo, terminou],
    )

    const tentarLance = useCallback(
        (uci: string) => {
            if (!jogo || terminou) return false
            const aceito = jogo.tentarLance(uci)       // o MOTOR decide se o lance vale
            if (aceito) setEstado(lerEstado(jogo))     // nova cópia -> React redesenha
            return aceito
        },
        [jogo, terminou],
    )

    const jogarIA = useCallback(
        (profundidade: number) => {
            if (
                !jogo ||
                terminou ||
                jogo.vezDasBrancas() ||
                !Number.isInteger(profundidade) ||
                profundidade < 1
            ) return null

            const fenAtual = jogo.fen()
            if (ultimaPosicaoCalculadaPelaIA.current === fenAtual) return null
            ultimaPosicaoCalculadaPelaIA.current = fenAtual

            const uci = jogo.melhorLanceIA(profundidade)
            if (!uci || !jogo.tentarLance(uci)) return null

            setEstado(lerEstado(jogo))
            return uci
        },
        [jogo, terminou],
    )

    const carregarFen = useCallback(
        (fen: string) => {
            if (!jogo) return false
            const carregado = jogo.carregarFen(fen)
            if (carregado) {
                ultimaPosicaoCalculadaPelaIA.current = null
                setEstado(lerEstado(jogo))
            }
            return carregado
        },
        [jogo],
    )

    const reiniciar = useCallback(() => {
        carregarFen(FEN_INICIAL)
    }, [carregarFen])

    return { pronta: jogo !== null, ...estado, lancesDe, tentarLance, jogarIA, carregarFen, reiniciar }
}
