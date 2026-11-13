import { render } from '@testing-library/react'
import { describe, it, expect } from 'vitest'
import FundoPecas from './FundoPecas'

function renderizarFundo() {
  const { container, ...resto } = render(<FundoPecas />)
  const fundo = container.querySelector('.fundo-pecas') as HTMLElement
  const pecas = Array.from(container.querySelectorAll('img.peca-fundo')) as HTMLImageElement[]
  return { container, fundo, pecas, ...resto }
}

describe('FundoPecas', () => {
  it('renderiza o container decorativo escondido de leitores de tela', () => {
    const { fundo } = renderizarFundo()

    expect(fundo).toBeInTheDocument()
    expect(fundo).toHaveAttribute('aria-hidden', 'true')
  })

  it('renderiza 12 peças de fundo', () => {
    const { pecas } = renderizarFundo()
    expect(pecas).toHaveLength(12)
  })

  it('usa uma imagem diferente para cada uma das 12 peças', () => {
    const { pecas } = renderizarFundo()

    const fontes = pecas.map((img) => img.getAttribute('src'))

    expect(fontes.every(Boolean)).toBe(true)
    expect(new Set(fontes).size).toBe(12)
  })

  it('as imagens são decorativas: alt vazio e não arrastáveis', () => {
    const { pecas } = renderizarFundo()

    pecas.forEach((img) => {
      expect(img).toHaveAttribute('alt', '')
      expect(img).toHaveAttribute('draggable', 'false')
    })
  })

  it('define a posição horizontal (--x) de cada peça', () => {
    const { pecas } = renderizarFundo()

    expect(pecas[0].style.getPropertyValue('--x')).toBe('0%')
    expect(pecas[1].style.getPropertyValue('--x')).toBe('37%')
    expect(pecas[2].style.getPropertyValue('--x')).toBe('74%')
    expect(pecas[3].style.getPropertyValue('--x')).toBe('11%')
  })

  it('alterna os tamanhos entre 48px, 64px e 96px', () => {
    const { pecas } = renderizarFundo()

    expect(pecas[0].style.getPropertyValue('--tamanho')).toBe('48px')
    expect(pecas[1].style.getPropertyValue('--tamanho')).toBe('64px')
    expect(pecas[2].style.getPropertyValue('--tamanho')).toBe('96px')
    expect(pecas[3].style.getPropertyValue('--tamanho')).toBe('48px')
  })

  it('mantém a duração da animação entre 25s e 44s', () => {
    const { pecas } = renderizarFundo()

    pecas.forEach((img) => {
      const duracao = parseInt(img.style.getPropertyValue('--duracao'), 10)
      expect(img.style.getPropertyValue('--duracao')).toMatch(/^\d+s$/)
      expect(duracao).toBeGreaterThanOrEqual(25)
      expect(duracao).toBeLessThanOrEqual(44)
    })

    expect(pecas[0].style.getPropertyValue('--duracao')).toBe('25s')
    expect(pecas[1].style.getPropertyValue('--duracao')).toBe('32s')
  })

  it('usa atraso negativo para a animação já começar no meio do caminho', () => {
    const { pecas } = renderizarFundo()

    pecas.forEach((img) => {
      expect(img.style.getPropertyValue('--atraso')).toMatch(/^-\d+s$/)
    })

    expect(pecas[1].style.getPropertyValue('--atraso')).toBe('-5s')
    expect(pecas[2].style.getPropertyValue('--atraso')).toBe('-10s')
  })

  it('gera sempre o mesmo resultado entre renderizações (determinístico)', () => {
    const primeira = renderizarFundo()
    const estilos1 = primeira.pecas.map((img) => img.getAttribute('style'))
    const srcs1 = primeira.pecas.map((img) => img.getAttribute('src'))
    primeira.unmount()

    const segunda = renderizarFundo()
    const estilos2 = segunda.pecas.map((img) => img.getAttribute('style'))
    const srcs2 = segunda.pecas.map((img) => img.getAttribute('src'))

    expect(estilos2).toEqual(estilos1)
    expect(srcs2).toEqual(srcs1)
  })
})