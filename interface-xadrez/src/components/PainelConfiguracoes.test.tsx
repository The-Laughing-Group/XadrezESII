import { render, screen, fireEvent } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { beforeEach, describe, it, expect, vi } from 'vitest'
import PainelConfiguracoes from './PainelConfiguracoes'
import { ConfiguracoesProvider } from '../contexts/ConfiguracaoContext'
import { useConfiguracoes } from '../hooks/useConfiguracoes'

const tocarMovimento = vi.fn()
vi.mock('../hooks/useSom', () => ({ useSom: () => tocarMovimento }))

function EspiaConfig() {
  const { config } = useConfiguracoes()
  return <output data-testid="config">{JSON.stringify(config)}</output>
}

function renderizar() {
  render(
    <ConfiguracoesProvider>
      <PainelConfiguracoes />
      <EspiaConfig />
    </ConfiguracoesProvider>
  )
  return () => JSON.parse(screen.getByTestId('config').textContent!)
}

describe('PainelConfiguracoes', () => {
  beforeEach(() => tocarMovimento.mockClear())

  const volumeEfeitos = () => screen.getByRole('slider', { name: /volume dos efeitos/i })
  const volumeMusica = () => screen.getByRole('slider', { name: /volume da música/i })

  it('reflete a configuração padrão', () => {
    renderizar()

    expect(screen.getByRole('checkbox', { name: /histórico de lances/i })).toBeChecked()
    expect(screen.getByRole('checkbox', { name: /efeitos sonoros/i })).toBeChecked()
    expect(screen.getByRole('checkbox', { name: /música ambiente/i })).toBeChecked()
    expect(screen.getByRole('slider', { name: /tamanho do tabuleiro/i })).toHaveValue('1')
    expect(volumeEfeitos()).toHaveAttribute('aria-valuetext', '50%')
    expect(volumeMusica()).toHaveAttribute('aria-valuetext', '30%')
  })

  it('o checkbox do histórico altera mostrarHistorico', async () => {
    const user = userEvent.setup()
    const lerConfig = renderizar()

    await user.click(screen.getByRole('checkbox', { name: /histórico de lances/i }))

    expect(lerConfig().mostrarHistorico).toBe(false)
  })

  it.each([
    ['tamanho do tabuleiro', /tamanho do tabuleiro/i, '0.8', 'escala', 0.8],
    ['volume dos efeitos', /volume dos efeitos/i, '0.8', 'volume', 0.8],
    ['volume da música', /volume da música/i, '0.6', 'volumeMusica', 0.6],
    ] as const)('o slider de %s altera a configuração', (_, nome, valor, chave, esperado) => {
      const lerConfig = renderizar()

      fireEvent.change(screen.getByRole('slider', { name: nome }), { target: { value: valor } })

      expect(lerConfig()[chave]).toBe(esperado)
    })

  it('soltar o slider de volume toca o som de exemplo', () => {
    renderizar()

    fireEvent.pointerUp(volumeEfeitos())

    expect(tocarMovimento).toHaveBeenCalledOnce()
  })

  it('desligar efeitos sonoros desabilita o volume dos efeitos, mas não o da música', async () => {
    const user = userEvent.setup()
    renderizar()

    await user.click(screen.getByRole('checkbox', { name: /efeitos sonoros/i }))

    expect(volumeEfeitos()).toBeDisabled()
    expect(volumeMusica()).toBeEnabled()
  })

  it('desligar a música desabilita o volume da música', async () => {
    const user = userEvent.setup()
    renderizar()

    await user.click(screen.getByRole('checkbox', { name: /música ambiente/i }))

    expect(volumeMusica()).toBeDisabled()
    expect(volumeEfeitos()).toBeEnabled()
  })
})