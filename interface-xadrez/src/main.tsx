import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import './index.css'
import { createBrowserRouter, RouterProvider } from 'react-router-dom'
import Inicial from './pages/Inicial.tsx'
import Modos from './pages/Modos.tsx'
import Game from './pages/Game.tsx'
import Configuracao from './pages/Configuracao.tsx'

const router = createBrowserRouter([
  { path: "/", element: <Inicial /> },
  { path: "/modos", element: <Modos /> },
  { path: "/game", element: <Game /> },
  { path: "/configuracoes", element: <Configuracao /> },
])

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <RouterProvider router={router} />
  </StrictMode>,
)
