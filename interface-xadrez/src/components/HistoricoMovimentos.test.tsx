import { render, screen, within } from '@testing-library/react'
import { describe, it, expect } from 'vitest'
import HistoricoMovimentos from './HistoricoMovimentos'

function renderizarHistorico(historicoMovimentos: string[]) {
  const { container } = render(<HistoricoMovimentos historicoMovimentos={historicoMovimentos} />)

  const [rodadas, brancas, pretas] = Array.from(
    container.querySelectorAll('.coluna-movimentos')
  ) as HTMLElement[]

  const textos = (coluna: HTMLElement) =>
    Array.from(coluna.querySelectorAll('p')).map((p) => p.textContent?.trim())

  return { container, rodadas, brancas, pretas, textos }
}

describe('HistoricoMovimentos', () => {
  it('exibe as três colunas com seus títulos', () => {
    const { container } = renderizarHistorico([])

    expect(container.querySelector('#historico-movimentos')).toBeInTheDocument()
    expect(container.querySelectorAll('.coluna-movimentos')).toHaveLength(3)
    expect(screen.getByRole('heading', { name: 'Rodadas' })).toBeInTheDocument()
    expect(screen.getByRole('heading', { name: 'Brancas' })).toBeInTheDocument()
    expect(screen.getByRole('heading', { name: 'Pretas' })).toBeInTheDocument()
  })

  it('não exibe nenhum movimento quando o histórico está vazio', () => {
    const { container } = renderizarHistorico([])
    expect(container.querySelectorAll('p')).toHaveLength(0)
  })

  it('com um único lance, preenche rodada e brancas e deixa as pretas vazias', () => {
    const { rodadas, brancas, pretas, textos } = renderizarHistorico(['e4'])

    expect(textos(rodadas)).toEqual(['1.'])
    expect(textos(brancas)).toEqual(['e4'])
    expect(textos(pretas)).toEqual([])
  })

  it('separa os lances das brancas (índices pares) e das pretas (índices ímpares)', () => {
    const { brancas, pretas, textos } = renderizarHistorico(['e4', 'e5', 'Nf3', 'Nc6'])

    expect(textos(brancas)).toEqual(['e4', 'Nf3'])
    expect(textos(pretas)).toEqual(['e5', 'Nc6'])
  })

  it('numera as rodadas a partir de 1, uma por lance das brancas', () => {
    const { rodadas, textos } = renderizarHistorico(['e4', 'e5', 'Nf3', 'Nc6', 'Bb5'])

    expect(textos(rodadas)).toEqual(['1.', '2.', '3.'])
  })

  it('com número ímpar de lances, a última rodada só tem lance das brancas', () => {
    const { brancas, pretas, textos } = renderizarHistorico(['e4', 'e5', 'Nf3'])

    expect(textos(brancas)).toHaveLength(2)
    expect(textos(pretas)).toHaveLength(1)
  })

  it('mostra cada lance dentro da coluna correta', () => {
    const { brancas, pretas } = renderizarHistorico(['d4', 'd5'])

    expect(within(brancas).getByText('d4')).toBeInTheDocument()
    expect(within(brancas).queryByText('d5')).not.toBeInTheDocument()
    expect(within(pretas).getByText('d5')).toBeInTheDocument()
    expect(within(pretas).queryByText('d4')).not.toBeInTheDocument()
  })

  it('exibe lances repetidos sem perder nenhum', () => {
    const { brancas, pretas, textos } = renderizarHistorico(['Nf3', 'Nf6', 'Ng1', 'Ng8', 'Nf3', 'Nf6'])

    expect(textos(brancas)).toEqual(['Nf3', 'Ng1', 'Nf3'])
    expect(textos(pretas)).toEqual(['Nf6', 'Ng8', 'Nf6'])
  })
})