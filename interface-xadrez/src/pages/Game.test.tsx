import { render, screen, within, act } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { MemoryRouter } from 'react-router-dom'
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import Game from './Game'

const { estado, usePartidaMock, useMusicaMock, sons } = vi.hoisted(() => {
  const sons = {
    movimento: vi.fn(),
    xeque: vi.fn(),
    fimjogo: vi.fn(),
    promocao: vi.fn(),
  } as Record<string, ReturnType<typeof vi.fn>>

  const estado = { partida: null as any }

  return {
    estado,
    sons,
    usePartidaMock: vi.fn(() => estado.partida),
    useMusicaMock: vi.fn(),
  }
})

vi.mock('../hooks/usePartida', () => ({ usePartida: usePartidaMock }))
vi.mock('../hooks/useSom', () => ({ useSom: (nome: string) => sons[nome] }))
vi.mock('../hooks/useMusica', () => ({ useMusica: useMusicaMock }))

vi.mock('../components/Tabuleiro', () => ({
  default: ({ contraIA }: { contraIA: boolean }) => (
    <div data-testid="tabuleiro" data-contra-ia={String(contraIA)} />
  ),
}))

vi.mock('../components/ModalConfiguracoes', () => ({
  default: ({
    aberto,
    aoFechar,
    reiniciarJogo,
  }: {
    aberto: boolean
    aoFechar: () => void
    reiniciarJogo: () => void
  }) =>
    aberto ? (
      <div data-testid="modal-config">
        <button onClick={reiniciarJogo}>reiniciar-config</button>
        <button onClick={aoFechar}>fechar-config</button>
      </div>
    ) : null,
}))

// ---------- helpers ----------

// Monta um tabuleiro 8x8 (strings) a partir de casas, ex.: { g1: 'N', e8: 'k' }
function montar(pecas: Record<string, string> = {}): string[] {
  const grade = Array.from({ length: 8 }, () => Array<string>(8).fill('.'))
  for (const [casa, peca] of Object.entries(pecas)) {
    const col = casa.charCodeAt(0) - 97
    const row = 8 - Number(casa[1])
    grade[row][col] = peca
  }
  return grade.map((linha) => linha.join(''))
}

const SEM_LANCE = { uci: '', tipo: 'normal', captura: false, promocao: null }

function criarPartida(overrides: Record<string, any> = {}) {
  const { situacao, ...resto } = overrides
  return {
    pronta: true,
    fen: 'fen-qualquer',
    tabuleiro: montar({ e1: 'K', e8: 'k' }),
    vezDasBrancas: true,
    situacao: {
      estado: 'em_andamento',
      emXeque: false,
      terminou: false,
      vencedor: null,
      ...situacao,
    },
    ultimoLance: SEM_LANCE,
    reiniciar: vi.fn(),
    jogarIA: vi.fn(),
    lancesDe: vi.fn(() => []),
    tentarLance: vi.fn(() => true),
    ...resto,
  }
}

const arvore = (url: string) => (
  <MemoryRouter initialEntries={[url]}>
    <Game />
  </MemoryRouter>
)

function renderizarGame(partida = criarPartida(), url = '/jogo') {
  estado.partida = partida
  const resultado = render(arvore(url))

  // troca a partida devolvida pelo hook e re-renderiza (mantém o estado do componente)
  const atualizar = (nova: ReturnType<typeof criarPartida>) => {
    estado.partida = nova
    resultado.rerender(arvore(url))
  }

  const indicador = () =>
    resultado.container.querySelector('.indicador-vez') as HTMLElement

  const colunas = () =>
    Array.from(resultado.container.querySelectorAll('.coluna-movimentos')).map((coluna) =>
      Array.from(coluna.querySelectorAll('p')).map((p) => p.textContent?.trim())
    )

  return { ...resultado, partida, atualizar, indicador, colunas }
}

describe('Game', () => {
  beforeEach(() => {
    vi.clearAllMocks()
  })

  afterEach(() => {
    vi.useRealTimers()
  })

  describe('renderização', () => {
    it('não renderiza nada enquanto a partida não está pronta', () => {
      const { container } = renderizarGame(criarPartida({ pronta: false }))
      expect(container).toBeEmptyDOMElement()
    })

    it('renderiza barra superior, tabuleiro e histórico quando a partida está pronta', () => {
      const { container } = renderizarGame()

      expect(container.querySelector('#game')).toBeInTheDocument()
      expect(container.querySelector('.barra-superior')).toBeInTheDocument()
      expect(screen.getByTestId('tabuleiro')).toBeInTheDocument()
      expect(container.querySelector('#historico-movimentos')).toBeInTheDocument()
    })

    it('inicia a música de fundo', () => {
      renderizarGame()
      expect(useMusicaMock).toHaveBeenCalled()
    })

    it('os modais começam fechados', () => {
      renderizarGame()

      expect(screen.queryByRole('dialog')).not.toBeInTheDocument()
      expect(screen.queryByTestId('modal-config')).not.toBeInTheDocument()
    })
  })

  describe('texto do indicador', () => {
    it('mostra a vez das brancas', () => {
      const { indicador } = renderizarGame(criarPartida({ vezDasBrancas: true }))
      expect(indicador()).toHaveTextContent('Vez das brancas')
    })

    it('mostra a vez das pretas', () => {
      const { indicador } = renderizarGame(criarPartida({ vezDasBrancas: false }))
      expect(indicador()).toHaveTextContent('Vez das pretas')
    })

    it('mostra xeque junto com a vez', () => {
      const { indicador } = renderizarGame(
        criarPartida({ situacao: { emXeque: true }, vezDasBrancas: false })
      )
      expect(indicador()).toHaveTextContent('Xeque! Vez das pretas')
    })

    it('mostra o vencedor no xeque-mate', () => {
      const { indicador } = renderizarGame(
        criarPartida({
          situacao: { estado: 'xeque_mate', vencedor: 'brancas', terminou: true },
        })
      )
      expect(indicador()).toHaveTextContent('Xeque-mate!')
      expect(indicador()).toHaveTextContent('Vencem as brancas')
    })

    it.each([
      ['afogamento', 'Empate: afogamento'],
      ['material_insuficiente', 'Empate: material insuficiente'],
      ['regra_dos_50', 'Empate: regra dos 50 lances'],
      ['repeticao', 'Empate: repetição de posição'],
    ])('mostra o texto de empate para %s', (estadoJogo, texto) => {
      const { indicador } = renderizarGame(
        criarPartida({ situacao: { estado: estadoJogo, terminou: true } })
      )
      expect(indicador()).toHaveTextContent(texto)
    })
  })

  describe('modo contra a IA', () => {
    it('passa contraIA=false ao tabuleiro por padrão', () => {
      renderizarGame()
      expect(screen.getByTestId('tabuleiro')).toHaveAttribute('data-contra-ia', 'false')
    })

    it('passa contraIA=true ao tabuleiro com ?modo=ia', () => {
      renderizarGame(criarPartida(), '/jogo?modo=ia')
      expect(screen.getByTestId('tabuleiro')).toHaveAttribute('data-contra-ia', 'true')
    })

    it('outro valor de modo não ativa a IA', () => {
      renderizarGame(criarPartida(), '/jogo?modo=local')
      expect(screen.getByTestId('tabuleiro')).toHaveAttribute('data-contra-ia', 'false')
    })
  })

  describe('jogada da IA', () => {
    const vezDaIA = (extra: Record<string, any> = {}) =>
      criarPartida({ vezDasBrancas: false, ...extra })

    it('joga depois de 600ms na vez das pretas com dificuldade média por padrão', () => {
      vi.useFakeTimers()
      const partida = vezDaIA()
      renderizarGame(partida, '/jogo?modo=ia')

      act(() => { vi.advanceTimersByTime(599) })
      expect(partida.jogarIA).not.toHaveBeenCalled()

      act(() => { vi.advanceTimersByTime(1) })
      expect(partida.jogarIA).toHaveBeenCalledTimes(1)
      expect(partida.jogarIA).toHaveBeenCalledWith(3)
    })

    it.each([
      ['facil', 2],
      ['medio', 3],
      ['dificil', 5],
      ['inexistente', 3],
    ])('dificuldade "%s" usa profundidade %i', (dificuldade, profundidade) => {
      vi.useFakeTimers()
      const partida = vezDaIA()
      renderizarGame(partida, `/jogo?modo=ia&dificuldade=${dificuldade}`)

      act(() => { vi.advanceTimersByTime(600) })

      expect(partida.jogarIA).toHaveBeenCalledWith(profundidade)
    })

    it.each([
      ['fora do modo IA', '/jogo', {}],
      ['na vez das brancas', '/jogo?modo=ia', { vezDasBrancas: true }],
      ['com a partida terminada', '/jogo?modo=ia', { situacao: { terminou: true, estado: 'afogamento' } }],
      ['sem fen', '/jogo?modo=ia', { fen: '' }],
      ['com a partida não pronta', '/jogo?modo=ia', { pronta: false }],
    ])('não joga %s', (_descricao, url, extra) => {
      vi.useFakeTimers()
      const partida = vezDaIA(extra)
      renderizarGame(partida, url)

      act(() => { vi.advanceTimersByTime(5000) })

      expect(partida.jogarIA).not.toHaveBeenCalled()
    })

    it('cancela a jogada agendada ao desmontar', () => {
      vi.useFakeTimers()
      const partida = vezDaIA()
      const { unmount } = renderizarGame(partida, '/jogo?modo=ia')

      unmount()
      act(() => { vi.advanceTimersByTime(5000) })

      expect(partida.jogarIA).not.toHaveBeenCalled()
    })
  })

  describe('sons', () => {
    it('toca o som de xeque quando o rei entra em xeque', () => {
      renderizarGame(criarPartida({ situacao: { emXeque: true } }))
      expect(sons.xeque).toHaveBeenCalledTimes(1)
    })

    it('não toca o som de xeque sem xeque', () => {
      renderizarGame()
      expect(sons.xeque).not.toHaveBeenCalled()
    })

    it('não toca o som de xeque em xeque-mate (partida terminada)', () => {
      renderizarGame(
        criarPartida({
          situacao: { emXeque: true, estado: 'xeque_mate', terminou: true, vencedor: 'pretas' },
        })
      )
      expect(sons.xeque).not.toHaveBeenCalled()
    })

    it('toca o som de xeque de novo quando o xeque muda de false para true', () => {
      const { atualizar } = renderizarGame()

      atualizar(criarPartida({ situacao: { emXeque: true } }))
      expect(sons.xeque).toHaveBeenCalledTimes(1)

      atualizar(criarPartida({ situacao: { emXeque: false } }))
      atualizar(criarPartida({ situacao: { emXeque: true } }))
      expect(sons.xeque).toHaveBeenCalledTimes(2)
    })

    it('toca o som de fim de jogo quando a partida termina', () => {
      renderizarGame(
        criarPartida({ situacao: { estado: 'afogamento', terminou: true } })
      )
      expect(sons.fimjogo).toHaveBeenCalledTimes(1)
    })

    it('não toca o som de fim de jogo com a partida em andamento', () => {
      renderizarGame()
      expect(sons.fimjogo).not.toHaveBeenCalled()
    })

    it('toca o som de movimento quando há um novo lance', () => {
      renderizarGame(
        criarPartida({
          tabuleiro: montar({ e4: 'P' }),
          ultimoLance: { ...SEM_LANCE, uci: 'e2e4' },
        })
      )
      expect(sons.movimento).toHaveBeenCalledTimes(1)
    })

    it('não toca o som de movimento sem lance', () => {
      renderizarGame()
      expect(sons.movimento).not.toHaveBeenCalled()
    })

    it('não toca o som de movimento quando o lance dá xeque', () => {
      renderizarGame(
        criarPartida({
          tabuleiro: montar({ e4: 'P' }),
          situacao: { emXeque: true },
          ultimoLance: { ...SEM_LANCE, uci: 'e2e4' },
        })
      )
      expect(sons.movimento).not.toHaveBeenCalled()
      expect(sons.xeque).toHaveBeenCalledTimes(1)
    })
  })

  describe('histórico de movimentos', () => {
    // Renderiza com o tabuleiro "antes" e depois aplica o lance, como acontece no jogo real
    function jogarLance(
      antes: Record<string, string>,
      depois: Record<string, string>,
      ultimoLance: Record<string, any>
    ) {
      const jogo = renderizarGame(criarPartida({ tabuleiro: montar(antes) }))
      jogo.atualizar(
        criarPartida({
          tabuleiro: montar(depois),
          vezDasBrancas: false,
          ultimoLance: { ...SEM_LANCE, ...ultimoLance },
        })
      )
      return jogo
    }

    it('começa vazio', () => {
      const { colunas } = renderizarGame()
      expect(colunas()).toEqual([[], [], []])
    })

    it('registra lance simples de peão pelo destino', () => {
      const { colunas } = jogarLance({ e2: 'P' }, { e4: 'P' }, { uci: 'e2e4' })
      expect(colunas()).toEqual([['1.'], ['e4'], []])
    })

    it('registra captura de peão com a coluna de origem', () => {
      const { colunas } = jogarLance({ e4: 'P', d5: 'p' }, { d5: 'P' }, { uci: 'e4d5', captura: true })
      expect(colunas()[1]).toEqual(['exd5'])
    })

    it('registra lance de peça com a letra maiúscula e o destino', () => {
      const { colunas } = jogarLance({ g1: 'N' }, { f3: 'N' }, { uci: 'g1f3' })
      expect(colunas()[1]).toEqual(['Nf3'])
    })

    it('registra captura de peça com "x"', () => {
      const { colunas } = jogarLance({ f1: 'B', c4: 'p' }, { c4: 'B' }, { uci: 'f1c4', captura: true })
      expect(colunas()[1]).toEqual(['Bxc4'])
    })

    it('registra lance de peça preta com letra maiúscula', () => {
      const { colunas } = jogarLance({ g8: 'n' }, { f6: 'n' }, { uci: 'g8f6' })
      expect(colunas()[1]).toEqual(['Nf6'])
    })

    it('desambigua usando o tabuleiro de antes do lance', () => {
      const { colunas } = jogarLance(
        { g1: 'N', g5: 'N' },
        { f3: 'N', g5: 'N' },
        { uci: 'g1f3' }
      )
      expect(colunas()[1]).toEqual(['N1f3'])
    })

    it('registra roque curto como 0-0', () => {
      const { colunas } = jogarLance({ e1: 'K', h1: 'R' }, { g1: 'K', f1: 'R' }, { uci: 'e1g1', tipo: 'roque_rei' })
      expect(colunas()[1]).toEqual(['0-0'])
    })

    it('registra roque longo como 0-0-0', () => {
      const { colunas } = jogarLance({ e1: 'K', a1: 'R' }, { c1: 'K', d1: 'R' }, { uci: 'e1c1', tipo: 'roque_dama' })
      expect(colunas()[1]).toEqual(['0-0-0'])
    })

    it('registra promoção com a letra da peça em maiúscula', () => {
      const { colunas } = jogarLance({ a7: 'P' }, { a8: 'Q' }, { uci: 'a7a8q', promocao: 'q' })
      expect(colunas()[1]).toEqual(['a8Q'])
    })

    it('registra promoção com captura', () => {
      const { colunas } = jogarLance({ b7: 'P', a8: 'r' }, { a8: 'N' }, { uci: 'b7a8n', captura: true, promocao: 'n' })
      expect(colunas()[1]).toEqual(['bxa8N'])
    })

    it('distribui os lances entre brancas e pretas em sequência', () => {
      const jogo = renderizarGame(criarPartida({ tabuleiro: montar({ e2: 'P', e7: 'p' }) }))

      jogo.atualizar(
        criarPartida({
          tabuleiro: montar({ e4: 'P', e7: 'p' }),
          vezDasBrancas: false,
          ultimoLance: { ...SEM_LANCE, uci: 'e2e4' },
        })
      )
      jogo.atualizar(
        criarPartida({
          tabuleiro: montar({ e4: 'P', e5: 'p' }),
          vezDasBrancas: true,
          ultimoLance: { ...SEM_LANCE, uci: 'e7e5' },
        })
      )

      expect(jogo.colunas()).toEqual([['1.'], ['e4'], ['e5']])
    })

    it('não registra nada quando o último lance não tem uci', () => {
      const jogo = renderizarGame()
      jogo.atualizar(criarPartida({ ultimoLance: { ...SEM_LANCE } }))

      expect(jogo.colunas()).toEqual([[], [], []])
    })
  })

  describe('modal de configurações', () => {
    it('abre ao clicar no botão de configurações da barra superior', async () => {
      const user = userEvent.setup()
      renderizarGame()

      await user.click(screen.getByRole('button', { name: /configurações/i }))

      expect(screen.getByTestId('modal-config')).toBeInTheDocument()
    })

    it('fecha pelo aoFechar', async () => {
      const user = userEvent.setup()
      renderizarGame()

      await user.click(screen.getByRole('button', { name: /configurações/i }))
      await user.click(screen.getByText('fechar-config'))

      expect(screen.queryByTestId('modal-config')).not.toBeInTheDocument()
    })

    it('reiniciar chama reiniciar da partida e limpa o histórico', async () => {
      const user = userEvent.setup()
      const jogo = renderizarGame(criarPartida({ tabuleiro: montar({ e2: 'P' }) }))

      jogo.atualizar(
        criarPartida({
          tabuleiro: montar({ e4: 'P' }),
          vezDasBrancas: false,
          ultimoLance: { ...SEM_LANCE, uci: 'e2e4' },
        })
      )
      expect(jogo.colunas()[1]).toEqual(['e4'])

      const reiniciar = estado.partida.reiniciar
      await user.click(screen.getByRole('button', { name: /configurações/i }))
      await user.click(screen.getByText('reiniciar-config'))

      expect(reiniciar).toHaveBeenCalledTimes(1)
      expect(jogo.colunas()).toEqual([[], [], []])
    })
  })

  describe('modal de fim de jogo', () => {
    const fimDeJogo = () =>
      criarPartida({
        situacao: { estado: 'xeque_mate', vencedor: 'brancas', terminou: true },
      })

    it('abre quando a partida termina, com o texto do resultado', () => {
      renderizarGame(fimDeJogo())

      const dialogo = screen.getByRole('dialog')
      expect(within(dialogo).getByRole('heading', { level: 1 })).toHaveTextContent('Xeque-mate!')
      expect(within(dialogo).getByRole('heading', { level: 1 })).toHaveTextContent('Vencem as brancas')
    })

    it('fica fechado com a partida em andamento', () => {
      renderizarGame()
      expect(screen.queryByRole('dialog')).not.toBeInTheDocument()
    })

    it('fecha ao dispensar e continua fechado mesmo com a partida terminada', async () => {
      const user = userEvent.setup()
      renderizarGame(fimDeJogo())

      await user.click(within(screen.getByRole('dialog')).getByRole('button', { name: '⨉' }))

      expect(screen.queryByRole('dialog')).not.toBeInTheDocument()
    })

    it('reiniciar pelo modal chama reiniciar da partida', async () => {
      const user = userEvent.setup()
      const partida = fimDeJogo()
      renderizarGame(partida)

      await user.click(within(screen.getByRole('dialog')).getByRole('button', { name: /reiniciar/i }))

      expect(partida.reiniciar).toHaveBeenCalledTimes(1)
    })

    it('reiniciar o jogo reabre o aviso de fim de jogo se a partida ainda estiver terminada', async () => {
      const user = userEvent.setup()
      renderizarGame(fimDeJogo())

      // dispensa o modal de fim de jogo
      await user.click(within(screen.getByRole('dialog')).getByRole('button', { name: '⨉' }))
      expect(screen.queryByRole('dialog')).not.toBeInTheDocument()

      // reinicia pelas configurações: a dispensa é resetada
      await user.click(screen.getByRole('button', { name: /configurações/i }))
      await user.click(screen.getByText('reiniciar-config'))

      expect(screen.getByRole('dialog')).toBeInTheDocument()
    })
  })
})