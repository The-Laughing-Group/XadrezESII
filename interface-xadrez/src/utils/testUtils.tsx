import { render } from '@testing-library/react'
import { MemoryRouter, Routes, Route } from 'react-router-dom'
import type { ReactElement } from 'react'
import { ConfiguracoesProvider } from '../contexts/ConfiguracaoContext'
import LocalAtual from './localAtual'

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