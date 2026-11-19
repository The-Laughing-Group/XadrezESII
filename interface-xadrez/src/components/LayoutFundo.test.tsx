import { render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { MemoryRouter, Routes, Route, Link } from 'react-router-dom'
import { describe, it, expect } from 'vitest'
import LayoutFundo from './LayoutFundo'

function renderizarLayout(rotaInicial = '/') {
  return render(
    <MemoryRouter initialEntries={[rotaInicial]}>
      <Routes>
        <Route element={<LayoutFundo />}>
          <Route
            path="/"
            element={
              <>
                <h1>Página inicial</h1>
                <Link to="/modos">Ir para modos</Link>
              </>
            }
          />
          <Route path="/modos" element={<h1>Página de modos</h1>} />
        </Route>
      </Routes>
    </MemoryRouter>
  )
}

describe('LayoutFundo', () => {
  it('renderiza o container do layout', () => {
    const { container } = renderizarLayout()
    expect(container.querySelector('#layout-fundo')).toBeInTheDocument()
  })

  it('renderiza o fundo de peças dentro do layout', () => {
    const { container } = renderizarLayout()

    const layout = container.querySelector('#layout-fundo') as HTMLElement
    const fundo = layout.querySelector('.fundo-pecas')

    expect(fundo).toBeInTheDocument()
    expect(layout.querySelectorAll('img.peca-fundo')).toHaveLength(12)
  })

  it('renderiza o conteúdo da rota filha via Outlet', () => {
    const { container } = renderizarLayout('/')

    const layout = container.querySelector('#layout-fundo') as HTMLElement

    expect(screen.getByText('Página inicial')).toBeInTheDocument()
    expect(layout).toContainElement(screen.getByText('Página inicial'))
  })

  it('renderiza o fundo antes do conteúdo da rota', () => {
    const { container } = renderizarLayout('/')

    const fundo = container.querySelector('.fundo-pecas') as HTMLElement
    const conteudo = screen.getByText('Página inicial')

    expect(
      fundo.compareDocumentPosition(conteudo) & Node.DOCUMENT_POSITION_FOLLOWING
    ).toBeTruthy()
  })

  it('exibe a rota filha correspondente à URL atual', () => {
    renderizarLayout('/modos')

    expect(screen.getByText('Página de modos')).toBeInTheDocument()
    expect(screen.queryByText('Página inicial')).not.toBeInTheDocument()
  })

  it('mantém o fundo ao navegar entre rotas', async () => {
    const user = userEvent.setup()
    const { container } = renderizarLayout('/')

    const fundoAntes = container.querySelector('.fundo-pecas')

    await user.click(screen.getByRole('link', { name: /ir para modos/i }))

    expect(screen.getByText('Página de modos')).toBeInTheDocument()
    expect(container.querySelectorAll('.fundo-pecas')).toHaveLength(1)
    expect(container.querySelector('.fundo-pecas')).toBe(fundoAntes)
  })
})