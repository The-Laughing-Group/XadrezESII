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

const tamanhoTabuleiro = () => screen.getByRole('slider', { name: /tamanho do tabuleiro/i })
const volumeEfeitos = () => screen.getByRole('slider', { name: /volume dos efeitos/i })
const volumeMusica = () => screen.getByRole('slider', { name: /volume da música/i })

describe('PainelConfiguracoes', () => {
  beforeEach(() => tocarMovimento.mockClear())

  it('reflete a configuração padrão', () => {
    renderizar()

    expect(screen.getByRole('checkbox', { name: /histórico de lances/i })).toBeChecked()
    expect(screen.getByRole('checkbox', { name: /efeitos sonoros/i })).toBeChecked()
    expect(screen.getByRole('checkbox', { name: /música ambiente/i })).toBeChecked()
    expect(tamanhoTabuleiro()).toHaveValue('1')
    expect(volumeEfeitos()).toHaveValue('0.5')
    expect(volumeMusica()).toHaveValue('0.3')
  })

  it('os sliders de volume têm nome fixo e o valor em aria-valuetext', () => {
    renderizar()

    expect(volumeEfeitos()).toHaveAccessibleName('Volume dos efeitos')
    expect(volumeEfeitos()).toHaveAttribute('aria-valuetext', '50%')
    expect(volumeMusica()).toHaveAccessibleName('Volume da música')
    expect(volumeMusica()).toHaveAttribute('aria-valuetext', '30%')
  })

  it('mostra as porcentagens como texto visível', () => {
    renderizar()

    expect(screen.getByText('Volume 50%')).toBeInTheDocument()
    expect(screen.getByText('Volume 30%')).toBeInTheDocument()
  })

  it('o checkbox do histórico altera mostrarHistorico', async () => {
    const user = userEvent.setup()
    const lerConfig = renderizar()

    await user.click(screen.getByRole('checkbox', { name: /histórico de lances/i }))

    expect(lerConfig().mostrarHistorico).toBe(false)
  })

  it.each([
    ['tamanho do tabuleiro', tamanhoTabuleiro, '0.8', 'escala', 0.8],
    ['volume dos efeitos', volumeEfeitos, '0.8', 'volume', 0.8],
    ['volume da música', volumeMusica, '0.6', 'volumeMusica', 0.6],
  ] as const)('o slider de %s altera só a sua chave da configuração', (_, slider, valor, chave, esperado) => {
    const lerConfig = renderizar()
    const antes = lerConfig()

    fireEvent.change(slider(), { target: { value: valor } })

    expect(lerConfig()).toEqual({ ...antes, [chave]: esperado })
  })

  it('o volume dos efeitos atualiza o texto visível e o aria-valuetext, sem mexer no da música', () => {
    renderizar()

    fireEvent.change(volumeEfeitos(), { target: { value: '0.8' } })

    expect(screen.getByText('Volume 80%')).toBeInTheDocument()
    expect(screen.queryByText('Volume 50%')).not.toBeInTheDocument()
    expect(volumeEfeitos()).toHaveAttribute('aria-valuetext', '80%')
    expect(volumeMusica()).toHaveAttribute('aria-valuetext', '30%')
  })

  it('o volume da música atualiza o texto visível e o aria-valuetext, sem mexer nos efeitos', () => {
    renderizar()

    fireEvent.change(volumeMusica(), { target: { value: '0.6' } })

    expect(screen.getByText('Volume 60%')).toBeInTheDocument()
    expect(screen.queryByText('Volume 30%')).not.toBeInTheDocument()
    expect(volumeMusica()).toHaveAttribute('aria-valuetext', '60%')
    expect(volumeEfeitos()).toHaveAttribute('aria-valuetext', '50%')
  })

  it('arredonda a porcentagem exibida', () => {
    renderizar()

    fireEvent.change(volumeEfeitos(), { target: { value: '0.35' } })

    expect(screen.getByText('Volume 35%')).toBeInTheDocument()
    expect(volumeEfeitos()).toHaveAttribute('aria-valuetext', '35%')
  })

  it('soltar o slider de volume dos efeitos toca o som de exemplo', () => {
    renderizar()

    fireEvent.pointerUp(volumeEfeitos())

    expect(tocarMovimento).toHaveBeenCalledOnce()
  })

  it('soltar o slider de volume da música não toca o som de efeito', () => {
    renderizar()

    fireEvent.pointerUp(volumeMusica())

    expect(tocarMovimento).not.toHaveBeenCalled()
  })

  it('desligar efeitos sonoros desabilita o volume dos efeitos, mas não o da música', async () => {
    const user = userEvent.setup()
    renderizar()

    await user.click(screen.getByRole('checkbox', { name: /efeitos sonoros/i }))

    expect(volumeEfeitos()).toBeDisabled()
    expect(volumeMusica()).toBeEnabled()
  })

  it('desligar a música desabilita o volume da música, mas não o dos efeitos', async () => {
    const user = userEvent.setup()
    renderizar()

    await user.click(screen.getByRole('checkbox', { name: /música ambiente/i }))

    expect(volumeMusica()).toBeDisabled()
    expect(volumeEfeitos()).toBeEnabled()
  })

  it('religar os efeitos sonoros reabilita o volume dos efeitos', async () => {
    const user = userEvent.setup()
    renderizar()
    const efeitos = screen.getByRole('checkbox', { name: /efeitos sonoros/i })

    await user.click(efeitos)
    await user.click(efeitos)

    expect(volumeEfeitos()).toBeEnabled()
  })

  it('religar a música reabilita o volume da música', async () => {
    const user = userEvent.setup()
    renderizar()
    const musica = screen.getByRole('checkbox', { name: /música ambiente/i })

    await user.click(musica)
    await user.click(musica)

    expect(volumeMusica()).toBeEnabled()
  })
})