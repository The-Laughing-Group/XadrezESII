import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import './index.css'
import { createBrowserRouter, RouterProvider } from 'react-router-dom'
import Inicial from './pages/Inicial.tsx'
import Modos from './pages/Modos.tsx'
import Game from './pages/Game.tsx'
import Configuracao from './pages/Configuracao.tsx'
import LayoutFundo from './components/LayoutFundo.tsx'
import { ConfiguracoesProvider } from './contexts/ConfiguracaoContext.tsx'

const router = createBrowserRouter([
  { element: <LayoutFundo />, 
    children: [
      { path: "/", element: <Inicial /> },
      { path: "/modos", element: <Modos /> },
      { path: "/configuracoes", element: <Configuracao /> },
  ]},
  { path: "/game", element: <Game /> },
])

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <ConfiguracoesProvider>
      <RouterProvider router={router} />
  </ConfiguracoesProvider>
  </StrictMode>,
)
