import { render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { MemoryRouter } from 'react-router-dom'
import { beforeEach, describe, it, expect, vi } from 'vitest'
import ModalConfiguracoes from './ModalConfiguracoes'
import { renderComRotas } from '../utils/testUtils'

vi.mock('./PainelConfiguracoes', () => ({
  default: () => <div>Painel falso</div>,
}))

const reiniciarJogo = vi.fn()
const aoFechar = vi.fn()

const modal = (aberto: boolean) => (
  <MemoryRouter>
    <ModalConfiguracoes aberto={aberto} reiniciarJogo={reiniciarJogo} aoFechar={aoFechar} />
  </MemoryRouter>
)

describe('ModalConfiguracoes', () => {
  let user: ReturnType<typeof userEvent.setup>

  beforeEach(() => {
    user = userEvent.setup()
    reiniciarJogo.mockReset()
    aoFechar.mockReset()
  })

  describe('fechado', () => {
    it('não renderiza nada', () => {
      const { container } = render(modal(false))

      expect(container).toBeEmptyDOMElement()
      expect(screen.queryByRole('dialog')).not.toBeInTheDocument()
    })

    it('não escuta o Escape', async () => {
      render(modal(false))

      await user.keyboard('{Escape}')

      expect(aoFechar).not.toHaveBeenCalled()
    })
  })

  describe('aberto', () => {
    it('mostra o diálogo com título, painel e botões', () => {
      render(modal(true))

      expect(screen.getByRole('dialog')).toBeInTheDocument()
      expect(screen.getByRole('heading', { name: 'Configurações' })).toBeInTheDocument()
      expect(screen.getByText('Painel falso')).toBeInTheDocument()
      expect(screen.getByRole('button', { name: /reiniciar/i })).toBeInTheDocument()
      expect(screen.getByRole('link', { name: /início/i })).toHaveAttribute('href', '/')
    })

    it('fecha ao apertar Escape', async () => {
      render(modal(true))

      await user.keyboard('{Escape}')

      expect(aoFechar).toHaveBeenCalledOnce()
    })

    it('não fecha com outras teclas', async () => {
      render(modal(true))

      await user.keyboard('{Enter}a')

      expect(aoFechar).not.toHaveBeenCalled()
    })

    it('fecha ao clicar no botão de fechar', async () => {
      render(modal(true))

      await user.click(screen.getByRole('button', { name: /fechar/i }))

      expect(aoFechar).toHaveBeenCalledOnce()
    })

    it('fecha ao clicar no fundo', async () => {
      render(modal(true))
      const fundo = screen.getByRole('dialog').parentElement!

      await user.click(fundo)

      expect(aoFechar).toHaveBeenCalledOnce()
    })

    it('não fecha ao clicar dentro da caixa (stopPropagation)', async () => {
      render(modal(true))

      await user.click(screen.getByRole('dialog'))
      await user.click(screen.getByRole('heading', { name: 'Configurações' }))
      await user.click(screen.getByText('Painel falso'))

      expect(aoFechar).not.toHaveBeenCalled()
    })

    it('Reiniciar reinicia o jogo e depois fecha o modal', async () => {
      const ordem: string[] = []
      reiniciarJogo.mockImplementation(() => ordem.push('reiniciar'))
      aoFechar.mockImplementation(() => ordem.push('fechar'))
      render(modal(true))

      await user.click(screen.getByRole('button', { name: /reiniciar/i }))

      expect(reiniciarJogo).toHaveBeenCalledOnce()
      expect(aoFechar).toHaveBeenCalledOnce()
      expect(ordem).toEqual(['reiniciar', 'fechar'])
    })

    it('Início leva para a página inicial', async () => {
      renderComRotas(
        <ModalConfiguracoes aberto reiniciarJogo={reiniciarJogo} aoFechar={aoFechar} />,
        '/game'
      )

      await user.click(screen.getByRole('link', { name: /início/i }))

      expect(screen.getByTestId('local')).toHaveTextContent(/^\/$/)
    })
  })

  describe('ciclo de vida do listener de teclado', () => {
    it('para de escutar o Escape depois que o modal fecha', async () => {
      const { rerender } = render(modal(true))

      rerender(modal(false))
      await user.keyboard('{Escape}')

      expect(aoFechar).not.toHaveBeenCalled()
    })

    it('começa a escutar quando o modal abre depois de montado', async () => {
      const { rerender } = render(modal(false))

      rerender(modal(true))
      await user.keyboard('{Escape}')

      expect(aoFechar).toHaveBeenCalledOnce()
    })

    it('remove o listener ao desmontar', async () => {
      const { unmount } = render(modal(true))

      unmount()
      await user.keyboard('{Escape}')

      expect(aoFechar).not.toHaveBeenCalled()
    })
  })
})