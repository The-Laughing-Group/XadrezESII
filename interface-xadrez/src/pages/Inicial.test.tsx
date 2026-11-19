import { render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { MemoryRouter, Routes, Route } from 'react-router-dom'
import { describe, it, expect } from 'vitest'
import Inicial from './Inicial'

function renderizarInicial() {
  return render(
    <MemoryRouter initialEntries={['/']}>
      <Routes>
        <Route path="/" element={<Inicial />} />
        <Route path="/modos" element={<h1>Página de modos</h1>} />
        <Route path="/configuracoes" element={<h1>Página de configurações</h1>} />
      </Routes>
    </MemoryRouter>
  )
}

describe('Inicial', () => {
  it('exibe a logo e os links do menu', () => {
    renderizarInicial()

    expect(screen.getByAltText('XadrES2')).toBeInTheDocument()
    expect(screen.getByRole('link', { name: /jogar/i })).toBeInTheDocument()
    expect(screen.getByRole('link', { name: /configurações/i })).toBeInTheDocument()
  })

  it('os links apontam para as rotas corretas', () => {
    renderizarInicial()

    expect(screen.getByRole('link', { name: /jogar/i })).toHaveAttribute('href', '/modos')
    expect(screen.getByRole('link', { name: /configurações/i })).toHaveAttribute('href', '/configuracoes')
  })

  it('clicar em Jogar leva para a página de modos', async () => {
    const user = userEvent.setup()
    renderizarInicial()

    await user.click(screen.getByRole('link', { name: /jogar/i }))

    expect(screen.getByText('Página de modos')).toBeInTheDocument()
  })

  it('clicar em Configurações leva para a página de configurações', async () => {
    const user = userEvent.setup()
    renderizarInicial()

    await user.click(screen.getByRole('link', { name: /configurações/i }))

    expect(screen.getByText('Página de configurações')).toBeInTheDocument()
  })
})