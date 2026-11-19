import { render, act } from '@testing-library/react'
import { describe, it, expect } from 'vitest'
import { ConfiguracoesProvider } from './ConfiguracaoContext'
import { useConfiguracoes } from '../hooks/useConfiguracoes'

function Controle() {
  const { atualizar } = useConfiguracoes()
  return (
    <>
      <button onClick={() => atualizar({ escala: 0.8 })}>escala</button>
      <button onClick={() => atualizar({ mostrarHistorico: false })}>esconder</button>
    </>
  )
}

describe('ConfiguracoesProvider', () => {
  const raiz = document.documentElement

  it('aplica a escala padrão como variável CSS', () => {
    render(<ConfiguracoesProvider><Controle /></ConfiguracoesProvider>)
    expect(raiz.style.getPropertyValue('--escala')).toBe('1')
    expect(raiz).not.toHaveClass('sem-historico')
  })

  it('atualiza --escala ao mudar o tamanho', async () => {
    const { getByText } = render(<ConfiguracoesProvider><Controle /></ConfiguracoesProvider>)

    await act(async () => getByText('escala').click())

    expect(raiz.style.getPropertyValue('--escala')).toBe('0.8')
  })

  it('adiciona a classe sem-historico ao esconder o histórico', async () => {
    const { getByText } = render(<ConfiguracoesProvider><Controle /></ConfiguracoesProvider>)

    await act(async () => getByText('esconder').click())

    expect(raiz).toHaveClass('sem-historico')
  })

  it('já aplica a classe ao montar se o histórico estava desligado', () => {
    localStorage.setItem('configuracoes', JSON.stringify({ mostrarHistorico: false }))

    render(<ConfiguracoesProvider><Controle /></ConfiguracoesProvider>)

    expect(raiz).toHaveClass('sem-historico')
  })
})