// INTEGRAÇÃO (camada 1): contrato entre o front e o motor de regras REAL (chess_engine.wasm).
//
// Sem React aqui. O objetivo é travar o que o front ASSUME sobre o motor
// (formatos de texto, campos dos objetos, tipos de lance, estados da partida), que é
// exatamente o que está descrito em src/wasm/chess_engine.d.ts.
// Se alguém recompilar o motor e mudar algo, o primeiro teste a quebrar é daqui.

import { beforeAll, describe, expect, it } from 'vitest'
import type { EstadoDoJogo, InfoLance, SituacaoPartida } from '../chess_engine.js'
import {
    carregarMotorReal, ESTADOS_CONHECIDOS_PELO_FRONT, FEN_INICIAL, FENS,
    jogarTodos, novoJogo, todosOsLances,
} from './motorReal'

const OITO_LANCES_DE_REPETICAO = ['g1f3', 'g8f6', 'f3g1', 'f6g8', 'g1f3', 'g8f6', 'f3g1', 'f6g8']

beforeAll(async () => {
    await carregarMotorReal()   // compila o WASM uma vez; pode demorar na primeira execução
}, 30_000)

describe('Contrato motor <-> front: o objeto Jogo', () => {
    it('nasce na posição inicial, com as brancas para jogar', async () => {
        const jogo = await novoJogo()
        expect(jogo.fen()).toBe(FEN_INICIAL)
        expect(jogo.vezDasBrancas()).toBe(true)
        expect(jogo.tabuleiro()).toEqual([
            'rnbqkbnr', 'pppppppp', '........', '........', '........', '........', 'PPPPPPPP', 'RNBQKBNR',
        ])
    })

    it('tabuleiro(): sempre 8 textos de 8 caracteres, só com símbolos de peça ou "."', async () => {
        const jogo = await novoJogo()
        const posicoes = [FEN_INICIAL, FENS.roques, FENS.enPassant, FENS.capturaPromocao, FENS.cravada]
        for (const fen of posicoes) {
            expect(jogo.carregarFen(fen)).toBe(true)
            const tabuleiro = jogo.tabuleiro()
            expect(tabuleiro).toHaveLength(8)
            for (const linha of tabuleiro) expect(linha).toMatch(/^[pnbrqkPNBRQK.]{8}$/)
        }
    })

    it('tabuleiro() devolve uma cópia: mexer no array não altera o motor', async () => {
        const jogo = await novoJogo()
        const copia = jogo.tabuleiro()
        copia[0] = 'XXXXXXXX'
        expect(jogo.tabuleiro()[0]).toBe('rnbqkbnr')
    })

    it('depois de delete() o objeto não pode mais ser usado (o front precisa liberar só uma vez)', async () => {
        const jogo = await novoJogo()
        jogo.delete()
        expect(() => jogo.fen()).toThrow()
    })
})

describe('Contrato motor <-> front: lancesDe(casa)', () => {
    it('posição inicial: 20 lances legais no total', async () => {
        expect(todosOsLances(await novoJogo())).toHaveLength(20)
    })

    it('peão e2 inicial: e2e3 e e2e4 (não dependemos da ordem)', async () => {
        const jogo = await novoJogo()
        expect([...jogo.lancesDe('e2')].sort()).toEqual(['e2e3', 'e2e4'])
    })

    it('casa vazia, peça do adversário e casa inexistente devolvem lista vazia', async () => {
        const jogo = await novoJogo()
        expect(jogo.lancesDe('e4')).toEqual([])
        expect(jogo.lancesDe('e7')).toEqual([])   // peça preta na vez das brancas
        expect(jogo.lancesDe('zz')).toEqual([])
    })

    it('todo lance vem em UCI: 4 caracteres, ou 5 com a promoção em minúscula', async () => {
        const jogo = await novoJogo(FENS.capturaPromocao)
        const lances = todosOsLances(jogo)
        for (const uci of lances) expect(uci).toMatch(/^[a-h][1-8][a-h][1-8][qrbn]?$/)
        expect(jogo.lancesDe('a7').sort()).toEqual([
            'a7a8b', 'a7a8n', 'a7a8q', 'a7a8r', 'a7b8b', 'a7b8n', 'a7b8q', 'a7b8r',
        ])
    })

    it('peça cravada só anda na linha da cravada', async () => {
        const jogo = await novoJogo(FENS.cravada)
        expect([...jogo.lancesDe('e2')].sort()).toEqual(['e2e3', 'e2e4', 'e2e5', 'e2e6', 'e2e7', 'e2e8'])
    })

    it('em xeque, só aparecem lances que resolvem o xeque', async () => {
        const jogo = await novoJogo()
        jogarTodos(jogo, ['e2e4', 'f7f6', 'd1h5'])   // Dh5+ : o rei preto só se salva com g7-g6
        expect(todosOsLances(jogo)).toEqual(['g7g6'])
    })
})

describe('Contrato motor <-> front: tentarLance(uci)', () => {
    it('aceita lance legal (true) e recusa ilegal (false) sem mexer na posição', async () => {
        const jogo = await novoJogo()
        expect(jogo.tentarLance('e2e5')).toBe(false)   // peão não anda 3
        expect(jogo.tentarLance('e7e5')).toBe(false)   // peça preta na vez das brancas
        expect(jogo.fen()).toBe(FEN_INICIAL)
        expect(jogo.tentarLance('e2e4')).toBe(true)
        expect(jogo.vezDasBrancas()).toBe(false)
    })

    it.each(['', 'e2', 'e2e4e4', 'E2E4', 'e2e4 ', 'zz99'])('texto inválido %j é recusado', async (uci) => {
        const jogo = await novoJogo()
        expect(jogo.tentarLance(uci)).toBe(false)
        expect(jogo.fen()).toBe(FEN_INICIAL)
    })

    it('promoção exige a letra da peça: a7a8 é recusado, a7a8q é aceito', async () => {
        const jogo = await novoJogo(FENS.promocao)
        expect(jogo.tentarLance('a7a8')).toBe(false)
        expect(jogo.tentarLance('a7a8q')).toBe(true)
        expect(jogo.tabuleiro()[0]).toBe('Q.......')
    })

    it('lance que deixaria o próprio rei em xeque é recusado', async () => {
        const jogo = await novoJogo(FENS.cravada)
        expect(jogo.tentarLance('e2d2')).toBe(false)   // abandonaria a cravada
    })
})

describe('Contrato motor <-> front: carregarFen', () => {
    it('FEN inválido devolve false e preserva a posição atual', async () => {
        const jogo = await novoJogo()
        jogarTodos(jogo, ['e2e4'])
        const antes = jogo.fen()
        for (const fen of ['', 'lixo', '8/8/8/8/8/8/8/8 w - - 0 1', 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR x KQkq - 0 1']) {
            expect(jogo.carregarFen(fen)).toBe(false)
        }
        expect(jogo.fen()).toBe(antes)
    })

    it('FEN válido troca a posição e zera o ultimoLance', async () => {
        const jogo = await novoJogo()
        jogarTodos(jogo, ['e2e4'])
        expect(jogo.carregarFen(FEN_INICIAL)).toBe(true)
        expect(jogo.fen()).toBe(FEN_INICIAL)
        expect(jogo.ultimoLance().uci).toBe('')
        expect(jogo.ultimoLance().tipo).toBe('')
    })
})

describe('Contrato motor <-> front: situacao()', () => {
    it('é um objeto comum com exatamente os 4 campos do d.ts', async () => {
        const situacao = (await novoJogo()).situacao()
        expect(Object.keys(situacao).sort()).toEqual(['emXeque', 'estado', 'terminou', 'vencedor'])
        expect(typeof situacao.estado).toBe('string')
        expect(typeof situacao.vencedor).toBe('string')
        expect(typeof situacao.emXeque).toBe('boolean')
        expect(typeof situacao.terminou).toBe('boolean')
    })

    interface CasoSituacao {
        nome: string
        fen: string
        lances: string[]
        esperado: SituacaoPartida
    }

    const CASOS: CasoSituacao[] = [
        {
            nome: 'posição inicial', fen: FEN_INICIAL, lances: [],
            esperado: { estado: 'em_andamento', vencedor: '', emXeque: false, terminou: false },
        },
        {
            nome: 'xeque simples (Dh5+)', fen: FEN_INICIAL, lances: ['e2e4', 'f7f6', 'd1h5'],
            esperado: { estado: 'em_andamento', vencedor: '', emXeque: true, terminou: false },
        },
        {
            nome: 'xeque-mate das pretas (mate do louco)', fen: FEN_INICIAL, lances: ['f2f3', 'e7e5', 'g2g4', 'd8h4'],
            esperado: { estado: 'xeque_mate', vencedor: 'pretas', emXeque: true, terminou: true },
        },
        {
            nome: 'xeque-mate das brancas (mate pastor)', fen: FEN_INICIAL,
            lances: ['e2e4', 'e7e5', 'd1h5', 'b8c6', 'f1c4', 'g8f6', 'h5f7'],
            esperado: { estado: 'xeque_mate', vencedor: 'brancas', emXeque: true, terminou: true },
        },
        {
            nome: 'afogamento', fen: FENS.quaseAfogamento, lances: ['a7f7'],
            esperado: { estado: 'afogamento', vencedor: '', emXeque: false, terminou: true },
        },
        {
            nome: 'material insuficiente (rei x rei)', fen: FENS.quaseMaterialInsuficiente, lances: ['e1e2'],
            esperado: { estado: 'material_insuficiente', vencedor: '', emXeque: false, terminou: true },
        },
        {
            nome: 'regra dos 50 lances', fen: FENS.quaseRegra50, lances: ['a2a3'],
            esperado: { estado: 'regra_dos_50', vencedor: '', emXeque: false, terminou: true },
        },
        {
            nome: 'repetição tripla', fen: FEN_INICIAL, lances: OITO_LANCES_DE_REPETICAO,
            esperado: { estado: 'repeticao', vencedor: '', emXeque: false, terminou: true },
        },
    ]

    it.each(CASOS)('$nome', async ({ fen, lances, esperado }) => {
        const jogo = await novoJogo(fen)
        jogarTodos(jogo, lances)
        expect(jogo.situacao()).toEqual(esperado)
    })

    it('a repetição só é declarada no 8º meio-lance, não antes', async () => {
        const jogo = await novoJogo()
        jogarTodos(jogo, OITO_LANCES_DE_REPETICAO.slice(0, 7))
        expect(jogo.situacao().estado).toBe('em_andamento')
        jogarTodos(jogo, OITO_LANCES_DE_REPETICAO.slice(7))
        expect(jogo.situacao().estado).toBe('repeticao')
    })

    it('FEN que já nasce terminada é reportada como terminada logo após carregarFen', async () => {
        const jogo = await novoJogo(FENS.jaAfogada)
        expect(jogo.situacao()).toEqual({ estado: 'afogamento', vencedor: '', emXeque: false, terminou: true })
    })

    // IMPORTANTE: este é o motivo de o usePartida "trancar" a partida no front.
    it.each([
        { nome: 'material insuficiente', fen: FENS.quaseMaterialInsuficiente, lances: ['e1e2'], proximo: 'g5g4' },
        { nome: 'regra dos 50', fen: FENS.quaseRegra50, lances: ['a2a3'], proximo: 'e8e7' },
        { nome: 'repetição', fen: FEN_INICIAL, lances: OITO_LANCES_DE_REPETICAO, proximo: 'g1f3' },
    ])('empate por $nome: o motor AINDA aceita lances (quem tranca é o front)', async ({ fen, lances, proximo }) => {
        const jogo = await novoJogo(fen)
        jogarTodos(jogo, lances)
        expect(jogo.situacao().terminou).toBe(true)
        expect(todosOsLances(jogo).length).toBeGreaterThan(0)
        expect(jogo.tentarLance(proximo)).toBe(true)
    })

    it('xeque-mate e afogamento: o motor não tem lance legal nenhum', async () => {
        const mate = await novoJogo()
        jogarTodos(mate, ['f2f3', 'e7e5', 'g2g4', 'd8h4'])
        expect(todosOsLances(mate)).toEqual([])

        const afogado = await novoJogo(FENS.quaseAfogamento)
        jogarTodos(afogado, ['a7f7'])
        expect(todosOsLances(afogado)).toEqual([])
    })

    it('todos os estados observados são tratados pelo front (textoDaBarra do Game.tsx)', async () => {
        const observados = new Set<EstadoDoJogo>()
        for (const { fen, lances } of CASOS) {
            const jogo = await novoJogo(fen)
            jogarTodos(jogo, lances)
            observados.add(jogo.situacao().estado)
        }
        for (const estado of observados) expect(ESTADOS_CONHECIDOS_PELO_FRONT).toContain(estado)
    })

    // O fonte atual de engine/src/game/jogo.cpp já pode devolver "limite_de_lances", mas o
    // chess_engine.wasm embutido no front é anterior e o front não conhece esse estado
    // (EstadoDoJogo e textoDaBarra). Quando o WASM for recompilado, trate esse caso.
    it.todo('estado "limite_de_lances" (só no fonte C++ novo): front precisa de texto na barra e de tipo no d.ts')
})

describe('Contrato motor <-> front: ultimoLance()', () => {
    it('é um objeto comum com exatamente os 8 campos do d.ts', async () => {
        const jogo = await novoJogo()
        jogarTodos(jogo, ['e2e4'])
        const info = jogo.ultimoLance()
        expect(Object.keys(info).sort()).toEqual([
            'captura', 'casaCapturada', 'pecaCapturada', 'promocao', 'tipo', 'torreDestino', 'torreOrigem', 'uci',
        ])
        expect(typeof info.captura).toBe('boolean')
        for (const campo of ['uci', 'tipo', 'pecaCapturada', 'casaCapturada', 'promocao', 'torreOrigem', 'torreDestino'] as const) {
            expect(typeof info[campo]).toBe('string')
        }
    })

    it('antes de qualquer lance, tudo vazio', async () => {
        expect((await novoJogo()).ultimoLance()).toEqual({
            uci: '', tipo: '', captura: false, pecaCapturada: '', casaCapturada: '',
            promocao: '', torreOrigem: '', torreDestino: '',
        })
    })

    const VAZIO = { captura: false, pecaCapturada: '', casaCapturada: '', promocao: '', torreOrigem: '', torreDestino: '' }

    interface CasoLance { nome: string; fen: string; lances: string[]; esperado: InfoLance }

    const CASOS: CasoLance[] = [
        { nome: 'lance normal', fen: FEN_INICIAL, lances: ['g1f3'], esperado: { ...VAZIO, uci: 'g1f3', tipo: 'normal' } },
        { nome: 'avanço duplo do peão', fen: FEN_INICIAL, lances: ['e2e4'], esperado: { ...VAZIO, uci: 'e2e4', tipo: 'avanco_duplo' } },
        {
            nome: 'captura', fen: FEN_INICIAL, lances: ['e2e4', 'd7d5', 'e4d5'],
            esperado: { ...VAZIO, uci: 'e4d5', tipo: 'captura', captura: true, pecaCapturada: 'p', casaCapturada: 'd5' },
        },
        {
            nome: 'roque curto das brancas', fen: FENS.roques, lances: ['e1g1'],
            esperado: { ...VAZIO, uci: 'e1g1', tipo: 'roque_rei', torreOrigem: 'h1', torreDestino: 'f1' },
        },
        {
            nome: 'roque longo das brancas', fen: FENS.roques, lances: ['e1c1'],
            esperado: { ...VAZIO, uci: 'e1c1', tipo: 'roque_dama', torreOrigem: 'a1', torreDestino: 'd1' },
        },
        {
            // a casa capturada (d5) NÃO é a casa de destino (d6): é o ponto que o d.ts destaca
            nome: 'en passant', fen: FENS.enPassant, lances: ['e5d6'],
            esperado: { ...VAZIO, uci: 'e5d6', tipo: 'en_passant', captura: true, pecaCapturada: 'p', casaCapturada: 'd5' },
        },
        {
            nome: 'promoção', fen: FENS.promocao, lances: ['a7a8q'],
            esperado: { ...VAZIO, uci: 'a7a8q', tipo: 'promocao', promocao: 'q' },
        },
        {
            nome: 'captura com promoção', fen: FENS.capturaPromocao, lances: ['a7b8n'],
            esperado: { ...VAZIO, uci: 'a7b8n', tipo: 'captura_promocao', captura: true, pecaCapturada: 'n', casaCapturada: 'b8', promocao: 'n' },
        },
    ]

    it.each(CASOS)('$nome', async ({ fen, lances, esperado }) => {
        const jogo = await novoJogo(fen)
        jogarTodos(jogo, lances)
        expect(jogo.ultimoLance()).toEqual(esperado)
    })

    it('lance recusado não altera o ultimoLance', async () => {
        const jogo = await novoJogo()
        jogarTodos(jogo, ['e2e4'])
        jogo.tentarLance('e7e2')   // ilegal
        expect(jogo.ultimoLance().uci).toBe('e2e4')
    })
})