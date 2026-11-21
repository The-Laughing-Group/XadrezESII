// INTEGRAÇÃO (camada 2): hook usePartida <-> motor de regras REAL.
//
// O usePartida é a ponte entre o React e o objeto C++. Aqui ele roda de verdade, com o
// chess_engine.wasm de verdade. Um segundo Jogo "cru" faz o papel de ORÁCULO: tocamos
// os mesmos lances nele e conferimos que o que o React guardou é igual ao que o motor diz.

import { act, renderHook, waitFor, type RenderHookResult } from '@testing-library/react'
import { afterEach, beforeAll, beforeEach, describe, expect, it, vi } from 'vitest'
import { usePartida, type Partida } from '../../hooks/usePartida'
import type { JogoWasm } from '../chess_engine.js'
import {
    carregarMotorReal, FEN_INICIAL, FENS, instalarDubles, jogarTodos, novoJogo, todosOsLances, WorkerFalso,
} from './motorReal'

type Hook = RenderHookResult<Partida, unknown>

const OITO_LANCES_DE_REPETICAO = ['g1f3', 'g8f6', 'f3g1', 'f6g8', 'g1f3', 'g8f6', 'f3g1', 'f6g8'] as const
const MATE_DO_LOUCO = ['f2f3', 'e7e5', 'g2g4', 'd8h4'] as const
const MATE_PASTOR = ['e2e4', 'e7e5', 'd1h5', 'b8c6', 'f1c4', 'g8f6', 'h5f7'] as const

beforeAll(async () => {
    await carregarMotorReal()
}, 30_000)

beforeEach(() => instalarDubles())
afterEach(() => vi.unstubAllGlobals())

// ----- ajudantes ------------------------------------------------------------

async function montarPartida(): Promise<Hook> {
    const hook = renderHook(() => usePartida())
    await waitFor(() => expect(hook.result.current.pronta).toBe(true), { timeout: 15_000 })
    return hook
}

// Os callbacks do hook mudam de identidade a cada estado novo: sempre ler result.current de novo.
function jogar(hook: Hook, uci: string): boolean {
    let aceito = false
    act(() => { aceito = hook.result.current.tentarLance(uci) })
    return aceito
}

function jogarVarios(hook: Hook, lances: readonly string[]): void {
    for (const uci of lances) expect(jogar(hook, uci), `o hook deveria aceitar ${uci}`).toBe(true)
}

function carregar(hook: Hook, fen: string): boolean {
    let ok = false
    act(() => { ok = hook.result.current.carregarFen(fen) })
    return ok
}

/** O que o React guardou tem que ser IGUAL ao que o motor diz, campo por campo. */
function expectIgualAoMotor(partida: Partida, motor: JogoWasm): void {
    expect(partida.fen).toBe(motor.fen())
    expect(partida.tabuleiro).toEqual(motor.tabuleiro())
    expect(partida.vezDasBrancas).toBe(motor.vezDasBrancas())
    expect(partida.situacao).toEqual(motor.situacao())
    expect(partida.ultimoLance).toEqual(motor.ultimoLance())
}

// ----- testes ---------------------------------------------------------------

describe('usePartida <-> motor real: carregamento', () => {
    it('começa "não pronta" (sem aceitar lances) e, quando o WASM carrega, espelha a posição inicial', async () => {
        const hook = renderHook(() => usePartida())
        expect(hook.result.current.pronta).toBe(false)
        expect(hook.result.current.lancesDe('e2')).toEqual([])
        expect(hook.result.current.tentarLance('e2e4')).toBe(false)

        await waitFor(() => expect(hook.result.current.pronta).toBe(true), { timeout: 15_000 })
        expectIgualAoMotor(hook.result.current, await novoJogo())
        expect(hook.result.current.fen).toBe(FEN_INICIAL)
    })
})

describe('usePartida <-> motor real: lances', () => {
    it('lance aceito pelo motor: devolve true e o React recebe a nova posição', async () => {
        const hook = await montarPartida()
        const oraculo = await novoJogo()

        expect(jogar(hook, 'e2e4')).toBe(true)
        oraculo.tentarLance('e2e4')

        expectIgualAoMotor(hook.result.current, oraculo)
        expect(hook.result.current.vezDasBrancas).toBe(false)
        expect(hook.result.current.ultimoLance.tipo).toBe('avanco_duplo')
    })

    it('lance recusado pelo motor: devolve false e o estado do React não muda', async () => {
        const hook = await montarPartida()
        const antes = hook.result.current

        for (const ilegal of ['e2e5', 'e7e5', 'e1e2', 'zz99', '', 'e2e4e4']) {
            expect(jogar(hook, ilegal), `não deveria aceitar ${JSON.stringify(ilegal)}`).toBe(false)
        }
        expect(hook.result.current.fen).toBe(antes.fen)
        expect(hook.result.current.ultimoLance.uci).toBe('')
    })

    it('lancesDe(casa) é exatamente o do motor, em qualquer casa, antes e depois de um lance', async () => {
        const hook = await montarPartida()
        const oraculo = await novoJogo()

        const conferirTodasAsCasas = () => {
            for (const coluna of 'abcdefgh') {
                for (const linha of '12345678') {
                    const casa = coluna + linha
                    expect([...hook.result.current.lancesDe(casa)].sort()).toEqual([...oraculo.lancesDe(casa)].sort())
                }
            }
        }

        conferirTodasAsCasas()
        jogar(hook, 'e2e4')
        oraculo.tentarLance('e2e4')
        conferirTodasAsCasas()
    })

    it('uma partida inteira (mate pastor): a cada meio-lance o React está igual ao motor', async () => {
        const hook = await montarPartida()
        const oraculo = await novoJogo()

        for (const uci of MATE_PASTOR) {
            expect(jogar(hook, uci)).toBe(true)
            expect(oraculo.tentarLance(uci)).toBe(true)
            expectIgualAoMotor(hook.result.current, oraculo)
        }
        expect(hook.result.current.situacao).toEqual({ estado: 'xeque_mate', vencedor: 'brancas', emXeque: true, terminou: true })
    })

    it('roque, en passant e promoção chegam ao React com as informações de ultimoLance', async () => {
        const hook = await montarPartida()

        expect(carregar(hook, FENS.roques)).toBe(true)
        jogarVarios(hook, ['e1g1'])
        expect(hook.result.current.ultimoLance).toMatchObject({ tipo: 'roque_rei', torreOrigem: 'h1', torreDestino: 'f1' })
        expect(hook.result.current.tabuleiro[7]).toBe('R....RK.')

        expect(carregar(hook, FENS.enPassant)).toBe(true)
        jogarVarios(hook, ['e5d6'])
        expect(hook.result.current.ultimoLance).toMatchObject({ tipo: 'en_passant', casaCapturada: 'd5', pecaCapturada: 'p' })

        expect(carregar(hook, FENS.promocao)).toBe(true)
        expect(jogar(hook, 'a7a8')).toBe(false)           // sem a letra, o motor não aceita
        jogarVarios(hook, ['a7a8q'])
        expect(hook.result.current.ultimoLance).toMatchObject({ tipo: 'promocao', promocao: 'q' })
        expect(hook.result.current.tabuleiro[0]).toBe('Q.......')
    })
})

describe('usePartida <-> motor real: fim de partida e trava do front', () => {
    it('xeque-mate: o React recebe o estado e trava (sem lances, sem aceitar mais nada)', async () => {
        const hook = await montarPartida()
        jogarVarios(hook, MATE_DO_LOUCO)

        expect(hook.result.current.situacao).toEqual({ estado: 'xeque_mate', vencedor: 'pretas', emXeque: true, terminou: true })
        expect(hook.result.current.lancesDe('e1')).toEqual([])
        expect(jogar(hook, 'e2e4')).toBe(false)
    })

    it('afogamento: estado terminal e trava', async () => {
        const hook = await montarPartida()
        expect(carregar(hook, FENS.quaseAfogamento)).toBe(true)
        jogarVarios(hook, ['a7f7'])

        expect(hook.result.current.situacao.estado).toBe('afogamento')
        expect(hook.result.current.situacao.terminou).toBe(true)
        expect(jogar(hook, 'h8g8')).toBe(false)
    })

    // O motor, sozinho, deixaria a partida seguir depois destes empates (ver motor.contrato.test.ts).
    // Quem tranca é o usePartida. Se alguém tirar essa trava, estes testes quebram.
    it.each([
        { nome: 'material insuficiente', estado: 'material_insuficiente', fen: FENS.quaseMaterialInsuficiente, lances: ['e1e2'], proximo: 'g5g4', casa: 'g5' },
        { nome: 'regra dos 50', estado: 'regra_dos_50', fen: FENS.quaseRegra50, lances: ['a2a3'], proximo: 'e8e7', casa: 'e8' },
        { nome: 'repetição tripla', estado: 'repeticao', fen: FEN_INICIAL, lances: OITO_LANCES_DE_REPETICAO, proximo: 'g1f3', casa: 'g1' },
    ])('empate por $nome: o motor ainda deixaria jogar, mas o hook trava', async ({ estado, fen, lances, proximo, casa }) => {
        const hook = await montarPartida()
        if (fen !== FEN_INICIAL) expect(carregar(hook, fen)).toBe(true)
        jogarVarios(hook, lances)
        expect(hook.result.current.situacao.estado).toBe(estado)

        // prova de que o MOTOR não impediria:
        const oraculo = await novoJogo(fen)
        jogarTodos(oraculo, [...lances])
        expect(oraculo.lancesDe(casa).length).toBeGreaterThan(0)

        // o hook impede:
        expect(jogar(hook, proximo)).toBe(false)
        expect(hook.result.current.lancesDe(casa)).toEqual([])
        expect(hook.result.current.fen).toBe(oraculo.fen())   // nada foi aplicado
    })

    it('a trava some quando se carrega uma posição nova (carregarFen) ou se reinicia', async () => {
        const hook = await montarPartida()
        jogarVarios(hook, MATE_DO_LOUCO)
        expect(jogar(hook, 'e2e4')).toBe(false)

        act(() => hook.result.current.reiniciar())
        expect(hook.result.current.situacao.terminou).toBe(false)
        expect(jogar(hook, 'e2e4')).toBe(true)
    })
})

describe('usePartida <-> motor real: carregarFen e reiniciar', () => {
    it('FEN válido: devolve true e o React passa a espelhar a nova posição', async () => {
        const hook = await montarPartida()
        expect(carregar(hook, FENS.cravada)).toBe(true)
        expectIgualAoMotor(hook.result.current, await novoJogo(FENS.cravada))
        expect([...hook.result.current.lancesDe('e2')].sort()).toEqual(['e2e3', 'e2e4', 'e2e5', 'e2e6', 'e2e7', 'e2e8'])
    })

    it('FEN inválido: devolve false e preserva a posição que estava na tela', async () => {
        const hook = await montarPartida()
        jogarVarios(hook, ['e2e4'])
        const antes = hook.result.current.fen

        expect(carregar(hook, 'isto não é um FEN')).toBe(false)
        expect(hook.result.current.fen).toBe(antes)
    })

    it('reiniciar volta à posição inicial e limpa o último lance', async () => {
        const hook = await montarPartida()
        jogarVarios(hook, ['e2e4', 'e7e5', 'g1f3'])

        act(() => hook.result.current.reiniciar())

        expectIgualAoMotor(hook.result.current, await novoJogo())
        expect(hook.result.current.ultimoLance.uci).toBe('')
    })
})

describe('usePartida <-> motor real: jogarIA (sem usar o módulo da IA)', () => {
    // O hook cria um Worker (só a IA usa). Aqui ele é falso: só conferimos que o hook NÃO
    // o aciona quando não é para a IA jogar.
    it('não aciona o worker na vez das brancas nem com profundidade inválida', async () => {
        const hook = await montarPartida()
        const worker = WorkerFalso.instancias[0]

        let resposta: string | null = 'x'
        await act(async () => { resposta = await hook.result.current.jogarIA(3) })
        expect(resposta).toBeNull()                 // vez das brancas

        jogarVarios(hook, ['e2e4'])                 // agora é a vez das pretas
        for (const profundidade of [0, -1, 2.5, Number.NaN]) {
            await act(async () => { resposta = await hook.result.current.jogarIA(profundidade) })
            expect(resposta).toBeNull()
        }
        expect(worker.postMessage).not.toHaveBeenCalled()
    })
})

describe('usePartida <-> motor real: sanidade', () => {
    it('depois do mate não existe nenhum lance legal no motor (conferência independente)', async () => {
        const oraculo = await novoJogo()
        jogarTodos(oraculo, [...MATE_DO_LOUCO])
        expect(todosOsLances(oraculo)).toEqual([])
    })
})