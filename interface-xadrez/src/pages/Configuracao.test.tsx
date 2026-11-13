import { screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { beforeEach, describe, it, expect, vi } from 'vitest'
import Configuracao from './Configuracao'
import { renderComRotas } from '../utils/testUtils'

vi.mock('../components/PainelConfiguracoes', () => ({
  default: () => <div>Painel falso</div>,
}))

describe('Configuracao', () => {
  let user: ReturnType<typeof userEvent.setup>

  beforeEach(() => {
    user = userEvent.setup()
    renderComRotas(<Configuracao />, '/configuracoes')
  })

  it('mostra o título da página', () => {
    expect(screen.getByRole('heading', { level: 1, name: 'Configurações' })).toBeInTheDocument()
  })

  it('renderiza o painel de configurações', () => {
    expect(screen.getByText('Painel falso')).toBeInTheDocument()
  })

  it('o botão Voltar aponta para a página inicial', () => {
    expect(screen.getByRole('link', { name: 'Voltar' })).toHaveAttribute('href', '/')
  })

  it('o botão Voltar tem dica de ferramenta (title)', () => {
    expect(screen.getByRole('link', { name: 'Voltar' })).toHaveAttribute('title', 'Voltar')
  })

  it('clicar em Voltar leva para a página inicial', async () => {
    expect(screen.getByTestId('local')).toHaveTextContent('/configuracoes')

    await user.click(screen.getByRole('link', { name: 'Voltar' }))

    expect(screen.getByTestId('local')).toHaveTextContent(/^\/$/)
    expect(screen.queryByText('Painel falso')).not.toBeInTheDocument()
  })
})