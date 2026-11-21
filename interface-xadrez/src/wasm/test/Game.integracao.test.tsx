// INTEGRAÇÃO (camada 3): tela Game COMPLETA <-> motor de regras REAL.
//
// Renderiza a página inteira (Game + Tabuleiro + Casa + BarraSuperior + HistoricoMovimentos +
// ModalFimJogo + ModalConfiguracoes + usePartida) e joga como uma pessoa: clicando nas casas.
// O motor C++ (chess_engine.wasm) é o de verdade. Nada de usePartida/Tabuleiro mockados.
//
// Só duas coisas são "de mentira", porque o jsdom não tem: Audio e Worker (ver motorReal.ts).
// Modo contra a IA fica de fora de propósito: aqui é só front <-> regras (2 jogadores).
//
// Para começar numa posição específica sem lances de preparação, o mock de '../hooks/useJogo'
// abaixo apenas carrega um FEN no Jogo REAL logo depois de criá-lo. O motor continua sendo o real.

import { screen, waitFor, within } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { afterEach, beforeAll, beforeEach, describe, expect, it, vi } from 'vitest'
import Game from '../../pages/Game'
import { renderComRotas } from '../../utils/testUtils'
import type { JogoWasm } from '../chess_engine.js'
import movimentoSom from '../../assets/sons/movimento.m4a'
import promocaoSom from '../../assets/sons/promocao.mp3'
import xequeSom from '../../assets/sons/xeque.mp3'
import fimjogoSom from '../../assets/sons/fimjogo.mp3'
import {
    AudioFalso, carregarMotorReal, casaEl, casaSelecionadaNaTela, criarSorteio, destinosMarcados,
    FEN_INICIAL, FENS, historicoDaTela, instalarDubles, pecaEm, tabuleiroDoDom, textoDoIndicador, todosOsLances,
} from './motorReal'

vi.setConfig({ testTimeout: 60_000 })

// vi.mock é "içado" para o topo do arquivo, por isso o estado compartilhado precisa de vi.hoisted.
const semente = vi.hoisted(() => ({
    fen: null as string | null,   // posição em que o próximo Jogo nasce (null = posição inicial)
    jogos: [] as JogoWasm[],      // os Jogos REAIS criados pela tela, para o teste conferir
}))

vi.mock('../../hooks/useJogo', async () => {
    const { carregarMotorReal: carregar } = await import('./motorReal')
    return {
        carregarModulo: async () => {
            const real = await carregar()
            return {
                // "new Jogo()" devolvendo um objeto faz o JS usar esse objeto: é o Jogo real, já posicionado.
                Jogo: function JogoSemeado() {
                    const jogo = new real.Jogo()
                    if (semente.fen && !jogo.carregarFen(semente.fen)) {
                        throw new Error(`FEN recusado pelo motor: ${semente.fen}`)
                    }
                    semente.jogos.push(jogo)
                    return jogo
                },
            }
        },
    }
})

beforeAll(async () => {
    await carregarMotorReal()
}, 30_000)

beforeEach(() => {
    instalarDubles()
    semente.fen = null
    semente.jogos = []
})
afterEach(() => vi.unstubAllGlobals())

// ----- ajudantes ------------------------------------------------------------

type Usuario = ReturnType<typeof userEvent.setup>

const BOTAO_PROMOCAO: Record<string, string> = { q: '♛', r: '♜', b: '♝', n: '♞' }

/** Abre a tela /game e espera o WebAssembly carregar. Devolve o Jogo REAL que a tela usa (só para LER). */
async function abrir(fen?: string) {
    semente.fen = fen ?? null
    const tela = renderComRotas(<Game />, '/game')
    await waitFor(() => {
        expect(document.querySelectorAll('.casa')).toHaveLength(64)
        expect(semente.jogos.length).toBeGreaterThan(0)
    }, { timeout: 15_000 })
    return { motor: semente.jogos[0], tela, usuario: userEvent.setup() }
}

/** Clica como uma pessoa: na peça, se houver; senão na casa vazia. */
async function clicar(usuario: Usuario, uci: string) {
    const casa = casaEl(uci)
    await usuario.click((casa.querySelector('img.peca') ?? casa.querySelector('.casa-area')) as HTMLElement)
}

/** Joga lances UCI pela tela: origem, destino e, se for promoção, o botão da peça. */
async function jogarNaTela(usuario: Usuario, ...lances: string[]) {
    for (const uci of lances) {
        await clicar(usuario, uci.slice(0, 2))
        await clicar(usuario, uci.slice(2, 4))
        if (uci.length === 5) await usuario.click(screen.getByRole('button', { name: BOTAO_PROMOCAO[uci[4]] }))
    }
}

const srcDaPeca = (uci: string) => casaEl(uci).querySelector('img.peca')?.getAttribute('src') ?? ''
const reisPiscando = () => document.querySelectorAll('img[src*="Danger"]').length

const REPETICAO = ['g1f3', 'g8f6', 'f3g1', 'f6g8', 'g1f3', 'g8f6', 'f3g1', 'f6g8']
const MATE_DO_LOUCO = ['f2f3', 'e7e5', 'g2g4', 'd8h4']

// ----- testes ---------------------------------------------------------------

describe('Game <-> motor real: posição inicial e seleção de peças', () => {
    it('desenha a posição inicial que o motor devolve', async () => {
        const { motor } = await abrir()

        expect(tabuleiroDoDom()).toEqual(motor.tabuleiro())
        expect(tabuleiroDoDom()).toEqual([
            'rnbqkbnr', 'pppppppp', '........', '........', '........', '........', 'PPPPPPPP', 'RNBQKBNR',
        ])
        expect(document.querySelectorAll('img.peca')).toHaveLength(32)
        expect(textoDoIndicador()).toBe('Vez das brancas')
        expect(historicoDaTela()).toEqual({ brancas: [], pretas: [] })
        expect(reisPiscando()).toBe(0)
    })

    it('ao tocar no peão e2, marca exatamente os destinos legais do motor', async () => {
        const { motor, usuario } = await abrir()
        await clicar(usuario, 'e2')

        expect(casaSelecionadaNaTela()).toBe('e2')
        expect(destinosMarcados()).toEqual(['e3', 'e4'])
        expect(destinosMarcados()).toEqual(motor.lancesDe('e2').map(l => l.slice(2, 4)).sort())
    })

    it('para CADA peça das brancas, os destinos marcados são os mesmos do motor', async () => {
        const { motor, usuario } = await abrir()
        const casasDasBrancas = ['a1', 'b1', 'c1', 'd1', 'e1', 'f1', 'g1', 'h1',
                                 'a2', 'b2', 'c2', 'd2', 'e2', 'f2', 'g2', 'h2']

        for (const casa of casasDasBrancas) {
            await clicar(usuario, casa)
            const esperado = [...new Set(motor.lancesDe(casa).map(l => l.slice(2, 4)))].sort()
            expect(destinosMarcados(), `destinos de ${casa}`).toEqual(esperado)
            await clicar(usuario, casa)   // clicar de novo desmarca, para o próximo não virar "movimento"
            expect(casaSelecionadaNaTela()).toBeNull()
        }
    })

    it('trocar de peça selecionada atualiza a seleção e os destinos', async () => {
        const { usuario } = await abrir()
        await clicar(usuario, 'e2')
        await clicar(usuario, 'g1')

        expect(casaSelecionadaNaTela()).toBe('g1')
        expect(destinosMarcados()).toEqual(['f3', 'h3'])
    })

    it('não deixa selecionar peça do adversário nem casa vazia', async () => {
        const { motor, usuario } = await abrir()

        await clicar(usuario, 'e7')   // peão preto na vez das brancas
        expect(casaSelecionadaNaTela()).toBeNull()
        expect(destinosMarcados()).toEqual([])

        await clicar(usuario, 'e4')   // casa vazia
        expect(casaSelecionadaNaTela()).toBeNull()
        expect(motor.fen()).toBe(FEN_INICIAL)
    })

    it('clicar numa casa que não é destino legal não move nada', async () => {
        const { motor, usuario } = await abrir()
        await clicar(usuario, 'e2')
        await clicar(usuario, 'e5')   // peão não chega lá

        expect(motor.fen()).toBe(FEN_INICIAL)
        expect(tabuleiroDoDom()).toEqual(motor.tabuleiro())
    })

    it('peça cravada: só aparecem destinos que não expõem o rei', async () => {
        const { motor, usuario } = await abrir(FENS.cravada)
        await clicar(usuario, 'e2')

        expect(destinosMarcados()).toEqual(['e3', 'e4', 'e5', 'e6', 'e7', 'e8'])   // nada em d2/f2
        expect(destinosMarcados()).toEqual(motor.lancesDe('e2').map(l => l.slice(2, 4)).sort())
    })
})

describe('Game <-> motor real: jogar lances', () => {
    it('e2-e4 pela tela: tabuleiro, vez, histórico e som seguem o motor', async () => {
        const { motor, usuario } = await abrir()
        await jogarNaTela(usuario, 'e2e4')

        expect(motor.fen()).toBe('rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1')
        expect(tabuleiroDoDom()).toEqual(motor.tabuleiro())
        expect(pecaEm('e4')).toBe('P')
        expect(pecaEm('e2')).toBe('.')
        expect(textoDoIndicador()).toBe('Vez das pretas')
        expect(historicoDaTela()).toEqual({ brancas: ['e4'], pretas: [] })
        expect(casaSelecionadaNaTela()).toBeNull()
        expect(destinosMarcados()).toEqual([])
        expect(AudioFalso.vezesQueTocou(movimentoSom)).toBe(1)
    })

    it('depois de e2-e4, só as pretas podem ser selecionadas', async () => {
        const { usuario } = await abrir()
        await jogarNaTela(usuario, 'e2e4')

        await clicar(usuario, 'e4')   // peão branco: não é a vez dele
        expect(casaSelecionadaNaTela()).toBeNull()

        await clicar(usuario, 'e7')
        expect(casaSelecionadaNaTela()).toBe('e7')
        expect(destinosMarcados()).toEqual(['e5', 'e6'])
    })

    it('captura de peão: tabuleiro e histórico (exd5)', async () => {
        const { motor, usuario } = await abrir()
        await jogarNaTela(usuario, 'e2e4', 'd7d5', 'e4d5')

        expect(tabuleiroDoDom()).toEqual(motor.tabuleiro())
        expect(pecaEm('d5')).toBe('P')
        expect(pecaEm('e4')).toBe('.')
        expect(historicoDaTela()).toEqual({ brancas: ['e4', 'exd5'], pretas: ['d5'] })
    })

    it('captura de peça: histórico com x (Nxe5)', async () => {
        const { usuario } = await abrir()
        await jogarNaTela(usuario, 'e2e4', 'e7e5', 'g1f3', 'b8c6', 'f3e5')

        expect(historicoDaTela()).toEqual({ brancas: ['e4', 'Nf3', 'Nxe5'], pretas: ['e5', 'Nc6'] })
    })

    it('dois cavalos podem ir à mesma casa: o histórico desambigua (Nbd2)', async () => {
        const { motor, usuario } = await abrir(FENS.doisCavalos)
        await jogarNaTela(usuario, 'b1d2')

        expect(tabuleiroDoDom()).toEqual(motor.tabuleiro())
        expect(historicoDaTela().brancas).toEqual(['Nbd2'])
    })

    it('en passant: o peão capturado SAI da casa certa (d5, não a de destino) e o histórico registra exd6', async () => {
        const { motor, usuario } = await abrir()
        await jogarNaTela(usuario, 'e2e4', 'a7a6', 'e4e5', 'd7d5')

        await clicar(usuario, 'e5')
        expect(destinosMarcados()).toEqual(['d6', 'e6'])   // d6 só existe por causa do en passant
        await clicar(usuario, 'd6')

        expect(motor.ultimoLance()).toMatchObject({ tipo: 'en_passant', casaCapturada: 'd5' })
        expect(tabuleiroDoDom()).toEqual(motor.tabuleiro())
        expect(pecaEm('d5')).toBe('.')
        expect(pecaEm('d6')).toBe('P')
        expect(historicoDaTela()).toEqual({ brancas: ['e4', 'e5', 'exd6'], pretas: ['a6', 'd5'] })
    })

    it('roque curto: rei vai para g1, torre para f1 e o histórico mostra 0-0', async () => {
        const { motor, usuario } = await abrir(FENS.roques)
        await clicar(usuario, 'e1')
        expect(destinosMarcados()).toEqual(['c1', 'd1', 'd2', 'e2', 'f1', 'f2', 'g1'])   // inclui g1 e c1 (roques)

        await clicar(usuario, 'g1')
        expect(tabuleiroDoDom()).toEqual(motor.tabuleiro())
        expect(tabuleiroDoDom()[7]).toBe('R....RK.')
        expect(historicoDaTela().brancas).toEqual(['0-0'])
    })

    it('roque longo: rei vai para c1, torre para d1 e o histórico mostra 0-0-0', async () => {
        const { motor, usuario } = await abrir(FENS.roques)
        await jogarNaTela(usuario, 'e1c1')

        expect(tabuleiroDoDom()).toEqual(motor.tabuleiro())
        expect(tabuleiroDoDom()[7]).toBe('..KR...R')
        expect(historicoDaTela().brancas).toEqual(['0-0-0'])
    })
})

describe('Game <-> motor real: promoção', () => {
    it('ao chegar na última fila aparecem os 4 botões e NADA é jogado até escolher', async () => {
        const { motor, usuario } = await abrir(FENS.promocao)
        await clicar(usuario, 'a7')
        await clicar(usuario, 'a8')

        for (const simbolo of Object.values(BOTAO_PROMOCAO)) {
            expect(screen.getByRole('button', { name: simbolo })).toBeInTheDocument()
        }
        expect(motor.fen()).toBe(FENS.promocao)
        expect(pecaEm('a7')).toBe('P')
        expect(textoDoIndicador()).toBe('Vez das brancas')
    })

    it.each([
        { letra: 'q', peca: 'Q' },
        { letra: 'r', peca: 'R' },
        { letra: 'b', peca: 'B' },
        { letra: 'n', peca: 'N' },
    ])('promover em $letra: a peça nova aparece, o histórico registra a8$peca e toca o som de promoção', async ({ letra, peca }) => {
        const { motor, usuario } = await abrir(FENS.promocao)
        await jogarNaTela(usuario, `a7a8${letra}`)

        expect(motor.ultimoLance()).toMatchObject({ tipo: 'promocao', promocao: letra })
        expect(tabuleiroDoDom()).toEqual(motor.tabuleiro())
        expect(pecaEm('a8')).toBe(peca)
        expect(pecaEm('a7')).toBe('.')
        expect(historicoDaTela().brancas).toEqual([`a8${peca}`])
        expect(AudioFalso.vezesQueTocou(promocaoSom)).toBe(1)
        expect(screen.queryByRole('button', { name: BOTAO_PROMOCAO[letra] })).not.toBeInTheDocument()
    })

    it('cancelar a escolha (⨉) desfaz tudo e o peão continua em a7', async () => {
        const { motor, usuario } = await abrir(FENS.promocao)
        await clicar(usuario, 'a7')
        await clicar(usuario, 'a8')
        await usuario.click(screen.getByRole('button', { name: '⨉' }))

        expect(screen.queryByRole('button', { name: '♛' })).not.toBeInTheDocument()
        expect(motor.fen()).toBe(FENS.promocao)
        expect(pecaEm('a7')).toBe('P')
        expect(casaSelecionadaNaTela()).toBeNull()
    })

    it('captura com promoção (axb8): o motor reconhece e o histórico mostra axb8N', async () => {
        const { motor, usuario } = await abrir(FENS.capturaPromocao)
        await clicar(usuario, 'a7')
        expect(destinosMarcados()).toEqual(['a8', 'b8'])
        await clicar(usuario, 'b8')
        await usuario.click(screen.getByRole('button', { name: BOTAO_PROMOCAO.n }))

        expect(motor.ultimoLance()).toMatchObject({ tipo: 'captura_promocao', promocao: 'n', pecaCapturada: 'n' })
        expect(pecaEm('b8')).toBe('N')
        expect(historicoDaTela().brancas).toEqual(['axb8N'])
    })
})

describe('Game <-> motor real: xeque', () => {
    it('xeque: texto da barra, rei piscando, som de xeque e só os lances que resolvem', async () => {
        const { motor, usuario } = await abrir()
        await jogarNaTela(usuario, 'e2e4', 'f7f6', 'd1h5')   // Dh5+

        expect(motor.situacao().emXeque).toBe(true)
        expect(textoDoIndicador()).toBe('Xeque! Vez das pretas')
        expect(reisPiscando()).toBe(1)
        expect(srcDaPeca('e8')).toContain("Danger")
        expect(srcDaPeca('e1')).not.toContain("Danger")
        expect(historicoDaTela()).toEqual({ brancas: ['e4', 'Qh5'], pretas: ['f6'] })
        expect(AudioFalso.vezesQueTocou(xequeSom)).toBe(1)
        expect(AudioFalso.vezesQueTocou(movimentoSom)).toBe(2)   // e4 e f6; o lance que dá xeque toca o som de xeque

        await clicar(usuario, 'g7')
        expect(destinosMarcados()).toEqual(['g6'])               // único jeito de se defender
        await clicar(usuario, 'g7')

        await clicar(usuario, 'e8')                              // o rei não tem para onde ir
        expect(destinosMarcados()).toEqual([])
        await clicar(usuario, 'e8')

        await clicar(usuario, 'a7')                              // outras peças não resolvem o xeque
        expect(destinosMarcados()).toEqual([])
    })

    it('ao resolver o xeque, a barra volta ao normal e o rei para de piscar', async () => {
        const { usuario } = await abrir()
        await jogarNaTela(usuario, 'e2e4', 'f7f6', 'd1h5', 'g7g6')

        expect(textoDoIndicador()).toBe('Vez das brancas')
        expect(reisPiscando()).toBe(0)
    })
})

describe('Game <-> motor real: fim de partida', () => {
    it('xeque-mate: barra, modal, histórico, som e tabuleiro travado', async () => {
        const { motor, usuario } = await abrir()
        await jogarNaTela(usuario, ...MATE_DO_LOUCO)

        expect(motor.situacao()).toEqual({ estado: 'xeque_mate', vencedor: 'pretas', emXeque: true, terminou: true })
        expect(textoDoIndicador()).toBe('Xeque-mate! Vencem as pretas')

        const modal = screen.getByRole('dialog')
        expect(within(modal).getByRole('heading')).toHaveTextContent(/Xeque-mate!\s+Vencem as pretas/)

        expect(historicoDaTela()).toEqual({ brancas: ['f3', 'g4'], pretas: ['e5', 'Qh4'] })
        expect(srcDaPeca('e1')).toContain("Danger")   // rei branco mateado
        expect(AudioFalso.vezesQueTocou(fimjogoSom)).toBe(1)
        expect(AudioFalso.vezesQueTocou(xequeSom)).toBe(0) // no mate toca só o som de fim de jogo

        await usuario.keyboard('{Escape}')                 // fecha o modal...
        expect(screen.queryByRole('dialog')).not.toBeInTheDocument()
        expect(textoDoIndicador()).toBe('Xeque-mate! Vencem as pretas')
        await clicar(usuario, 'e2')                        // ...mas o tabuleiro continua travado
        expect(casaSelecionadaNaTela()).toBeNull()
        expect(destinosMarcados()).toEqual([])
    })

    it('xeque-mate das brancas (mate pastor): vencedor certo na tela', async () => {
        const { usuario } = await abrir()
        await jogarNaTela(usuario, 'e2e4', 'e7e5', 'd1h5', 'b8c6', 'f1c4', 'g8f6', 'h5f7')

        expect(textoDoIndicador()).toBe('Xeque-mate! Vencem as brancas')
        expect(srcDaPeca('e8')).toContain("Danger")   // rei preto mateado
        expect(historicoDaTela().brancas.at(-1)).toBe('Qxf7')
    })

    it('afogamento: empate, modal aberto e partida travada', async () => {
        const { motor, usuario } = await abrir(FENS.quaseAfogamento)
        await jogarNaTela(usuario, 'a7f7')

        expect(motor.situacao().estado).toBe('afogamento')
        expect(textoDoIndicador()).toBe('Empate: afogamento')
        expect(screen.getByRole('dialog')).toHaveTextContent('Empate: afogamento')
        expect(AudioFalso.vezesQueTocou(fimjogoSom)).toBe(1)
    })

    it('material insuficiente: o motor ainda deixaria jogar, mas a tela trava', async () => {
        const { motor, usuario } = await abrir(FENS.quaseMaterialInsuficiente)
        await jogarNaTela(usuario, 'e1e2')   // Rxe2: sobram só os dois reis

        expect(textoDoIndicador()).toBe('Empate: material insuficiente')
        expect(screen.getByRole('dialog')).toHaveTextContent('Empate: material insuficiente')
        expect(motor.lancesDe('g5').length).toBeGreaterThan(0)   // o motor tem lances...

        await usuario.keyboard('{Escape}')
        await clicar(usuario, 'g5')                              // ...a tela não deixa pegar a peça
        expect(casaSelecionadaNaTela()).toBeNull()
        expect(destinosMarcados()).toEqual([])
    })

    it('regra dos 50 lances: empate declarado ao completar 100 meios-lances', async () => {
        const { usuario } = await abrir(FENS.quaseRegra50)
        expect(textoDoIndicador()).toBe('Vez das brancas')
        await jogarNaTela(usuario, 'a2a3')

        expect(textoDoIndicador()).toBe('Empate: regra dos 50 lances')
        expect(screen.getByRole('dialog')).toBeInTheDocument()
    })

    it('repetição tripla: só na 3ª vez (8º meio-lance), nunca antes', async () => {
        const { usuario } = await abrir()
        await jogarNaTela(usuario, ...REPETICAO.slice(0, 7))

        expect(textoDoIndicador()).toBe('Vez das pretas')
        expect(screen.queryByRole('dialog')).not.toBeInTheDocument()

        await jogarNaTela(usuario, REPETICAO[7])
        expect(textoDoIndicador()).toBe('Empate: repetição de posição')
        expect(screen.getByRole('dialog')).toBeInTheDocument()
    })

    it('posição que já nasce terminada (afogamento) abre o modal assim que carrega', async () => {
        const { motor } = await abrir(FENS.jaAfogada)

        expect(motor.situacao().estado).toBe('afogamento')
        expect(textoDoIndicador()).toBe('Empate: afogamento')
        expect(screen.getByRole('dialog')).toBeInTheDocument()
        expect(AudioFalso.vezesQueTocou(fimjogoSom)).toBe(1)
    })
})

describe('Game <-> motor real: reiniciar, navegar e liberar o motor', () => {
    it('Reiniciar no modal de fim de jogo volta à posição inicial e destrava o tabuleiro', async () => {
        const { motor, usuario } = await abrir()
        await jogarNaTela(usuario, ...MATE_DO_LOUCO)

        await usuario.click(within(screen.getByRole('dialog')).getByRole('button', { name: /Reiniciar/ }))

        expect(motor.fen()).toBe(FEN_INICIAL)
        expect(motor.ultimoLance().uci).toBe('')
        expect(tabuleiroDoDom()).toEqual(motor.tabuleiro())
        expect(textoDoIndicador()).toBe('Vez das brancas')
        expect(historicoDaTela()).toEqual({ brancas: [], pretas: [] })
        expect(screen.queryByRole('dialog')).not.toBeInTheDocument()
        expect(reisPiscando()).toBe(0)

        await jogarNaTela(usuario, 'e2e4')   // e dá para jogar de novo
        expect(historicoDaTela().brancas).toEqual(['e4'])
    })

    it('Reiniciar pelas Configurações no meio da partida zera tabuleiro e histórico', async () => {
        const { motor, usuario } = await abrir()
        await jogarNaTela(usuario, 'e2e4', 'e7e5')
        expect(historicoDaTela().brancas).toHaveLength(1)

        await usuario.click(screen.getByRole('button', { name: 'Configurações' }))
        await usuario.click(within(screen.getByRole('dialog')).getByRole('button', { name: /Reiniciar/ }))

        expect(motor.fen()).toBe(FEN_INICIAL)
        expect(tabuleiroDoDom()).toEqual(motor.tabuleiro())
        expect(historicoDaTela()).toEqual({ brancas: [], pretas: [] })
        expect(textoDoIndicador()).toBe('Vez das brancas')
        expect(screen.queryByRole('dialog')).not.toBeInTheDocument()
    })

    it('reiniciar no meio de uma seleção limpa a seleção e os destinos', async () => {
        const { usuario } = await abrir()
        await jogarNaTela(usuario, 'e2e4')
        await clicar(usuario, 'e7')
        expect(destinosMarcados()).toEqual(['e5', 'e6'])

        await usuario.click(screen.getByRole('button', { name: 'Configurações' }))
        await usuario.click(within(screen.getByRole('dialog')).getByRole('button', { name: /Reiniciar/ }))

        expect(casaSelecionadaNaTela()).toBeNull()
        expect(destinosMarcados()).toEqual([])
    })

    it('"Início" no modal de fim de jogo leva para "/" e libera o objeto C++ do motor', async () => {
        const { motor, usuario } = await abrir()
        await jogarNaTela(usuario, ...MATE_DO_LOUCO)
        expect(screen.getByTestId('local')).toHaveTextContent('/game')

        await usuario.click(within(screen.getByRole('dialog')).getByRole('link', { name: /Início/ }))

        expect(screen.getByTestId('local')).toHaveTextContent(/^\/$/)
        expect(() => motor.fen()).toThrow()   // o Jogo foi delete()d: usar de novo seria vazamento/bug
    })

    it('desmontar a tela libera o objeto C++ do motor', async () => {
        const { motor, tela } = await abrir()
        expect(() => motor.fen()).not.toThrow()

        tela.unmount()
        expect(() => motor.fen()).toThrow()
    })
})

describe('Game <-> motor real: partidas pseudoaleatórias (sincronia tela x motor)', () => {
    // Joga lances legais sorteados (semente fixa, então é sempre a mesma partida), todos pela tela.
    // A cada meio-lance confere se o que está DESENHADO bate com o que o motor diz.
    it.each([1, 7, 2026])('semente %i: 80 meios-lances (ou até o fim da partida)', async (sementeDoSorteio) => {
        const { motor, usuario } = await abrir()
        const sortear = criarSorteio(sementeDoSorteio)
        let meiosLances = 0

        while (!motor.situacao().terminou && meiosLances < 80) {
            const lances = todosOsLances(motor)
            const uci = lances[Math.floor(sortear() * lances.length)]
            const fenAntes = motor.fen()

            await jogarNaTela(usuario, uci)
            meiosLances++

            // a tela mandou exatamente o lance que a pessoa "quis"
            expect(motor.fen(), `a tela não aplicou ${uci} em ${fenAntes}`).not.toBe(fenAntes)
            expect(motor.ultimoLance().uci).toBe(uci)

            // o que está desenhado é o que o motor tem
            expect(tabuleiroDoDom(), `tabuleiro após ${uci}`).toEqual(motor.tabuleiro())

            const historico = historicoDaTela()
            expect(historico.brancas.length + historico.pretas.length).toBe(meiosLances)

            const situacao = motor.situacao()
            expect(reisPiscando(), `rei piscando após ${uci}`).toBe(situacao.emXeque ? 1 : 0)

            if (situacao.terminou) {
                expect(screen.getByRole('dialog'), `modal de fim após ${uci}`).toBeInTheDocument()
            } else {
                const vez = motor.vezDasBrancas() ? 'brancas' : 'pretas'
                expect(textoDoIndicador()).toBe(`${situacao.emXeque ? 'Xeque! ' : ''}Vez das ${vez}`)
            }
        }
        expect(meiosLances).toBeGreaterThan(0)
    })
})