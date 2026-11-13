import { render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { describe, it, expect, vi, beforeEach } from 'vitest'
import BarraSuperior from './BarraSuperior'
import type { Partida } from '../hooks/usePartida'
import { posicaoToUci, type Posicao } from '../utils/notacao'

const { tocarPromocao, useSomMock } = vi.hoisted(() => {
  const tocarPromocao = vi.fn()
  return { tocarPromocao, useSomMock: vi.fn(() => tocarPromocao) }
})

vi.mock('../hooks/useSom', () => ({
  useSom: useSomMock,
}))

const ORIGEM: Posicao = { row: 1, col: 0 }
const DESTINO: Posicao = { row: 0, col: 0 }
const UCI_BASE = posicaoToUci(ORIGEM.row, ORIGEM.col) + posicaoToUci(DESTINO.row, DESTINO.col)

function criarPartida(overrides: Record<string, unknown> = {}): Partida {
  return {
    vezDasBrancas: true,
    tentarLance: vi.fn(() => true),
    ...overrides,
  } as unknown as Partida
}

interface Opcoes {
  partida?: Partida
  promocao?: { origem: Posicao; destino: Posicao } | null
  textoIndicador?: string
}

function renderizarBarra({ partida = criarPartida(), promocao = null, textoIndicador = 'Vez das brancas' }: Opcoes = {}) {
  const setConfigAberta = vi.fn()
  const limparSelecao = vi.fn()

  const resultado = render(
    <BarraSuperior
      setConfigAberta={setConfigAberta}
      promocao={promocao}
      limparSelecao={limparSelecao}
      partida={partida}
      textoIndicador={textoIndicador}
    />
  )

  return { ...resultado, partida, setConfigAberta, limparSelecao }
}

describe('BarraSuperior', () => {
  beforeEach(() => {
    vi.clearAllMocks()
  })

  describe('botão de configurações', () => {
    it('exibe o botão com nome acessível e título', () => {
      renderizarBarra()

      const botao = screen.getByRole('button', { name: /configurações/i })
      expect(botao).toBeInTheDocument()
      expect(botao).toHaveAttribute('title', 'Configurações')
    })

    it('clicar no botão abre as configurações', async () => {
      const user = userEvent.setup()
      const { setConfigAberta } = renderizarBarra()

      await user.click(screen.getByRole('button', { name: /configurações/i }))

      expect(setConfigAberta).toHaveBeenCalledTimes(1)
      expect(setConfigAberta).toHaveBeenCalledWith(true)
    })
  })

  describe('indicador de vez', () => {
    it('exibe o texto recebido por props', () => {
      renderizarBarra({ textoIndicador: 'Xeque!' })
      expect(screen.getByText('Xeque!')).toBeInTheDocument()
    })

    it('usa a classe branca na vez das brancas', () => {
      renderizarBarra({ partida: criarPartida({ vezDasBrancas: true }) })

      const indicador = screen.getByText('Vez das brancas')
      expect(indicador).toHaveClass('indicador-vez', 'indicador-vez-branca')
      expect(indicador).not.toHaveClass('indicador-vez-preta')
    })

    it('usa a classe preta na vez das pretas', () => {
      renderizarBarra({
        partida: criarPartida({ vezDasBrancas: false }),
        textoIndicador: 'Vez das pretas',
      })

      const indicador = screen.getByText('Vez das pretas')
      expect(indicador).toHaveClass('indicador-vez', 'indicador-vez-preta')
      expect(indicador).not.toHaveClass('indicador-vez-branca')
    })
  })

  describe('opções de promoção', () => {
    it('não exibe opções de promoção quando não há promoção pendente', () => {
      renderizarBarra({ promocao: null })

      expect(screen.queryByRole('button', { name: '♛' })).not.toBeInTheDocument()
      expect(screen.queryByRole('button', { name: '⨉' })).not.toBeInTheDocument()
      expect(screen.getAllByRole('button')).toHaveLength(1)
    })

    it('exibe rainha, torre, bispo, cavalo e o botão de cancelar quando há promoção', () => {
      renderizarBarra({ promocao: { origem: ORIGEM, destino: DESTINO } })

      expect(screen.getByRole('button', { name: '♛' })).toBeInTheDocument()
      expect(screen.getByRole('button', { name: '♜' })).toBeInTheDocument()
      expect(screen.getByRole('button', { name: '♝' })).toBeInTheDocument()
      expect(screen.getByRole('button', { name: '♞' })).toBeInTheDocument()
      expect(screen.getByRole('button', { name: '⨉' })).toHaveClass('cancelar')
      // 4 opções + cancelar + configurações
      expect(screen.getAllByRole('button')).toHaveLength(6)
    })

    it.each([
      ['♛', 'q'],
      ['♜', 'r'],
      ['♝', 'b'],
      ['♞', 'n'],
    ])('escolher %s tenta o lance com a letra "%s"', async (simbolo, letra) => {
      const user = userEvent.setup()
      const { partida, limparSelecao } = renderizarBarra({
        promocao: { origem: ORIGEM, destino: DESTINO },
      })

      await user.click(screen.getByRole('button', { name: simbolo }))

      expect(partida.tentarLance).toHaveBeenCalledTimes(1)
      expect(partida.tentarLance).toHaveBeenCalledWith(UCI_BASE + letra)
      expect(limparSelecao).toHaveBeenCalledTimes(1)
    })

    it('toca o som de promoção quando o lance é aceito', async () => {
      const user = userEvent.setup()
      renderizarBarra({
        partida: criarPartida({ tentarLance: vi.fn(() => true) }),
        promocao: { origem: ORIGEM, destino: DESTINO },
      })

      await user.click(screen.getByRole('button', { name: '♛' }))

      expect(tocarPromocao).toHaveBeenCalledTimes(1)
    })

    it('não toca o som, mas limpa a seleção, quando o lance é recusado', async () => {
      const user = userEvent.setup()
      const { limparSelecao } = renderizarBarra({
        partida: criarPartida({ tentarLance: vi.fn(() => false) }),
        promocao: { origem: ORIGEM, destino: DESTINO },
      })

      await user.click(screen.getByRole('button', { name: '♛' }))

      expect(tocarPromocao).not.toHaveBeenCalled()
      expect(limparSelecao).toHaveBeenCalledTimes(1)
    })

    it('cancelar limpa a seleção sem tentar lance nem tocar som', async () => {
      const user = userEvent.setup()
      const { partida, limparSelecao } = renderizarBarra({
        promocao: { origem: ORIGEM, destino: DESTINO },
      })

      await user.click(screen.getByRole('button', { name: '⨉' }))

      expect(limparSelecao).toHaveBeenCalledTimes(1)
      expect(partida.tentarLance).not.toHaveBeenCalled()
      expect(tocarPromocao).not.toHaveBeenCalled()
    })
  })

  describe('som', () => {
    it('carrega o som "promocao" via useSom', () => {
      renderizarBarra()
      expect(useSomMock).toHaveBeenCalledWith('promocao')
    })
  })
})