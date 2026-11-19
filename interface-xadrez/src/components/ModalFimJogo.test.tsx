import { render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { MemoryRouter, Routes, Route } from 'react-router-dom'
import { describe, it, expect, vi } from 'vitest'
import ModalFimJogo from './ModalFimJogo'

interface Opcoes {
  aberto?: boolean
  textoIndicador?: string
}

function renderizarModal({ aberto = true, textoIndicador = 'Xeque-mate! Brancas vencem' }: Opcoes = {}) {
  const aoFechar = vi.fn()
  const reiniciarJogo = vi.fn()

  const elemento = (aoFecharAtual: () => void, abertoAtual: boolean) => (
    <MemoryRouter initialEntries={['/jogo']}>
      <Routes>
        <Route
          path="/jogo"
          element={
            <ModalFimJogo
              aberto={abertoAtual}
              aoFechar={aoFecharAtual}
              reiniciarJogo={reiniciarJogo}
              textoIndicador={textoIndicador}
            />
          }
        />
        <Route path="/" element={<h1>Página inicial</h1>} />
      </Routes>
    </MemoryRouter>
  )

  const resultado = render(elemento(aoFechar, aberto))

  const rerenderizar = (novoAoFechar: () => void, novoAberto = aberto) =>
    resultado.rerender(elemento(novoAoFechar, novoAberto))

  return { ...resultado, aoFechar, reiniciarJogo, rerenderizar }
}

describe('ModalFimJogo', () => {
  describe('visibilidade', () => {
    it('não renderiza nada quando aberto é false', () => {
      const { container } = renderizarModal({ aberto: false })

      expect(screen.queryByRole('dialog')).not.toBeInTheDocument()
      expect(container).toBeEmptyDOMElement()
    })

    it('renderiza o diálogo quando aberto é true', () => {
      renderizarModal()

      const dialogo = screen.getByRole('dialog')
      expect(dialogo).toBeInTheDocument()
      expect(dialogo).toHaveAttribute('aria-modal', 'true')
    })
  })

  describe('conteúdo', () => {
    it('exibe o texto do indicador como título', () => {
      renderizarModal({ textoIndicador: 'Empate por afogamento' })

      expect(screen.getByRole('heading', { level: 1, name: 'Empate por afogamento' })).toBeInTheDocument()
    })

    it('exibe os botões de fechar e reiniciar e o link de início', () => {
      renderizarModal()

      expect(screen.getByRole('button', { name: '⨉' })).toHaveClass('modal-fechar')
      expect(screen.getByRole('button', { name: /reiniciar/i })).toBeInTheDocument()
      expect(screen.getByRole('link', { name: /início/i })).toBeInTheDocument()
    })

    it('o link de início aponta para a rota raiz', () => {
      renderizarModal()
      expect(screen.getByRole('link', { name: /início/i })).toHaveAttribute('href', '/')
    })
  })

  describe('ações', () => {
    it('clicar em Reiniciar chama reiniciarJogo', async () => {
      const user = userEvent.setup()
      const { reiniciarJogo, aoFechar } = renderizarModal()

      await user.click(screen.getByRole('button', { name: /reiniciar/i }))

      expect(reiniciarJogo).toHaveBeenCalledTimes(1)
      expect(aoFechar).not.toHaveBeenCalled()
    })

    it('clicar no botão de fechar chama aoFechar uma única vez', async () => {
      const user = userEvent.setup()
      const { aoFechar } = renderizarModal()

      await user.click(screen.getByRole('button', { name: '⨉' }))

      // o clique não pode "vazar" para o fundo e disparar aoFechar duas vezes
      expect(aoFechar).toHaveBeenCalledTimes(1)
    })

    it('clicar no fundo escuro chama aoFechar', async () => {
      const user = userEvent.setup()
      const { container, aoFechar } = renderizarModal()

      await user.click(container.querySelector('.modal-fundo') as HTMLElement)

      expect(aoFechar).toHaveBeenCalledTimes(1)
    })

    it('clicar dentro da caixa não fecha o modal', async () => {
      const user = userEvent.setup()
      const { aoFechar } = renderizarModal()

      await user.click(screen.getByRole('dialog'))
      await user.click(screen.getByRole('heading', { level: 1 }))

      expect(aoFechar).not.toHaveBeenCalled()
    })

    it('clicar em Início navega para a página inicial', async () => {
      const user = userEvent.setup()
      renderizarModal()

      await user.click(screen.getByRole('link', { name: /início/i }))

      expect(screen.getByText('Página inicial')).toBeInTheDocument()
      expect(screen.queryByRole('dialog')).not.toBeInTheDocument()
    })
  })

  describe('tecla Escape', () => {
    it('pressionar Escape chama aoFechar quando o modal está aberto', async () => {
      const user = userEvent.setup()
      const { aoFechar } = renderizarModal()

      await user.keyboard('{Escape}')

      expect(aoFechar).toHaveBeenCalledTimes(1)
    })

    it('outras teclas não fecham o modal', async () => {
      const user = userEvent.setup()
      const { aoFechar } = renderizarModal()

      await user.keyboard('{Enter}')
      await user.keyboard('a')

      expect(aoFechar).not.toHaveBeenCalled()
    })

    it('pressionar Escape não faz nada quando o modal está fechado', async () => {
      const user = userEvent.setup()
      const { aoFechar } = renderizarModal({ aberto: false })

      await user.keyboard('{Escape}')

      expect(aoFechar).not.toHaveBeenCalled()
    })

    it('remove o listener ao desmontar', async () => {
      const user = userEvent.setup()
      const { aoFechar, unmount } = renderizarModal()

      unmount()
      await user.keyboard('{Escape}')

      expect(aoFechar).not.toHaveBeenCalled()
    })

    it('remove o listener quando o modal é fechado', async () => {
      const user = userEvent.setup()
      const { aoFechar, rerenderizar } = renderizarModal()

      rerenderizar(aoFechar, false)
      await user.keyboard('{Escape}')

      expect(aoFechar).not.toHaveBeenCalled()
    })

    it('usa o aoFechar mais recente quando a função muda', async () => {
      const user = userEvent.setup()
      const { aoFechar: antigo, rerenderizar } = renderizarModal()
      const novo = vi.fn()

      rerenderizar(novo)
      await user.keyboard('{Escape}')

      expect(antigo).not.toHaveBeenCalled()
      expect(novo).toHaveBeenCalledTimes(1)
    })
  })
})