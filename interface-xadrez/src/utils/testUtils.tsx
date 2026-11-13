import { render } from '@testing-library/react'
import { MemoryRouter, Routes, Route, useLocation } from 'react-router-dom'
import type { ReactElement } from 'react'
import { ConfiguracoesProvider } from '../contexts/ConfiguracaoContext'

function LocalAtual() {
  const { pathname, search } = useLocation()
  return <div data-testid="local">{pathname + search}</div>
}

export function renderComRotas(pagina: ReactElement, rota: string) {
  return render(
    <ConfiguracoesProvider>
      <MemoryRouter initialEntries={[rota]}>
        <Routes>
          <Route path={rota.split('?')[0]} element={pagina} />
          <Route path="*" element={null} />
        </Routes>
        <LocalAtual />
      </MemoryRouter>
    </ConfiguracoesProvider>
  )
}