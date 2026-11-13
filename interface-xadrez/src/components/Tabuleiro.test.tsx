import { useState } from 'react'
import { render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { describe, it, expect, vi } from 'vitest'
import Tabuleiro from './Tabuleiro'
import type { Partida } from '../hooks/usePartida'
import { posicaoToUci, type Posicao } from '../utils/notacao'

const TABULEIRO_INICIAL = [
  'rnbqkbnr',
  'pppppppp',
  '........',
  '........',
  '........',
  '........',
  'PPPPPPPP',
  'RNBQKBNR',
]

const TABULEIRO_PROMOCAO = [
  '....k...',
  'P.......',
  '........',
  '........',
  '........',
  '........',
  '........',
  '....K...',
]

const uci = (row: number, col: number) => posicaoToUci(row, col)

function criarPartida(overrides: Record<string, unknown> = {}): Partida {
  return {
    tabuleiro: TABULEIRO_INICIAL,
    vezDasBrancas: true,
    situacao: { emXeque: false, estado: 'em_andamento', terminou: false },
    lancesDe: vi.fn(() => []),
    tentarLance: vi.fn(),
    ...overrides,
  } as unknown as Partida
}

interface WrapperProps {
  partida: Partida
  contraIA?: boolean
}

function TabuleiroComEstado({ partida, contraIA = false }: WrapperProps) {
  const [promocao, setPromocao] = useState<{ origem: Posicao; destino: Posicao } | null>(null)
  const [casaSelecionada, setCasaSelecionada] = useState<Posicao | null>(null)
  const [movimentosPossiveis, setMovimentosPossiveis] = useState<Posicao[]>([])

  const limparSelecao = () => {
    setCasaSelecionada(null)
    setMovimentosPossiveis([])
    setPromocao(null)
  }

  return (
    <>
      <Tabuleiro
        partida={partida}
        promocao={promocao}
        setPromocao={setPromocao}
        casaSelecionada={casaSelecionada}
        setCasaSelecionada={setCasaSelecionada}
        movimentosPossiveis={movimentosPossiveis}
        setMovimentosPossiveis={setMovimentosPossiveis}
        limparSelecao={limparSelecao}
        contraIA={contraIA}
      />
      <span data-testid="promocao">{promocao ? JSON.stringify(promocao) : 'nenhuma'}</span>
    </>
  )
}

function renderizarTabuleiro(partida: Partida, contraIA = false) {
  const resultado = render(<TabuleiroComEstado partida={partida} contraIA={contraIA} />)
  const casaEm = (row: number, col: number) =>
    resultado.container.querySelector(`[data-row="${row}"][data-col="${col}"]`) as HTMLElement
  const pecaEm = (row: number, col: number) =>
    casaEm(row, col).querySelector('img') as HTMLImageElement | null
  return { ...resultado, casaEm, pecaEm }
}

describe('Tabuleiro', () => {
  describe('renderização', () => {
    it('renderiza 64 casas e 32 peças na posição inicial', () => {
      const { container } = renderizarTabuleiro(criarPartida())

      expect(container.querySelectorAll('.casa')).toHaveLength(64)
      expect(screen.getAllByRole('img')).toHaveLength(32)
    })

    it('posiciona as peças nas casas corretas', () => {
      const { pecaEm } = renderizarTabuleiro(criarPartida())

      expect(pecaEm(0, 0)).toHaveAttribute('data-tipo', 'r')
      expect(pecaEm(0, 4)).toHaveAttribute('data-tipo', 'k')
      expect(pecaEm(1, 3)).toHaveAttribute('data-tipo', 'p')
      expect(pecaEm(6, 0)).toHaveAttribute('data-tipo', 'P')
      expect(pecaEm(7, 3)).toHaveAttribute('data-tipo', 'Q')
      expect(pecaEm(7, 4)).toHaveAttribute('data-tipo', 'K')
      expect(pecaEm(4, 4)).toBeNull()
    })

    it('renderiza apenas as peças presentes no tabuleiro', () => {
      renderizarTabuleiro(criarPartida({ tabuleiro: TABULEIRO_PROMOCAO }))
      expect(screen.getAllByRole('img')).toHaveLength(3)
    })
  })

  describe('rei em xeque', () => {
    it('usa imagem diferente no rei branco em xeque quando é a vez das brancas', () => {
      const normal = renderizarTabuleiro(criarPartida())
      const srcNormal = normal.pecaEm(7, 4)!.getAttribute('src')
      normal.unmount()

      const { pecaEm } = renderizarTabuleiro(
        criarPartida({ situacao: { emXeque: true, estado: 'xeque', terminou: false } })
      )

      expect(pecaEm(7, 4)!.getAttribute('src')).not.toBe(srcNormal)
      expect(pecaEm(7, 4)!.getAttribute('src')).toMatch(/Danger/i)
    })

    it('não altera a imagem do rei preto quando é a vez das brancas', () => {
      const normal = renderizarTabuleiro(criarPartida())
      const srcNormal = normal.pecaEm(0, 4)!.getAttribute('src')
      normal.unmount()

      const { pecaEm } = renderizarTabuleiro(
        criarPartida({ situacao: { emXeque: true, estado: 'xeque', terminou: false } })
      )

      expect(pecaEm(0, 4)!.getAttribute('src')).toBe(srcNormal)
    })

    it('usa imagem de xeque no rei preto quando é a vez das pretas', () => {
      const normal = renderizarTabuleiro(criarPartida({ vezDasBrancas: false }))
      const srcNormal = normal.pecaEm(0, 4)!.getAttribute('src')
      normal.unmount()

      const { pecaEm } = renderizarTabuleiro(
        criarPartida({
          vezDasBrancas: false,
          situacao: { emXeque: true, estado: 'xeque', terminou: false },
        })
      )

      expect(pecaEm(0, 4)!.getAttribute('src')).not.toBe(srcNormal)
    })

    it('também mostra o rei em perigo em xeque-mate', () => {
      const normal = renderizarTabuleiro(criarPartida())
      const srcNormal = normal.pecaEm(7, 4)!.getAttribute('src')
      normal.unmount()

      const { pecaEm } = renderizarTabuleiro(
        criarPartida({ situacao: { emXeque: false, estado: 'xeque_mate', terminou: true } })
      )

      expect(pecaEm(7, 4)!.getAttribute('src')).not.toBe(srcNormal)
    })
  })

  describe('quais peças podem ser selecionadas', () => {
    it('na vez das brancas, só as peças brancas são selecionáveis', () => {
      const { pecaEm } = renderizarTabuleiro(criarPartida())

      expect(pecaEm(6, 4)).toHaveClass('peca-selecionavel')
      expect(pecaEm(7, 0)).toHaveClass('peca-selecionavel')
      expect(pecaEm(1, 4)).not.toHaveClass('peca-selecionavel')
      expect(pecaEm(0, 0)).not.toHaveClass('peca-selecionavel')
    })

    it('na vez das pretas, só as peças pretas são selecionáveis', () => {
      const { pecaEm } = renderizarTabuleiro(criarPartida({ vezDasBrancas: false }))

      expect(pecaEm(1, 4)).toHaveClass('peca-selecionavel')
      expect(pecaEm(6, 4)).not.toHaveClass('peca-selecionavel')
    })

    it('nenhuma peça é selecionável quando a partida terminou', () => {
      const { pecaEm } = renderizarTabuleiro(
        criarPartida({ situacao: { emXeque: false, estado: 'xeque_mate', terminou: true } })
      )

      expect(pecaEm(6, 4)).not.toHaveClass('peca-selecionavel')
      expect(pecaEm(1, 4)).not.toHaveClass('peca-selecionavel')
    })

    it('contra a IA, nenhuma peça é selecionável na vez das pretas', () => {
      const { pecaEm } = renderizarTabuleiro(criarPartida({ vezDasBrancas: false }), true)

      expect(pecaEm(1, 4)).not.toHaveClass('peca-selecionavel')
      expect(pecaEm(6, 4)).not.toHaveClass('peca-selecionavel')
    })

    it('contra a IA, as peças brancas continuam selecionáveis na vez das brancas', () => {
      const { pecaEm } = renderizarTabuleiro(criarPartida(), true)
      expect(pecaEm(6, 4)).toHaveClass('peca-selecionavel')
    })
  })

  describe('seleção e movimentos', () => {
    const lancesDoPeaoE2 = () =>
      vi.fn((origem: string) =>
        origem === uci(6, 4) ? [uci(6, 4) + uci(4, 4), uci(6, 4) + uci(5, 4)] : []
      )

    it('clicar em uma peça própria seleciona a casa e destaca os movimentos', async () => {
      const user = userEvent.setup()
      const lancesDe = lancesDoPeaoE2()
      const { casaEm, pecaEm } = renderizarTabuleiro(criarPartida({ lancesDe }))

      await user.click(pecaEm(6, 4)!)

      expect(lancesDe).toHaveBeenCalledWith(uci(6, 4))
      expect(casaEm(6, 4)).toHaveClass('casa-selecionada')
      expect(casaEm(4, 4)).toHaveClass('movimento-possivel-branco')
      expect(casaEm(5, 4)).toHaveClass('movimento-possivel-preto')
      expect(casaEm(3, 4)).not.toHaveClass('movimento-possivel-preto')
      expect(casaEm(3, 4)).not.toHaveClass('movimento-possivel-branco')
    })

    it('clicar em uma peça adversária não seleciona nada', async () => {
      const user = userEvent.setup()
      const lancesDe = lancesDoPeaoE2()
      const { casaEm, pecaEm } = renderizarTabuleiro(criarPartida({ lancesDe }))

      await user.click(pecaEm(1, 4)!)

      expect(lancesDe).not.toHaveBeenCalled()
      expect(casaEm(1, 4)).not.toHaveClass('casa-selecionada')
    })

    it('clicar em uma casa vazia sem seleção não faz nada', async () => {
      const user = userEvent.setup()
      const partida = criarPartida()
      const { casaEm } = renderizarTabuleiro(partida)

      await user.click(casaEm(4, 4).querySelector('.casa-area')!)

      expect(partida.tentarLance).not.toHaveBeenCalled()
      expect(casaEm(4, 4)).not.toHaveClass('casa-selecionada')
    })

    it('clicar de novo na casa selecionada remove a seleção e os destaques', async () => {
      const user = userEvent.setup()
      const { casaEm, pecaEm } = renderizarTabuleiro(criarPartida({ lancesDe: lancesDoPeaoE2() }))

      await user.click(pecaEm(6, 4)!)
      await user.click(pecaEm(6, 4)!)

      expect(casaEm(6, 4)).not.toHaveClass('casa-selecionada')
      expect(casaEm(4, 4)).not.toHaveClass('movimento-possivel-branco')
      expect(casaEm(5, 4)).not.toHaveClass('movimento-possivel-preto')
    })

    it('clicar em outra peça própria troca a seleção', async () => {
      const user = userEvent.setup()
      const lancesDe = vi.fn((origem: string) =>
        origem === uci(6, 4) ? [uci(6, 4) + uci(4, 4)] : origem === uci(6, 3) ? [uci(6, 3) + uci(4, 3)] : []
      )
      const { casaEm, pecaEm } = renderizarTabuleiro(criarPartida({ lancesDe }))

      await user.click(pecaEm(6, 4)!)
      await user.click(pecaEm(6, 3)!)

      expect(casaEm(6, 4)).not.toHaveClass('casa-selecionada')
      expect(casaEm(6, 3)).toHaveClass('casa-selecionada')
      expect(casaEm(4, 3)).toHaveClass('movimento-possivel-preto')
      expect(casaEm(4, 4)).not.toHaveClass('movimento-possivel-branco')
    })

    it('clicar em um destino possível executa o lance e limpa a seleção', async () => {
      const user = userEvent.setup()
      const partida = criarPartida({ lancesDe: lancesDoPeaoE2() })
      const { casaEm, pecaEm } = renderizarTabuleiro(partida)

      await user.click(pecaEm(6, 4)!)
      await user.click(casaEm(4, 4).querySelector('.casa-area')!)

      expect(partida.tentarLance).toHaveBeenCalledTimes(1)
      expect(partida.tentarLance).toHaveBeenCalledWith(uci(6, 4) + uci(4, 4))
      expect(casaEm(6, 4)).not.toHaveClass('casa-selecionada')
      expect(casaEm(4, 4)).not.toHaveClass('movimento-possivel-branco')
    })

    it('clicar em uma casa que não é destino possível não executa lance', async () => {
      const user = userEvent.setup()
      const partida = criarPartida({ lancesDe: lancesDoPeaoE2() })
      const { casaEm, pecaEm } = renderizarTabuleiro(partida)

      await user.click(pecaEm(6, 4)!)
      await user.click(casaEm(3, 0).querySelector('.casa-area')!)

      expect(partida.tentarLance).not.toHaveBeenCalled()
    })

    it('não seleciona peças quando a partida terminou', async () => {
      const user = userEvent.setup()
      const lancesDe = lancesDoPeaoE2()
      const { casaEm, pecaEm } = renderizarTabuleiro(
        criarPartida({
          lancesDe,
          situacao: { emXeque: false, estado: 'xeque_mate', terminou: true },
        })
      )

      await user.click(pecaEm(6, 4)!)

      expect(lancesDe).not.toHaveBeenCalled()
      expect(casaEm(6, 4)).not.toHaveClass('casa-selecionada')
    })
  })

  describe('promoção', () => {
    const lancesDePromocao = () =>
      vi.fn((origem: string) =>
        origem === uci(1, 0)
          ? ['q', 'r', 'b', 'n'].map((p) => uci(1, 0) + uci(0, 0) + p)
          : []
      )

    it('mover peão para a última linha abre a promoção em vez de executar o lance', async () => {
      const user = userEvent.setup()
      const partida = criarPartida({
        tabuleiro: TABULEIRO_PROMOCAO,
        lancesDe: lancesDePromocao(),
      })
      const { casaEm, pecaEm } = renderizarTabuleiro(partida)

      await user.click(pecaEm(1, 0)!)
      await user.click(casaEm(0, 0).querySelector('.casa-area')!)

      expect(partida.tentarLance).not.toHaveBeenCalled()
      expect(JSON.parse(screen.getByTestId('promocao').textContent!)).toEqual({
        origem: { row: 1, col: 0 },
        destino: { row: 0, col: 0 },
      })
    })

    it('com promoção pendente, cliques no tabuleiro são ignorados', async () => {
      const user = userEvent.setup()
      const lancesDe = vi.fn(() => [])
      const setCasaSelecionada = vi.fn()
      const setMovimentosPossiveis = vi.fn()
      const limparSelecao = vi.fn()

      render(
        <Tabuleiro
          partida={criarPartida({ lancesDe })}
          promocao={{ origem: { row: 1, col: 0 }, destino: { row: 0, col: 0 } }}
          setPromocao={vi.fn()}
          casaSelecionada={null}
          setCasaSelecionada={setCasaSelecionada}
          movimentosPossiveis={[]}
          setMovimentosPossiveis={setMovimentosPossiveis}
          limparSelecao={limparSelecao}
          contraIA={false}
        />
      )

      await user.click(screen.getAllByAltText('P')[0])

      expect(lancesDe).not.toHaveBeenCalled()
      expect(setCasaSelecionada).not.toHaveBeenCalled()
      expect(setMovimentosPossiveis).not.toHaveBeenCalled()
      expect(limparSelecao).not.toHaveBeenCalled()
    })
  })

  describe('props controladas', () => {
    it('destaca a casa selecionada e os movimentos recebidos por props', () => {
      const { container } = render(
        <Tabuleiro
          partida={criarPartida()}
          promocao={null}
          setPromocao={vi.fn()}
          casaSelecionada={{ row: 6, col: 4 }}
          setCasaSelecionada={vi.fn()}
          movimentosPossiveis={[{ row: 4, col: 4 }, { row: 5, col: 4 }]}
          setMovimentosPossiveis={vi.fn()}
          limparSelecao={vi.fn()}
          contraIA={false}
        />
      )

      expect(container.querySelector('[data-row="6"][data-col="4"]')).toHaveClass('casa-selecionada')
      expect(container.querySelector('[data-row="4"][data-col="4"]')).toHaveClass('movimento-possivel-branco')
      expect(container.querySelector('[data-row="5"][data-col="4"]')).toHaveClass('movimento-possivel-preto')
    })
  })
})