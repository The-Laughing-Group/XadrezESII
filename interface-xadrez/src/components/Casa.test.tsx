import { render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { describe, it, expect, vi } from 'vitest'
import Casa, { type CasaProps } from './Casa'

const pecaPeao = { tipo: 'P' as const, img: '/peaoBranco.png' }

function renderizarCasa(props: Partial<CasaProps> = {}) {
  const onSelecionar = vi.fn()
  const resultado = render(
    <Casa
      row={0}
      col={0}
      peca={null}
      selecionada={false}
      onSelecionar={onSelecionar}
      movimentoPossivel={false}
      podeSelecionar={false}
      {...props}
    />
  )
  const casa = resultado.container.querySelector('.casa') as HTMLElement
  const area = resultado.container.querySelector('.casa-area') as HTMLElement
  return { ...resultado, casa, area, onSelecionar }
}

describe('Casa', () => {
  describe('cores', () => {
    it('usa casa-branca quando row + col é par', () => {
      const { casa } = renderizarCasa({ row: 0, col: 0 })
      expect(casa).toHaveClass('casa', 'casa-branca')
      expect(casa).not.toHaveClass('casa-preta')
    })

    it('usa casa-preta quando row + col é ímpar', () => {
      const { casa } = renderizarCasa({ row: 0, col: 1 })
      expect(casa).toHaveClass('casa', 'casa-preta')
      expect(casa).not.toHaveClass('casa-branca')
    })

    it('usa casa-selecionada quando está selecionada', () => {
      const { casa } = renderizarCasa({ selecionada: true })
      expect(casa).toHaveClass('casa-selecionada')
      expect(casa).not.toHaveClass('casa-branca')
    })

    it('usa movimento-possivel-branco em casa clara com movimento possível', () => {
      const { casa } = renderizarCasa({ row: 2, col: 2, movimentoPossivel: true })
      expect(casa).toHaveClass('movimento-possivel-branco')
      expect(casa).not.toHaveClass('casa-branca')
    })

    it('usa movimento-possivel-preto em casa escura com movimento possível', () => {
      const { casa } = renderizarCasa({ row: 2, col: 3, movimentoPossivel: true })
      expect(casa).toHaveClass('movimento-possivel-preto')
      expect(casa).not.toHaveClass('casa-preta')
    })

    it('selecionada tem prioridade sobre movimento possível', () => {
      const { casa } = renderizarCasa({ selecionada: true, movimentoPossivel: true })
      expect(casa).toHaveClass('casa-selecionada')
      expect(casa).not.toHaveClass('movimento-possivel-branco')
      expect(casa).not.toHaveClass('movimento-possivel-preto')
    })
  })

  describe('atributos e estilo', () => {
    it('expõe data-row e data-col', () => {
      const { casa } = renderizarCasa({ row: 3, col: 5 })
      expect(casa).toHaveAttribute('data-row', '3')
      expect(casa).toHaveAttribute('data-col', '5')
    })

    it('define as variáveis CSS --lin e --col', () => {
      const { casa } = renderizarCasa({ row: 3, col: 5 })
      expect(casa.style.getPropertyValue('--lin')).toBe('3')
      expect(casa.style.getPropertyValue('--col')).toBe('5')
    })
  })

  describe('peça', () => {
    it('não renderiza imagem quando não há peça', () => {
      renderizarCasa({ peca: null })
      expect(screen.queryByRole('img')).not.toBeInTheDocument()
    })

    it('renderiza a imagem da peça com src, alt e data-tipo', () => {
      renderizarCasa({ peca: pecaPeao })

      const img = screen.getByRole('img')
      expect(img).toHaveAttribute('src', '/peaoBranco.png')
      expect(img).toHaveAttribute('alt', 'P')
      expect(img).toHaveAttribute('data-tipo', 'P')
      expect(img).toHaveAttribute('draggable', 'false')
      expect(img).toHaveClass('peca')
    })

    it('adiciona peca-selecionavel quando podeSelecionar é true', () => {
      renderizarCasa({ peca: pecaPeao, podeSelecionar: true })
      expect(screen.getByRole('img')).toHaveClass('peca-selecionavel')
    })

    it('não adiciona peca-selecionavel quando podeSelecionar é false ou undefined', () => {
      const { unmount } = renderizarCasa({ peca: pecaPeao, podeSelecionar: false })
      expect(screen.getByRole('img')).not.toHaveClass('peca-selecionavel')
      unmount()

      renderizarCasa({ peca: pecaPeao, podeSelecionar: undefined })
      expect(screen.getByRole('img')).not.toHaveClass('peca-selecionavel')
    })
  })

  describe('cliques', () => {
    it('clicar na área da casa chama onSelecionar com row e col', async () => {
      const user = userEvent.setup()
      const { area, onSelecionar } = renderizarCasa({ row: 4, col: 6 })

      await user.click(area)

      expect(onSelecionar).toHaveBeenCalledTimes(1)
      expect(onSelecionar).toHaveBeenCalledWith(4, 6)
    })

    it('clicar na imagem da peça chama onSelecionar com row e col', async () => {
      const user = userEvent.setup()
      const { onSelecionar } = renderizarCasa({ row: 7, col: 1, peca: pecaPeao })

      await user.click(screen.getByRole('img'))

      expect(onSelecionar).toHaveBeenCalledTimes(1)
      expect(onSelecionar).toHaveBeenCalledWith(7, 1)
    })
  })
})